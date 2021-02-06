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
#include "bm/ip/dewarp/cmodel.h"
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

#define OP_AXI_MASTER_DISABLE
#define OP_DEWARP_START_MANUALLY

#define DEWARP_CTRL 0x04
#define DEWARP_CTRL_MASK_DST_AUTO_SHADOW BIT_MASK(10, 10)

#define MAP_LUT_ADDR 0x08

#define SRC_IMAGE_Y_BASE 0x10
#define SRC_IMAGE_UV_BASE 0x14

#define MAP_LUT_ADDR2 0x20

#define SRC_IMAGE_Y_BASE2 0x28
#define SRC_IMAGE_UV_BASE2 0x2C

#define DST_IMAGE_Y_BASE 0x38
#define DST_IMAGE_UV_BASE 0x3C

#define DST_IMAGE_Y_BASE2 0x48
#define DST_IMAGE_UV_BASE2 0x4C

#define INTERRUPT_STATUS 0x70
#define INTERRUPT_STATUS_MASK_INT_FRAME_DONE BIT_MASK(0, 0)
#define INTERRUPT_STATUS_MASK_INT_ERROR_STATUS BIT_MASK(7, 1)

#define BUS_CTRL 0x74
#define BUS_CTRL_MASK_AXI_MASTER_ENABLE BIT_MASK(31, 31)

#define PIC_BUFFER_ALIGN 0x400

#define BUFFER_ADDRESS_SHIFT_BITS 4

#define SHADOW_CHANNEL_OFFSET 2

#define CORE_INDEX pCtx->pBmTest->coreIndex

/*
Input Buffer Map
    0     1     2     3
0   Y     UV    LUT   LUT2
1   -     -     -     -
*/

typedef enum {
  INPUT_Y,
  INPUT_UV,
  INPUT_LUT,
  INPUT_LUT2,
} InputIndex_t;

/*
Output Buffer Map
    0     1     2     3
0   Y     UV    Y2    UV2
1   -     -     -     -
*/

typedef enum {
  OUTPUT_Y,
  OUTPUT_UV,
  OUTPUT_Y2,
  OUTPUT_UV2,
} OutputIndex_t;

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
  pBufPoolConfig->bufSize = (16 * 1920 * 1080);
  pBufPoolConfig->bufAlign = PIC_BUFFER_ALIGN;
  pBufPoolConfig->metaDataSizeMediaBuf = 0;
  pBufPoolConfig->metaDataSizeScmiBuf = sizeof(PicBufMetaData_t);
  pBufPoolConfig->flags = BUFPOOL_RINGBUFFER;

  assert(!MediaBufPoolGetSize(pBufPoolConfig));

  MediaBufPoolMemory_t *pBufPoolMemory = &pMemPool->bufPoolMemory;

  pBufPoolMemory->pMetaDataMemory = calloc(pBufPoolConfig->metaDataMemSize, 1);
  assert(pBufPoolMemory->pMetaDataMemory);

  pBufPoolMemory->pBufferMemory =
      HalAllocMemory(pCtx->hHal, pBufPoolConfig->bufMemSize);

  assert(!MediaBufPoolCreate(pBufPool, pBufPoolConfig, *pBufPoolMemory));

  for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer(pBufPool);
    assert(pBuffer);

    pBmTest->pInputBuffer[0][i] = pBuffer;
  }

  for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer(pBufPool);
    assert(pBuffer);

    pBmTest->pOutputBuffer[0][i] = pBuffer;
    pBmTest->goldenBaseAddress[0][i] = GOLDEN_BASE_ADDRESS_INIT;
  }
#endif

#if defined CMODEL_COMPARE
  CMDA_Create();
#endif
}

void IP_Error(TestCtx_t *pCtx) {
  uint32_t value = REG_Read(pCtx, REG_BASE(CORE_INDEX) + INTERRUPT_STATUS) &
                   INTERRUPT_STATUS_MASK_INT_ERROR_STATUS;
  if (value) {
    TRACE(BM_TEST_WAN,
          COLOR_STRING(COLOR_YELLOW, "Error Status: 0x%08X >> 0x%08X \n"),
          REG_BASE(CORE_INDEX) + INTERRUPT_STATUS, value);
  }
}

void IP_Free(TestCtx_t *pCtx) {
#if defined CMODEL_COMPARE
  CMDA_Free();
#endif

  BMTest_t *pBmTest = pCtx->pBmTest;

#if !defined JUST_VERIFY_CASE_FILES
  for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
    if (pBmTest->pInputBuffer[0][i]) {
      MediaBufPoolFreeBuffer(&pBmTest->memPool.bufPool,
                             pBmTest->pInputBuffer[0][i]);
      pBmTest->pInputBuffer[0][i] = NULL;
    }
  }

  for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
    if (pBmTest->pOutputBuffer[0][i]) {
      MediaBufPoolFreeBuffer(&pBmTest->memPool.bufPool,
                             pBmTest->pOutputBuffer[0][i]);
      pBmTest->pOutputBuffer[0][i] = NULL;
      pBmTest->goldenBaseAddress[0][i] = GOLDEN_BASE_ADDRESS_INIT;
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
  snprintf(pTestCase->pathOutput, MAX_LEN, "CModel.case/dewarp/%s",
           pTestCase->name);

  DIR_MkDir(pTestCase->pathOutput, 0777);

  char pathCurrent[MAX_LEN] = {0};
  getcwd(pathCurrent, sizeof(pathCurrent));

  if (chdir(pTestCase->pathOutput)) {
    TRACE(BM_TEST_INF, "chdir failed %d: %s\n", errno, pTestCase->pathOutput);
  }

  for (int32_t i = 0; i < CORE_MAX; i++) {
    char dataFileName[MAX_LEN] = {0};
    sprintf(dataFileName, "%s/%s", pTestCase->pathInput,
            pTestConfig->fileNameInput[i][0][0]);

    char dataFileName2[MAX_LEN] = {0};
    sprintf(dataFileName2, "%s/%s", pTestCase->pathInput,
            pTestConfig->fileNameInput[i][0][1]);

    char mapFileName[MAX_LEN] = {0};
    sprintf(mapFileName, "%s/%s", pTestCase->pathInput,
            pTestConfig->fileNameInput[i][0][2]);

    char mapFileName2[MAX_LEN] = {0};
    sprintf(mapFileName2, "%s/%s", pTestCase->pathInput,
            pTestConfig->fileNameInput[i][0][3]);

    CMDA_Input(dataFileName, dataFileName2, mapFileName, mapFileName2);

    char registerFileName[MAX_LEN] = {0};
    snprintf(registerFileName, sizeof(registerFileName), "%s/%s",
             pTestCase->pathInput, pTestConfig->fileNameRegister[i]);

    CMDA_Refresh(i, registerFileName, pReport);
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
      if (!strlen(pTestConfig->fileNameOutput[CORE_INDEX][j][i])) {
        break;
      }

      char goldenFileName[MAX_LEN] = {0};

      snprintf(goldenFileName, sizeof(goldenFileName), "%s/%s",
               pCtx->testCase.pathOutput,
               pTestConfig->fileNameOutput[CORE_INDEX][j][i]);

      BMTest_t *pBmTest = pCtx->pBmTest;
      assert(pBmTest);

      cJSON *pItem = cJSON_CreateObject();
      cJSON_AddItemToObject(pReport, basename(goldenFileName), pItem);

      int8_t bufferOffset =
          (pBmTest->isDstAutoShadow &&
           pBmTest->frameNumberDstAutoShadow % SHADOW_CHANNEL_OFFSET)
              ? SHADOW_CHANNEL_OFFSET
              : 0;

      BMG_AnalyzeResult_t *pAnalyzeResult =
          &pBmTest->goldenAnalyzeResult[j][i + bufferOffset];

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
          snprintf(desc, sizeof(desc), "0x%lX", pAnalyzeResult->minAddr);
          cJSON_AddStringToObject(pItem, JA_MIN_ADDR, desc);
        }

        snprintf(desc, sizeof(desc), "0x%lX", pAnalyzeResult->bufferLength);
        cJSON_AddStringToObject(pItem, JA_BUFFER_LENGTH, desc);
      }

      if (pAnalyzeResult->isNonlinear) {
        cJSON_AddTrueToObject(pItem, JS_IS_NONLINEAR);
      }
    }
  }
}

void IP_Hack(TestCtx_t *pCtx, uint64_t address, BMTest_t *pBmTest,
             uint32_t *pValue) {
  switch (address) {
  case DEWARP_CTRL:
    if (*pValue & DEWARP_CTRL_MASK_DST_AUTO_SHADOW) {
      pBmTest->isDstAutoShadow = true;
      TRACE(BM_TEST_INF, "DST_AUTO_SHADOW on\n");
    } else {
      pBmTest->isDstAutoShadow = false;
      pBmTest->frameNumberDstAutoShadow = 0;
    }
    break;

  case SRC_IMAGE_Y_BASE:
    *pValue = pBmTest->pInputBuffer[0][INPUT_Y]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "SRC_IMAGE_Y_BASE = 0x%08X\n", *pValue);
    break;

  case SRC_IMAGE_UV_BASE:
    *pValue = pBmTest->pInputBuffer[0][INPUT_UV]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "SRC_IMAGE_UV_BASE = 0x%08X\n", *pValue);
    break;

  case MAP_LUT_ADDR:
    *pValue = pBmTest->pInputBuffer[0][INPUT_LUT]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "MAP_LUT_ADDR = 0x%08X\n", *pValue);
    break;

  case MAP_LUT_ADDR2:
    *pValue = pBmTest->pInputBuffer[0][INPUT_LUT2]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "MAP_LUT_ADDR2 = 0x%08X\n", *pValue);
    break;

  case SRC_IMAGE_Y_BASE2:
    TRACE(BM_TEST_WAN,
          "Unexpected register operating DST_IMAGE_Y_BASE2 = 0x%08X\n",
          *pValue);
    break;

  case SRC_IMAGE_UV_BASE2:
    TRACE(BM_TEST_WAN,
          "Unexpected register operating SRC_IMAGE_UV_BASE2 = 0x%08X\n",
          *pValue);
    break;

  case DST_IMAGE_Y_BASE:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_Y]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "DST_IMAGE_Y_BASE = 0x%08X\n", *pValue);
    break;

  case DST_IMAGE_UV_BASE:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_UV]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "DST_IMAGE_UV_BASE = 0x%08X\n", *pValue);
    break;

  case DST_IMAGE_Y_BASE2:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_Y2]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "DST_IMAGE_Y_BASE2 = 0x%08X\n", *pValue);
    break;

  case DST_IMAGE_UV_BASE2:
    *pValue = pBmTest->pOutputBuffer[0][OUTPUT_UV2]->buf.base_address >>
              BUFFER_ADDRESS_SHIFT_BITS;
    TRACE(BM_TEST_INF, "DST_IMAGE_UV_BASE2 = 0x%08X\n", *pValue);
    break;

  default:
    break;
  }
}

void IP_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                  cJSON *pReport) {
  int32_t ret = 0;
  uint64_t size = 0;

  for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
    char fileName[MAX_LEN] = {0};

    snprintf(fileName, sizeof(fileName), "%s/%s", pCtx->testCase.pathInput,
             pTestConfig->fileNameInput[CORE_INDEX][0][i]);
    if (!strlen(fileName)) {
      break;
    }

    cJSON *pItem = cJSON_CreateObject();
    cJSON_AddItemToObject(pReport, basename(fileName), pItem);

    ret = FLE_Size(fileName, &size);
    if (ret) {
      cJSON_AddBoolToObject(pItem, JA_IS_EXIST, false);
      continue;
    }

#if defined JUST_VERIFY_CASE_FILES
    continue;
#endif

    if (size == 0) {
      continue;
    }

    ret = BMBUF_Load(pCtx->hHal, fileName, pCtx->pBmTest->pInputBuffer[0][i],
                     pItem);
    if (ret) {
      cJSON_AddBoolToObject(pItem, JA_IS_VALID, false);
    }
  }
}

void IP_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport) {
  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[CORE_INDEX][j][0])) {
      continue;
    }

    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      if (!strlen(pTestConfig->fileNameOutput[CORE_INDEX][j][i])) {
        break;
      }

      char goldenFileName[MAX_LEN] = {0};

      snprintf(goldenFileName, sizeof(goldenFileName), "%s/%s",
               pCtx->testCase.pathOutput,
               pTestConfig->fileNameOutput[CORE_INDEX][j][i]);

      MediaBuffer_t *pMediaBuffer = pBmTest->pOutputBuffer[j][i];
      assert(pMediaBuffer);

      TRACE(BM_TEST_INF,
            COLOR_STRING(COLOR_BLUE, "pBmTest->isDstAutoShadow = %d \n"),
            pBmTest->isDstAutoShadow);

      int8_t bufferOffset =
          (pBmTest->isDstAutoShadow &&
           pBmTest->frameNumberDstAutoShadow % SHADOW_CHANNEL_OFFSET)
              ? SHADOW_CHANNEL_OFFSET
              : 0;

      BMG_AnalyzeResult_t *pAnalyzeResult =
          &pBmTest->goldenAnalyzeResult[j][i + bufferOffset];

      assert(!BMBUF_Save(
          pCtx->hHal, goldenFileName,
          pBmTest->pOutputBuffer[j][i + bufferOffset], pAnalyzeResult,
          &pBmTest->goldenBaseAddress[j][i + bufferOffset],
          pBmTest->isPingPangBuffer,
          cJSON_GetObjectItem(pReport, basename(goldenFileName))));
    }
  }

  if (pBmTest->isDstAutoShadow) {
    pBmTest->frameNumberDstAutoShadow++;
  }
}

void IP_RefreshBegin(TestCtx_t *pCtx) {
  uint32_t value = 0;

#if defined OP_SET_LUT_ADDR_MANUALLY
  TRACE(BM_TEST_INF,
        COLOR_STRING(COLOR_BLUE, "REG extra ops: set LUT address \n"));

  IP_Hack(MAP_LUT_ADDR, pCtx->pBmTest, &value);
  assert(value);

  REG_Write(pCtx, REG_BASE(CORE_INDEX) + MAP_LUT_ADDR, value);
#endif
}

void IP_RefreshAfter(TestCtx_t *pCtx) {
  uint32_t value = 0;

#if defined OP_DEWARP_START_MANUALLY
  TRACE(BM_TEST_INF,
        COLOR_STRING(COLOR_BLUE,
                     "REG extra ops: trigger dewarp start manually \n"));

  value = REG_Read(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX));

  REG_Write(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX),
            value | REG_BASE_DWE_CTRL_MASK_DEWARP_START);

  REG_Write(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX),
            value & ~REG_BASE_DWE_CTRL_MASK_DEWARP_START);
#endif
}

void IP_Reset(TestCtx_t *pCtx) {
  TRACE(BM_TEST_INF, COLOR_STRING(COLOR_BLUE, "Reset dewarp core %d ... \n"),
        CORE_INDEX);

  uint32_t value = REG_Read(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX));

  REG_Write(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX),
            value & ~REG_BASE_DWE_CTRL_MASK_DEWARP_RESET);

  REG_Write(pCtx, REG_BASE_DWE_CTRL(CORE_INDEX),
            value | REG_BASE_DWE_CTRL_MASK_DEWARP_RESET);
}

void IP_WaitForReady(TestCtx_t *pCtx) {
  while (!(REG_Read(pCtx, REG_BASE(CORE_INDEX) + INTERRUPT_STATUS) &
           INTERRUPT_STATUS_MASK_INT_FRAME_DONE)) {
    IP_Error(pCtx);

    sleep(1);
  }

#if defined OP_AXI_MASTER_DISABLE
  TRACE(BM_TEST_INF,
        COLOR_STRING(COLOR_BLUE, "REG extra ops: disable AXI master \n"));

  uint32_t value = REG_Read(pCtx, REG_BASE(CORE_INDEX) + BUS_CTRL);

  REG_Write(pCtx, REG_BASE(CORE_INDEX) + BUS_CTRL,
            value & ~BUS_CTRL_MASK_AXI_MASTER_ENABLE);
#endif
}
