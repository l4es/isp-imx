/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "bash-color.h"
#include "bm/base.h"
#include "bm/buffer.h"
#include "bm/ip/nano-d/cmodel.h"
#include "file/file.h"
#include "hal.h"
#include "json/json-app.h"
#include <assert.h>
#include <common/picture_buffer.h>
#include <libgen.h>
#if defined CMODEL_COMPARE
#include "file/dir.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// #define DISABLE_HANDSHAKE_MANUALLY
// #define INTERNAL_RESET
// #define OUTPUT_YUV
// #define CMODEL_RESET_STATE_COMPARE
#define INSERT_ACQUIRE_FRAME_NUMBER

#define MRV_BASE 0

#define VI_IRCL (MRV_BASE + 0x0014)
#define VI_IRCL_MASK_VI_ISP_SOFT_RST BIT_MASK(0, 0) // 1 -> 0

#define ISP_CTRL 0x400
#define ISP_CTRL_MASK_ISP_ENABLE BIT_MASK(0, 0)

#define ISP_ACQ_NR_FRAMES 0x418

#define MRV_MI_BASE 0x1400

#define MI_DMA_Y_PIC_START_AD (MRV_MI_BASE + 0x00C8)
#define MI_DMA_CB_PIC_START_AD (MRV_MI_BASE + 0x00D8)
#define MI_DMA_CR_PIC_START_AD (MRV_MI_BASE + 0x00E8)

#define MI_CTRL (MRV_MI_BASE + 0)
#define MI_CTRL_MASK_MP_PINGPONG_ENABLE BIT_MASK(11, 11)

#define MI_MP_Y_BASE_AD_INIT (MRV_MI_BASE + 0x0008)
#define MI_MP_Y_SIZE_INIT (MRV_MI_BASE + 0x000c)
#define MI_MP_Y_SIZE_INIT_MASK BIT_MASK(30, 3)
#define MI_MP_Y_BASE_AD_INIT2 (MRV_MI_BASE + 0x0130)

#define MI_MP_CB_BASE_AD_INIT (MRV_MI_BASE + 0x001C)
#define MI_MP_CB_SIZE_INIT (MRV_MI_BASE + 0x0020)
#define MI_MP_CB_SIZE_INIT_MASK BIT_MASK(29, 3)
#define MI_MP_CB_BASE_AD_INIT2 (MRV_MI_BASE + 0x0134)

#define MI_MP_CR_BASE_AD_INIT (MRV_MI_BASE + 0x002C)
#define MI_MP_CR_SIZE_INIT (MRV_MI_BASE + 0x0030)
#define MI_MP_CR_SIZE_INIT_MASK BIT_MASK(29, 3)
#define MI_MP_CR_BASE_AD_INIT2 (MRV_MI_BASE + 0x0138)

#if defined DISABLE_HANDSHAKE_MANUALLY
#define MI_MP_HANDSHAKE (MRV_MI_BASE + 0x014C)
#define MI_MP_HANDSHAKE_MASK_MP_HANDSHAKE_ENABLE BIT_MASK(0, 0)
#endif

#define MI_MIS (MRV_MI_BASE + 0x0100)
#define MI_MIS_MASK_MP_FRAME_END BIT_MASK(0, 0)

#define TPG_BASE 0x700

#define TPG_CTRL (TPG_BASE + 0)
#define TPG_CTRL_MASK_TPG_IMG_NUM BIT_MASK(3, 1)

#define TPG_SIZE (TPG_BASE + 0x08)
#define TPG_SIZE_MASK_HEIGHT BIT_MASK(13, 0)
#define TPG_SIZE_MASK_WIDTH BIT_MASK(27, 14)

#define PIC_BUFFER_ALIGN (0x400)

#define BUFFER_ADDRESS_SHIFT_BITS 2

#define CORE_INDEX pCtx->pBmTest->coreIndex

void IP_Create(TestCtx_t *pCtx) {
  assert(pCtx);

  if (pCtx->pBmTest) {
    return;
  }

  pCtx->pBmTest = calloc(sizeof(BMTest_t), 1);
  assert(pCtx->pBmTest);

#if !defined JUST_VERIFY_CASE_FILES
  BMTest_t *pBmTest = pCtx->pBmTest;
  BMMemPool_t *pMemPool = &pBmTest->memPool;
  MediaBufPool_t *pBufPool = &pMemPool->bufPool;
  MediaBufPoolConfig_t *pBufPoolConfig = &pMemPool->bufPoolConfig;

  pBufPoolConfig->bufNum = CHANNEL_COUNT;
  pBufPoolConfig->maxBufNum = CHANNEL_COUNT;
  pBufPoolConfig->bufSize = (24 * 1024 * 1024);
  pBufPoolConfig->bufAlign = PIC_BUFFER_ALIGN;
  pBufPoolConfig->metaDataSizeMediaBuf = 0;
  pBufPoolConfig->metaDataSizeScmiBuf = sizeof(PicBufMetaData_t);
  pBufPoolConfig->flags = BUFPOOL_RINGBUFFER;

  assert(!MediaBufPoolGetSize(pBufPoolConfig));

  MediaBufPoolMemory_t *pBufPoolMemory = &pMemPool->bufPoolMemory;

  pBufPoolMemory->pMetaDataMemory = calloc(pBufPoolConfig->metaDataMemSize, 1);
  pBufPoolMemory->pBufferMemory =
      HalAllocMemory(pCtx->hHal, pBufPoolConfig->bufMemSize);

  assert(!MediaBufPoolCreate(pBufPool, pBufPoolConfig, *pBufPoolMemory));

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer(pBufPool);
      assert(pBuffer);

      pBmTest->pOutputBuffer[j][i] = pBuffer;
      pBmTest->goldenBaseAddress[j][i] = GOLDEN_BASE_ADDRESS_INIT;
    }
  }
#endif

#if defined CMODEL_COMPARE
  CMDA_Create();
#endif
}

void IP_Error(TestCtx_t *pCtx) {}

void IP_Free(TestCtx_t *pCtx) {
#if defined CMODEL_COMPARE
  CMDA_Free();
#endif

  BMTest_t *pBmTest = pCtx->pBmTest;

#if !defined JUST_VERIFY_CASE_FILES
  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      if (pBmTest->pOutputBuffer[j][i]) {
        MediaBufPoolFreeBuffer(&pBmTest->memPool.bufPool,
                               pBmTest->pOutputBuffer[j][i]);
        pBmTest->pOutputBuffer[j][i] = NULL;
        pBmTest->goldenBaseAddress[j][i] = GOLDEN_BASE_ADDRESS_INIT;
      }
    }
  }

  BMMemPool_t *pMemPool = &pBmTest->memPool;
  MediaBufPool_t *pBufPool = &pMemPool->bufPool;
  MediaBufPoolMemory_t *pBufPoolMemory = &pMemPool->bufPoolMemory;

  HalFreeMemory(pCtx->hHal, pBufPoolMemory->pBufferMemory);
  pBufPoolMemory->pBufferMemory = ~0U;

  free(pBufPoolMemory->pMetaDataMemory);
  pBufPoolMemory->pMetaDataMemory = NULL;

  MediaBufPoolDestroy(pBufPool);
#endif

  free(pCtx->pBmTest);
  pCtx->pBmTest = NULL;
}

void IP_Golden(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig, cJSON *pReport) {
  if (CORE_INDEX) {
    return;
  }

  TestCase_t *pTestCase = &pCtx->testCase;

#if defined CMODEL_COMPARE
  snprintf(pTestCase->pathOutput, MAX_LEN, "CModel.case/nano-d/%s",
           pTestCase->name);

  DIR_MkDir(pTestCase->pathOutput, 0777);

  char pathCurrent[MAX_LEN] = {0};
  getcwd(pathCurrent, sizeof(pathCurrent));

  if (chdir(pTestCase->pathOutput)) {
    TRACE(BM_TEST_INF, "chdir failed %d: %s\n", errno, pTestCase->pathOutput);
  }

  char registerFileName[MAX_LEN] = {0};

  for (int32_t i = 0; i < CORE_MAX; i++) {
    snprintf(registerFileName, sizeof(registerFileName), "%s/%s",
             pTestCase->pathInput, pTestConfig->fileNameRegister[i]);
    if (CMDA_Refresh(i, registerFileName, pReport) < 0) {
      continue;
    }
  }

  CMDA_Output();

  if (chdir(pathCurrent)) {
    TRACE(BM_TEST_INF, "chdir failed %d: %s\n", errno, pathCurrent);
  }
#else
  snprintf(pTestCase->pathOutput, MAX_LEN, "%s", pTestCase->pathInput);
#endif
}

void IP_GoldenAnalyze(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                      cJSON *pReport) {
  int32_t ret = 0;

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[CORE_INDEX][j][0])) {
      continue;
    }

    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      char goldenFileName[MAX_LEN] = {0};

      snprintf(goldenFileName, sizeof(goldenFileName), "%s/%s",
               pCtx->testCase.pathOutput,
               pTestConfig->fileNameOutput[CORE_INDEX][j][i]);

      if (!strlen(goldenFileName)) {
        break;
      }

      BMTest_t *pBmTest = pCtx->pBmTest;
      assert(pBmTest);

      cJSON *pItem = cJSON_CreateObject();
      cJSON_AddItemToObject(pReport, basename(goldenFileName), pItem);

      BMG_AnalyzeResult_t *pAnalyzeResult = &pBmTest->goldenAnalyzeResult[j][i];

      ret = BMG_AnalyzeBuffer(goldenFileName, pAnalyzeResult);
      if (ret < 0) {
        cJSON_AddFalseToObject(pItem, JA_IS_EXIST);
        continue;
      }

      if (!pAnalyzeResult->bufferLength) {
        continue;
      }

      if (pAnalyzeResult->isRollback) {
        cJSON_AddTrueToObject(pItem, JA_IS_ROLLBACK);

        char desc[100] = {0};

        if (pAnalyzeResult->minAddr > 0) {
          snprintf(desc, sizeof(desc), "0x%09lX", pAnalyzeResult->minAddr);
          cJSON_AddStringToObject(pItem, JA_MIN_ADDR, desc);
        }

        snprintf(desc, sizeof(desc), "0x%09lX", pAnalyzeResult->bufferLength);
        cJSON_AddStringToObject(pItem, JA_BUFFER_LENGTH, desc);
      }

      if (pAnalyzeResult->isNonlinear) {
        cJSON_AddTrueToObject(pItem, JS_IS_NONLINEAR);
      }
    }

#if defined OUTPUT_YUV
    char fileName[MAX_LEN] = {0};

    snprintf(fileName, MAX_LEN, "yuv-golden-core%d-path%d-frame%d.bin",
             CORE_INDEX, j, pCtx->pBmTest->frameNumber);

    FLE_ComposeYuv(pTestConfig->fileNameOutput[CORE_INDEX][j][0],
                   pTestConfig->fileNameOutput[CORE_INDEX][j][1],
                   pTestConfig->fileNameOutput[CORE_INDEX][j][2], fileName);
#endif
  }
}

void IP_Hack(TestCtx_t *pCtx, uint64_t address, BMTest_t *pBmTest,
             uint32_t *pValue, cJSON *pReport) {
  char desc[200] = {0};

  switch (address) {
#if defined INSERT_ACQUIRE_FRAME_NUMBER
  case ISP_CTRL:
    if (*pValue & ISP_CTRL_MASK_ISP_ENABLE) {
      char number = pCtx->testCase.configCount;

      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE, "Insert acquire frame number: %d \n"),
            number);
      REG_Write(pCtx, REG_BASE(CORE_INDEX) + ISP_ACQ_NR_FRAMES, number);
    }
    break;
#endif

  case MI_CTRL:
    if (*pValue & MI_CTRL_MASK_MP_PINGPONG_ENABLE) {
      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE,
                         "Enable pingpong buffer, use same buffer... \n"),
            *pValue);
      pBmTest->isPingPangBuffer = true;

      uint32_t value =
          pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_Y]->buf.base_address >>
          BUFFER_ADDRESS_SHIFT_BITS;

      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE, "MI_MP_Y_BASE_AD_INIT2 = 0x%08X \n"),
            value);
      REG_Write(pCtx, REG_BASE(CORE_INDEX) + MI_MP_Y_BASE_AD_INIT2, value);

      value = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CB]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE, "MI_MP_CB_BASE_AD_INIT2 = 0x%08X \n"),
            value);
      REG_Write(pCtx, REG_BASE(CORE_INDEX) + MI_MP_CB_BASE_AD_INIT2, value);

      value = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CR]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE, "MI_MP_CR_BASE_AD_INIT2 = 0x%08X \n"),
            value);
      REG_Write(pCtx, REG_BASE(CORE_INDEX) + MI_MP_CR_BASE_AD_INIT2, value);
    }
    break;

  case MI_DMA_Y_PIC_START_AD:
    assert(!"Unexpected register ops: MI_DMA_Y_PIC_START_AD");
    TRACE(BM_TEST_WAN,
          COLOR_STRING(COLOR_YELLOW, "MI_DMA_Y_PIC_START_AD = 0x%X \n"),
          *pValue);
    break;

  case MI_DMA_CB_PIC_START_AD:
    assert(!"Unexpected register ops: MI_DMA_CB_PIC_START_AD");
    TRACE(BM_TEST_WAN,
          COLOR_STRING(COLOR_YELLOW, "MI_DMA_CB_PIC_START_AD = 0x%X \n"),
          *pValue);
    break;

  case MI_DMA_CR_PIC_START_AD:
    assert(!"Unexpected register ops: MI_DMA_CR_PIC_START_AD");
    TRACE(BM_TEST_WAN,
          COLOR_STRING(COLOR_YELLOW, "MI_DMA_CR_PIC_START_AD = 0x%X \n"),
          *pValue);
    break;

  case MI_MP_Y_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_Y]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_Y_BASE_AD_INIT = 0x%08X \n", *pValue);
    break;

  case MI_MP_Y_SIZE_INIT:
    snprintf(desc, sizeof(desc), "MI_MP_Y_SIZE_INIT = 0x%X",
             *pValue & MI_MP_Y_SIZE_INIT_MASK >> 3);
    TRACE(BM_TEST_INF, "%s \n", desc);
    cJSON_AddItemToArray(pReport, cJSON_CreateString(desc));
    break;

  case MI_MP_CB_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CB]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_CB_BASE_AD_INIT = 0x%08X \n", *pValue);
    break;

  case MI_MP_CB_SIZE_INIT:
    snprintf(desc, sizeof(desc), "MI_MP_CB_SIZE_INIT = 0x%X",
             *pValue & MI_MP_CB_SIZE_INIT_MASK >> 3);
    TRACE(BM_TEST_INF, "%s \n", desc);
    cJSON_AddItemToArray(pReport, cJSON_CreateString(desc));
    break;

  case MI_MP_CR_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CR]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_CR_BASE_AD_INIT = 0x%08X \n", *pValue);
    break;

  case MI_MP_CR_SIZE_INIT:
    snprintf(desc, sizeof(desc), "MI_MP_CR_SIZE_INIT = 0x%X",
             *pValue & MI_MP_CR_SIZE_INIT_MASK >> 3);
    TRACE(BM_TEST_INF, "%s \n", desc);
    cJSON_AddItemToArray(pReport, cJSON_CreateString(desc));
    break;

  case MI_MP_Y_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_Y]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_Y_BASE_AD_INIT2 = 0x%08X \n", *pValue);
    break;

  case MI_MP_CB_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CB]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_CB_BASE_AD_INIT2 = 0x%08X \n", *pValue);
    break;

  case MI_MP_CR_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_BUFFER_CR]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;

    TRACE(BM_TEST_INF, "MI_MP_CR_BASE_AD_INIT2 = 0x%08X \n", *pValue);
    break;

  case TPG_CTRL:
    snprintf(desc, sizeof(desc), "TPG_IMG_NUM: %d",
             *pValue & TPG_CTRL_MASK_TPG_IMG_NUM >> 1);
    TRACE(BM_TEST_INF, "%s \n", desc);
    cJSON_AddItemToArray(pReport, cJSON_CreateString(desc));
    break;

  case TPG_SIZE:
    snprintf(desc, sizeof(desc), "TPG size: %d x %d",
             *pValue & TPG_SIZE_MASK_WIDTH >> 14,
             *pValue & TPG_SIZE_MASK_HEIGHT);
    TRACE(BM_TEST_INF, "%s \n", desc);
    cJSON_AddItemToArray(pReport, cJSON_CreateString(desc));
    break;

#if defined DISABLE_HANDSHAKE_MANUALLY
  case MI_MP_HANDSHAKE:
    if (*pValue & MI_MP_HANDSHAKE_MASK_MP_HANDSHAKE_ENABLE) {
      *pValue &= ~MI_MP_HANDSHAKE_MASK_MP_HANDSHAKE_ENABLE;
      TRACE(BM_TEST_WAN,
            COLOR_STRING(COLOR_YELLOW, "Disable handshake manually \n"));
    }
    break;
#endif

  default:
    break;
  }
}

void IP_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                  cJSON *pReport) {}

void IP_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport) {
  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[CORE_INDEX][j][0])) {
      continue;
    }

    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      char goldenFileName[MAX_LEN] = {0};

      snprintf(goldenFileName, sizeof(goldenFileName), "%s/%s",
               pCtx->testCase.pathOutput,
               pTestConfig->fileNameOutput[CORE_INDEX][j][i]);

      if (!strlen(goldenFileName)) {
        break;
      }

      MediaBuffer_t *pMediaBuffer = pBmTest->pOutputBuffer[j][i];
      assert(pMediaBuffer);

      assert(!BMBUF_Save(
          pCtx->hHal, goldenFileName, pBmTest->pOutputBuffer[j][i],
          &pBmTest->goldenAnalyzeResult[j][i],
          &pBmTest->goldenBaseAddress[j][i], pBmTest->isPingPangBuffer,
          cJSON_GetObjectItem(pReport, basename(goldenFileName))));
    }

#if defined OUTPUT_YUV
    char fileName[MAX_LEN] = {0};

    snprintf(fileName, MAX_LEN, "yuv-fpga-core%d-path%d-frame%d.bin",
             CORE_INDEX, j, pCtx->pBmTest->frameNumber);

    char fileNameY[MAX_LEN] = {0};
    sprintf(fileNameY, "%s.out", pTestConfig->fileNameOutput[CORE_INDEX][j][0]);
    char fileNameCb[MAX_LEN] = {0};
    sprintf(fileNameCb, "%s.out",
            pTestConfig->fileNameOutput[CORE_INDEX][j][1]);
    char fileNameCr[MAX_LEN] = {0};
    sprintf(fileNameCr, "%s.out",
            pTestConfig->fileNameOutput[CORE_INDEX][j][2]);

    FLE_ComposeYuv(fileNameY, fileNameCb, fileNameCr, fileName);
#endif
  }
}

void IP_RefreshBegin(TestCtx_t *pCtx) {}

void IP_RefreshAfter(TestCtx_t *pCtx) {}

void IP_Reset(TestCtx_t *pCtx) {
  TRACE(BM_TEST_INF, COLOR_STRING(COLOR_BLUE, "Reset nano-d core %d ... \n"),
        CORE_INDEX);

  uint32_t value = REG_Read(pCtx, REG_BASE_TPG(CORE_INDEX));

  REG_Write(pCtx, REG_BASE_TPG(CORE_INDEX),
            value & ~REG_BASE_TPG_MASK_ISP_RST_N);

  REG_Write(pCtx, REG_BASE_TPG(CORE_INDEX),
            value | REG_BASE_TPG_MASK_ISP_RST_N);

#if defined INTERNAL_RESET
  value = REG_Read(pCtx, REG_BASE(CORE_INDEX) + VI_IRCL);

  REG_Write(pCtx, REG_BASE(CORE_INDEX) + VI_IRCL,
            value | VI_IRCL_MASK_VI_ISP_SOFT_RST);

  REG_Write(pCtx, REG_BASE(CORE_INDEX) + VI_IRCL,
            value & ~VI_IRCL_MASK_VI_ISP_SOFT_RST);
#endif

#if defined CMODEL_RESET_STATE_COMPARE
  FILE *pF = FLE_Open("CModel/dump_isp_regs_all.txt", "r");
  assert(pF);

  uint32_t ops = 0, address = 0, defaultValue = 0;

  while (fscanf(pF, "%u 0x%x 0x%x", &ops, &address, &defaultValue) > 0) {
    value = REG_Read(pCtx, REG_BASE(CORE_INDEX) + address);

    if (value != defaultValue) {
      TRACE(BM_TEST_WAN, COLOR_STRING(COLOR_YELLOW, "0x%x >> 0x%x != 0x%x \n"),
            address, value, defaultValue);
    } else {
      TRACE(BM_TEST_DBG, "0x%x >> 0x%x == 0x%x \n", address, value,
            defaultValue);
    }
  }

  FLE_Close(pF);
#endif
}

void IP_WaitForReady(TestCtx_t *pCtx) {
  /* Wait for HW to complete with main path. */
  while (!(REG_Read(pCtx, REG_BASE(CORE_INDEX) + MI_MIS) &
           MI_MIS_MASK_MP_FRAME_END)) {
    sleep(1);
  }
}
