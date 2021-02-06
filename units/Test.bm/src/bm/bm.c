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

#include "bm/bm.h"
#include "bash-color.h"
#include "bm/abstract.h"
#include "bm/base.h"
#include "bm/golden.h"
#include "json/json-app.h"
#include <assert.h>

typedef struct {
  TestCtx_t *pCtx;
  BMTestConfig_t *pBmTestConfig;
  cJSON *pReport;
} BM_FrameTestParam_t;

static int32_t frameTest(BM_FrameTestParam_t *pParam);

void BM_ParseConfig(TestCtx_t *pCtx, cJSON *pJObject) {
  assert(pCtx);
  assert(pJObject);

  BMA_ConfigParse(pCtx, pJObject);
}

void BM_TestCase(TestCtx_t *pCtx, cJSON *pReport) {
  assert(pCtx);
  assert(pReport);

  for (int32_t j = 0; j < CORE_MAX; j++) {
    if (!pCtx->pBmTest) {
      BMA_Create(pCtx);
    }

    BMA_BuffersInit(pCtx);
    BMA_BuffersCheck(pCtx);

    BMTest_t *pBmTest = pCtx->pBmTest;
    TestCase_t *pTestCase = &pCtx->testCase;

    pBmTest->coreIndex = j;

    TRACE(BM_TEST_INF, COLOR_STRING(COLOR_BLUE, "Test core %d >>> \n"), j);

    BMA_Reset(pCtx);

    for (int32_t i = 0; i < pTestCase->configCount; i++) {
      pBmTest->frameNumber = i;

      BM_FrameTestParam_t frameTestParam = {
          .pCtx = pCtx,
          .pBmTestConfig = &pTestCase->config[i].bm,
          .pReport = pReport,
      };

      frameTest(&frameTestParam);
    }

    if (pCtx->pBmTest) {
      BMA_Free(pCtx);
    }

    TRACE(BM_TEST_INF, COLOR_STRING(COLOR_BLUE, "Test core %d <<< \n"), j);
  }
}

static int32_t frameTest(BM_FrameTestParam_t *pParam) {
  assert(pParam);

  TestCtx_t *pCtx = pParam->pCtx;
  assert(pCtx);

  cJSON *pReport = pParam->pReport;

  cJSON *pFrame = cJSON_CreateObject();
  cJSON_AddItemToArray(pReport, pFrame);

  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  cJSON_AddNumberToObject(pFrame, JA_CORE_INDEX, pBmTest->coreIndex);
  cJSON_AddNumberToObject(pFrame, JA_FRAME, pBmTest->frameNumber);

  BMTestConfig_t *pBmTestConfig = pParam->pBmTestConfig;
  assert(pBmTestConfig);

  cJSON *pGolden = cJSON_CreateObject();
  cJSON_AddItemToObject(pFrame, JA_GOLDEN, pGolden);

  BMA_Golden(pCtx, pBmTestConfig, pGolden);

  cJSON *pInput = cJSON_CreateObject();
  cJSON_AddItemToObject(pFrame, JA_INPUT, pInput);

  BMA_InputLoad(pCtx, pBmTestConfig, pInput);

  cJSON *pRegister = cJSON_CreateObject();
  cJSON_AddItemToObject(pFrame, JA_REGISTER, pRegister);

  int32_t ret = BMA_Refresh(pCtx, pBmTestConfig, pRegister);
  if (ret) {
    return ret;
  }

  cJSON *pOutput = cJSON_CreateObject();
  cJSON_AddItemToObject(pFrame, JA_OUTPUT, pOutput);

  BMA_GoldenAnalyze(pCtx, pBmTestConfig, pOutput);

  BMA_OutputSave(pCtx, pBmTestConfig, pOutput);

  BMA_OutputVerify(pCtx, pBmTestConfig, pOutput);

  return 0;
}
