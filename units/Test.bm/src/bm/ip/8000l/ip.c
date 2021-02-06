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

#include "bm/base.h"
#include "bm/buffer.h"
#include "file/file.h"
#include "json/json-app.h"
#include <assert.h>
#include <common/picture_buffer.h>
#include <libgen.h>

#define MRV_MI_BASE 0x1400
#define MI_DMA_Y_PIC_START_AD (MRV_MI_BASE + 0x00C8)
#define MI_DMA_CB_PIC_START_AD (MRV_MI_BASE + 0x00D8)
#define MI_DMA_CR_PIC_START_AD (MRV_MI_BASE + 0x00E8)

#define MI_MP_Y_BASE_AD_INIT (MRV_MI_BASE + 0x0008)
#define MI_MP_Y_BASE_AD_INIT2 (MRV_MI_BASE + 0x0130)
// #define MI_MP_Y_BASE_AD (MRV_MI_BASE + 0x0078)

#define MI_MP_CB_BASE_AD_INIT (MRV_MI_BASE + 0x001C)
#define MI_MP_CB_BASE_AD_INIT2 (MRV_MI_BASE + 0x0134)
// #define MI_MP_CB_BASE_AD (MRV_MI_BASE + 0x0088)

#define MI_MP_CR_BASE_AD_INIT (MRV_MI_BASE + 0x002C)
#define MI_MP_CR_BASE_AD_INIT2 (MRV_MI_BASE + 0x0138)
// #define MI_MP_CR_BASE_AD (MRV_MI_BASE + 0x0094)

#define MI_SP_Y_BASE_AD_INIT (MRV_MI_BASE + 0x003C)
#define MI_SP_Y_BASE_AD_INIT2 (MRV_MI_BASE + 0x013C)
// #define MI_SP_Y_BASE_AD (MRV_MI_BASE + 0x00A0)

#define MI_SP_CB_BASE_AD_INIT (MRV_MI_BASE + 0x0050)
#define MI_SP_CB_BASE_AD_INIT2 (MRV_MI_BASE + 0x0140)
// #define MI_SP_CB_BASE_AD (MRV_MI_BASE + 0x00B0)

#define MI_SP_CR_BASE_AD_INIT (MRV_MI_BASE + 0x0060)
#define MI_SP_CR_BASE_AD_INIT2 (MRV_MI_BASE + 0x0144)
// #define MI_SP_CR_BASE_AD (MRV_MI_BASE + 0x00BC)

#define MI_CTRL (MRV_MI_BASE + 0x0000)

#define MRV_MI_CTRL_PATH_ENABLE_MASK 0x0000000FU

#define MI_MIS (MRV_MI_BASE + 0x0100)

#define MRV_MI_MP_FRAME_END_MASK 0x00000001U
#define MRV_MI_SP_FRAME_END_MASK 0x00000002U

#define PIC_BUFFER_ALIGN (0x400)

#define REG_BASE_TPG(_X_) (0x00308300)

void IP_Create(TestCtx_t *pCtx) {
  assert(pCtx);

  assert(!pCtx->pBmTest);
  pCtx->pBmTest = calloc(sizeof(BMTest_t), 1);
  assert(pCtx->pBmTest);

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

  for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
    MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer(pBufPool);
    assert(pBuffer);

    pBmTest->pInputBuffer[0][i] = pBuffer;
  }

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      MediaBuffer_t *pBuffer = MediaBufPoolGetBuffer(pBufPool);
      assert(pBuffer);

      pBmTest->pOutputBuffer[j][i] = pBuffer;
      pBmTest->goldenBaseAddress[j][i] = GOLDEN_BASE_ADDRESS_INIT;
    }
  }
}

void IP_Free(TestCtx_t *pCtx) {
  assert(pCtx);

  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  BMCmdBuffer_t *pCmdBuffer = &pBmTest->cmdBuffer;

  if (pCmdBuffer->registerOp) {
    free(pCmdBuffer->registerOp);
    pCmdBuffer->registerOp = NULL;
  }

  pCmdBuffer->cmdNum = 0;

  for (int32_t j = 0; j < PATH_INPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
      if (pBmTest->pInputBuffer[j][i]) {
        MediaBufPoolFreeBuffer(&pBmTest->memPool.bufPool,
                               pBmTest->pInputBuffer[j][i]);
        pBmTest->pInputBuffer[j][i] = NULL;
      }
    }
  }

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

  free(pCtx->pBmTest);
  pCtx->pBmTest = NULL;
}

void IP_GoldenAnalyze(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                      cJSON *pReport) {
  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  int32_t ret = 0;

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[j][0])) {
      continue;
    }

    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      char *pGoldenFileName = pTestConfig->fileNameOutput[j][i];

      if (!strlen(pGoldenFileName)) {
        break;
      }

      cJSON *pItem = cJSON_CreateObject();
      cJSON_AddItemToObject(pReport, basename(pGoldenFileName), pItem);

      BMG_AnalyzeResult_t *pAnalyzeResult =
          &pCtx->pBmTest->goldenAnalyzeResult[j][i];

      ret = BMG_AnalyzeBuffer(pGoldenFileName, pAnalyzeResult);
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

void IP_Hack(uint64_t address, BMTest_t *pBmTest, uint32_t *pValue) {
  switch (address) {
  case MI_DMA_Y_PIC_START_AD:
    *pValue = pBmTest->pInputBuffer[0][0]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_DMA_Y_PIC_START_AD = 0x%X \n", *pValue);
    break;

  case MI_DMA_CB_PIC_START_AD:
    *pValue = pBmTest->pInputBuffer[0][1]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_DMA_CB_PIC_START_AD = 0x%X \n", *pValue);
    break;

  case MI_DMA_CR_PIC_START_AD:
    *pValue = pBmTest->pInputBuffer[0][2]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_DMA_CR_PIC_START_AD = 0x%X \n", *pValue);
    break;

  case MI_MP_Y_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][0]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_Y_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_MP_CB_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][1]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_CB_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_MP_CR_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[0][2]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_CR_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_MP_Y_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][0]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_Y_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  case MI_MP_CB_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][1]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_CB_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  case MI_MP_CR_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[0][2]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_MP_CR_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  case MI_SP_Y_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[1][0]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_Y_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_SP_CB_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[1][1]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_CB_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_SP_CR_BASE_AD_INIT:
    *pValue = pBmTest->pOutputBuffer[1][2]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_CR_BASE_AD_INIT = 0x%X \n", *pValue);
    break;

  case MI_SP_Y_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[1][0]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_Y_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  case MI_SP_CB_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[1][1]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_CB_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  case MI_SP_CR_BASE_AD_INIT2:
    *pValue = pBmTest->pOutputBuffer[1][2]->buf.base_address;
    TRACE(BM_TEST_INF, "MI_SP_CR_BASE_AD_INIT2 = 0x%X \n", *pValue);
    pBmTest->isPingPangBuffer = true;
    break;

  default:
    break;
  }
}

void IP_WaitForReady(TestCtx_t *pCtx) {
  uint32_t val = HalReadReg(pCtx->hHal, MI_CTRL) & MRV_MI_CTRL_PATH_ENABLE_MASK;

  bool isMp = false;
  bool isSp = false;

  if (val == 0x01) {
    isMp = true;
  } else if (val == 0x02) {
    isSp = true;
  } else if (val == 0x03) {
    isMp = true;
    isSp = true;
  }

#ifdef IP_NANO
  if (isSp) {
    TRACE(ISP_BM_TEST_ERROR, "SP channel is opened, ignore this issue. \n");
    isSp = false;
  }
#endif

  /* Wait for HW to complete with main path. */
  while (isMp && !(HalReadReg(pCtx->hHal, MI_MIS) & MRV_MI_MP_FRAME_END_MASK)) {
    sleep(1);
  }

  while (isSp && !(HalReadReg(pCtx->hHal, MI_MIS) & MRV_MI_SP_FRAME_END_MASK)) {
    sleep(1);
  }
}

void IP_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                  cJSON *pReport) {
  uint64_t size = 0;

  int32_t ret = 0;

  for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
    char *pFileName = pTestConfig->fileNameInput[0][i];
    if (!strlen(pFileName)) {
      break;
    }

    ret = FLE_Size(pFileName, &size);
    assert(!ret);

    if (size == 0) {
      continue;
    }

    ret = BMBUF_Load(pCtx->hHal, pFileName, pCtx->pBmTest->pInputBuffer[0][i],
                     pReport);
    assert(!ret);
  }
}

void IP_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport) {
  int32_t ret = 0;

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[j][0])) {
      continue;
    }

    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      char *pGoldenFileName = pTestConfig->fileNameOutput[j][i];

      if (!strlen(pGoldenFileName)) {
        break;
      }

      cJSON *pItem = cJSON_CreateObject();
      cJSON_AddItemToObject(pReport, basename(pGoldenFileName), pItem);

      BMG_AnalyzeResult_t *pAnalyzeResult =
          &pCtx->pBmTest->goldenAnalyzeResult[j][i];

      ret = BMG_AnalyzeBuffer(pGoldenFileName, pAnalyzeResult);
      if (ret < 0) {
        cJSON_AddFalseToObject(pItem, "is.exist");
        return ret;
      }

      if (!pAnalyzeResult->bufferLength) {
        cJSON_AddTrueToObject(pItem, "is.empty.file");
        continue;
      }

      if (pAnalyzeResult->isRollback) {
        cJSON_AddTrueToObject(pItem, "is.rollback");

        char desc[100] = {0};

        if (pAnalyzeResult->minAddr > 0) {
          snprintf(desc, sizeof(desc), "0x%lX", pAnalyzeResult->minAddr);
          cJSON_AddStringToObject(pItem, "output.offset", desc);
        }

        snprintf(desc, sizeof(desc), "0x%lX", pAnalyzeResult->bufferLength);
        cJSON_AddStringToObject(pItem, JA_BUFFER_LENGTH, desc);
      }

      if (pAnalyzeResult->isNonlinear) {
        cJSON_AddTrueToObject(pItem, "is.nonlinear");
      }

      BMTest_t *pBmTest = pCtx->pBmTest;
      assert(pBmTest);

      MediaBuffer_t *pMediaBuffer = pBmTest->pOutputBuffer[j][i];
      assert(pMediaBuffer);

      ret =
          BMBUF_Save(pCtx->hHal, pGoldenFileName, pBmTest->pOutputBuffer[j][i],
                     &pBmTest->goldenAnalyzeResult[j][i],
                     &pBmTest->goldenBaseAddress[j][i],
                     pBmTest->isPingPangBuffer, pReport);
      assert(!ret);
    }
  }
}

void IP_Reset(TestCtx_t *pCtx) {
  uint32_t data = HalReadReg(pCtx->hHal, REG_BASE_TPG(i));

  data &= ~REG_BASE_TPG_MASK_ISP_RST_N;
  HalWriteReg(pCtx->hHal, REG_BASE_TPG(i), data);

  data |= REG_BASE_TPG_MASK_ISP_RST_N;
  HalWriteReg(pCtx->hHal, REG_BASE_TPG(i), data);
}
