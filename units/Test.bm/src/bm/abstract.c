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
#include "bm/golden.h"
#include "bm/ip/ip.h"
#include "file/file.h"
#include "hal.h"
#include "json/json-app.h"
#include <assert.h>
#include <common/picture_buffer.h>
#include <libgen.h>
#include <unistd.h>

#define REG_WRITE 0
#define REG_READ 1

#define BUFFER_INIT_VALUE 0xFF

static void initBuffer(HalHandle_t, MediaBuffer_t *);
static void checkBuffer(HalHandle_t, MediaBuffer_t *);

void BMA_BuffersInit(TestCtx_t *pCtx) {
#if defined JUST_VERIFY_CASE_FILES
  return;
#endif

  assert(pCtx);

  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  for (int32_t j = 0; j < PATH_INPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
      MediaBuffer_t *pMediaBuffer = pBmTest->pInputBuffer[j][i];
      if (pMediaBuffer) {
        initBuffer(pCtx->hHal, pMediaBuffer);
      }
    }
  }

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      MediaBuffer_t *pMediaBuffer = pBmTest->pOutputBuffer[j][i];
      if (pMediaBuffer) {
        initBuffer(pCtx->hHal, pMediaBuffer);
      }
    }
  }
}

void BMA_BuffersCheck(TestCtx_t *pCtx) {
#if defined JUST_VERIFY_CASE_FILES
  return;
#endif

  assert(pCtx);

  BMTest_t *pBmTest = pCtx->pBmTest;
  assert(pBmTest);

  for (int32_t j = 0; j < PATH_INPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_INPUT_MAX; i++) {
      MediaBuffer_t *pMediaBuffer = pBmTest->pInputBuffer[j][i];
      if (pMediaBuffer) {
        checkBuffer(pCtx->hHal, pMediaBuffer);
      }
    }
  }

  for (int32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    for (int32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      MediaBuffer_t *pMediaBuffer = pBmTest->pOutputBuffer[j][i];
      if (pMediaBuffer) {
        checkBuffer(pCtx->hHal, pMediaBuffer);
      }
    }
  }
}

void BMA_ConfigParse(TestCtx_t *pCtx, cJSON *pJObject) {
  assert(pCtx);
  assert(pJObject);

  IP_Parse(pCtx, pJObject);
}

void BMA_Create(TestCtx_t *pCtx) {
  assert(pCtx);

  IP_Create(pCtx);
}

void BMA_Free(TestCtx_t *pCtx) {
  assert(pCtx);

  IP_Free(pCtx);
}

void BMA_Golden(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig, cJSON *pReport) {
  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  IP_Golden(pCtx, pTestConfig, pReport);
}

void BMA_GoldenAnalyze(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                       cJSON *pReport) {
  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  IP_GoldenAnalyze(pCtx, pTestConfig, pReport);
}

void BMA_InputLoad(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                   cJSON *pReport) {
  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  IP_InputLoad(pCtx, pTestConfig, pReport);
}

void BMA_OutputSave(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                    cJSON *pReport) {
#if defined JUST_VERIFY_CASE_FILES
  return;
#endif

  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  IP_OutputSave(pCtx, pTestConfig, pReport);
}

void BMA_OutputVerify(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                      cJSON *pReport) {
#if defined JUST_VERIFY_CASE_FILES
  return;
#endif

  assert(pCtx);
  assert(pTestConfig);
  assert(pReport);

  int32_t coreIndex = pCtx->pBmTest->coreIndex;

  bool isVerified = true;

  for (uint32_t j = 0; j < PATH_OUTPUT_MAX; j++) {
    if (!strlen(pTestConfig->fileNameOutput[coreIndex][j][0])) {
      continue;
    }

    for (uint32_t i = 0; i < CHANNEL_OUTPUT_MAX; i++) {
      if (!strlen(pTestConfig->fileNameOutput[coreIndex][j][i])) {
        break;
      }

      char goldenFileName[MAX_LEN] = {0};

      snprintf(goldenFileName, sizeof(goldenFileName), "%s/%s",
               pCtx->testCase.pathOutput,
               pTestConfig->fileNameOutput[coreIndex][j][i]);

      cJSON *pItem = cJSON_GetObjectItem(pReport, basename(goldenFileName));

      BMG_AnalyzeResult_t *pAnalyzeResult =
          &pCtx->pBmTest->goldenAnalyzeResult[j][i];

      if (!pAnalyzeResult->bufferLength) {
        cJSON_AddTrueToObject(pItem, JA_IS_EMPTY_FILE);
        continue;
      }

      assert(!BMBUF_Verify(goldenFileName, pAnalyzeResult, pItem));

      if (strcmp(
              cJSON_GetObjectString(cJSON_GetObjectItem(pItem, JA_MATCH_RATE)),
              "OK")) {
        isVerified = false;
      }
    }
  }

  cJSON_AddBoolToObject(pReport, JA_IS_VERIFY, isVerified);
}

int32_t BMA_Refresh(TestCtx_t *pCtx, BMTestConfig_t *pTestConfig,
                    cJSON *pReport) {
  assert(pCtx);
  assert(pTestConfig);

  BMTest_t *pBmTest = pCtx->pBmTest;

  int32_t coreIndex = pBmTest->coreIndex;

  if (!strlen(pTestConfig->fileNameRegister[coreIndex])) {
    cJSON *pItem = cJSON_CreateObject();
    cJSON_AddItemToObject(pReport, "register.file", pItem);

    cJSON_AddBoolToObject(pItem, JA_IS_EXIST, false);
    return -ENFILE;
  }

  char fileName[MAX_LEN] = {0};

  snprintf(fileName, sizeof(fileName), "%s/%s", pCtx->testCase.pathInput,
           pTestConfig->fileNameRegister[coreIndex]);

  cJSON *pItem = cJSON_CreateObject();
  cJSON_AddItemToObject(pReport, basename(fileName), pItem);

  FILE *pF = FLE_Open(fileName, "r");
  if (!pF) {
    cJSON_AddBoolToObject(pItem, JA_IS_EXIST, false);
    return -ENFILE;
  }

#if defined JUST_VERIFY_CASE_FILES
  FLE_Close(pF);
  return 0;
#endif

  IP_RefreshBegin(pCtx);

  uint32_t ops = 0, address = 0, value = 0, value2 = 0, count = 0;

  cJSON *pJArrayDesc = cJSON_CreateArray();
  assert(pJArrayDesc);
  cJSON_AddItemToObject(pItem, JA_DESCRIPTIONS, pJArrayDesc);

  while (fscanf(pF, "%u %x %x", &ops, &address, &value) > 0) {
    value2 = value;

    IP_Hack(pCtx, address, pBmTest, &value, pJArrayDesc);

    if (ops == REG_WRITE) {
      REG_Write(pCtx, REG_BASE(coreIndex) + address, value);
    } else {
      REG_Read(pCtx, REG_BASE(coreIndex) + address);
    }

    count++;
  }

  FLE_Close(pF);

  cJSON_AddNumberToObject(pItem, JA_REGISTER_COUNT, count);

  if (pBmTest->isPingPangBuffer) {
    cJSON_AddTrueToObject(pItem, JA_IS_PING_PANG_BUFFER);
  }

  if (pBmTest->isDstAutoShadow) {
    cJSON_AddTrueToObject(pItem, JA_IS_DST_AUTO_SHADOW);
  }

  IP_RefreshAfter(pCtx);

  IP_Error(pCtx);

  IP_WaitForReady(pCtx);

  return 0;
}

void BMA_Reset(TestCtx_t *pCtx) {
#if defined JUST_VERIFY_CASE_FILES
  return;
#endif

  assert(pCtx);

  REG_Reset();

  IP_Reset(pCtx);
}

static void initBuffer(HalHandle_t hHal, MediaBuffer_t *pMediaBuffer) {
  assert(hHal);
  assert(pMediaBuffer);

  uint8_t *pMapBuffer = NULL;

  RESULT ret =
      HalMapMemory(hHal, pMediaBuffer->buf.base_address, pMediaBuffer->buf.size,
                   HAL_MAPMEM_WRITEONLY, (void *)&pMapBuffer);
  assert(ret == RET_SUCCESS);

  memset(pMapBuffer, BUFFER_INIT_VALUE, pMediaBuffer->buf.size);

  ret = HalUnMapMemory(hHal, pMapBuffer);
  assert(ret == RET_SUCCESS);
}

static void checkBuffer(HalHandle_t hHal, MediaBuffer_t *pMediaBuffer) {
  assert(hHal);
  assert(pMediaBuffer);

  uint8_t *pMapBuffer = NULL;

  RESULT ret =
      HalMapMemory(hHal, pMediaBuffer->buf.base_address, pMediaBuffer->buf.size,
                   HAL_MAPMEM_WRITEONLY, (void *)&pMapBuffer);
  assert(ret == RET_SUCCESS);

  for (uint32_t i = 0; i < pMediaBuffer->buf.size; i++) {
    assert(*(pMapBuffer + i) == BUFFER_INIT_VALUE);
  }

  ret = HalUnMapMemory(hHal, pMapBuffer);
  assert(ret == RET_SUCCESS);
}
