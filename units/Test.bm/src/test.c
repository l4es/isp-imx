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
#include "bm/bm.h"
#include "list.h"
#include "parse.h"
#include "result.h"
#include "json/cJSON.h"
#include "json/json-app.h"
#include <assert.h>
#include <libgen.h>
#include <stdio.h>

#define MAX_LEN 256

static CaseResult_t test(TestCtx_t *, TestCase_t *, cJSON *);
static bool isAllFilesExist(cJSON *pFiles);

void TST_Do(TestCtx_t *pCtx, const char *pFileName) {
  int32_t ret = 0;

  cJSON *pReport = cJSON_CreateArray();

  int32_t passCount = 0;
  int32_t invalidCount = 0;
  int32_t failedCount = 0;

  char **ppCaseList = NULL;
  int32_t caseCount = 0;

  ret = LST_Load(pFileName, &ppCaseList, &caseCount);
  assert(!ret);

  snprintf(pCtx->pathInputRoot, sizeof(pCtx->pathInputRoot), "%s", pFileName);
  dirname(pCtx->pathInputRoot);

  time_t timeStart, timeEnd;

  for (int32_t i = 0; i < caseCount; i++) {
    time(&timeStart);

    memset(&pCtx->testCase, 0, sizeof(pCtx->testCase));

    char caseFilename[MAX_LEN] = "";

    snprintf(caseFilename, sizeof(caseFilename), "%s", ppCaseList[i]);

    TRACE(BM_TEST_INF,
          COLOR_STRING(COLOR_BLUE, "CASE [%03d/%03d]: %s START...\n"), i + 1,
          caseCount, basename(ppCaseList[i]));

    cJSON *pCase = cJSON_CreateObject();
    cJSON_AddItemToArray(pReport, pCase);

    cJSON_AddItemToObject(pCase, JA_NAME,
                          cJSON_CreateString(basename(ppCaseList[i])));

    TestCase_t *pTestCase = &pCtx->testCase;
    cJSON *pJRoot = NULL;
    CaseResult_t caseResult = CASE_UNKNOWN;

    if ((pJRoot = JA_ReadFromFile(caseFilename))) {
      strncpy(pTestCase->pathInput, pFileName, sizeof(pTestCase->pathInput));
      dirname(pTestCase->pathInput);

      ret = PAS_ParseTestCase(pCtx, pJRoot);
      if (!ret) {
        caseResult = test(pCtx, pTestCase, pCase);
      } else {
        cJSON_AddFalseToObject(pCase, JA_IS_JSON_VALID);
        caseResult = CASE_INVALID;
      }

      cJSON_Delete(pJRoot);
    } else {
      cJSON_AddFalseToObject(pCase, JA_IS_EXIST);
      caseResult = CASE_INVALID;
    }

    if (caseResult == CASE_PASS) {
      passCount++;
    } else if (caseResult == CASE_INVALID) {
      invalidCount++;
    } else if (caseResult == CASE_FAILED) {
      failedCount++;
    }

    time(&timeEnd);

    double timeSpan = difftime(timeEnd, timeStart);

    TRACE(BM_TEST_INF,
          (caseResult == CASE_PASS)
              ? COLOR_STRING(COLOR_GREEN, "CASE [%03d/%03d]: %s = %s, %.1fs \n")
              : COLOR_STRING(COLOR_RED, "CASE [%03d/%03d]: %s = %s, %.1fs \n"),
          i + 1, caseCount, basename(ppCaseList[i]),
          RES_GetResultDescription(caseResult), timeSpan);

    cJSON_AddItemToObject(
        pCase, JA_VERIFY,
        cJSON_CreateString(RES_GetResultDescription(caseResult)));

    char desc[MAX_LEN] = {0};

    uint32_t offset = snprintf(desc, sizeof(desc), "%d/%d:", i + 1, caseCount);

    if (passCount) {
      offset += snprintf(desc + offset, sizeof(desc),
                         COLOR_STRING(COLOR_GREEN, " PASS %d,"), passCount);
    }

    if (invalidCount) {
      offset +=
          snprintf(desc + offset, sizeof(desc) - offset,
                   COLOR_STRING(COLOR_YELLOW, " INVALID %d,"), invalidCount);
    }

    if (failedCount) {
      offset += snprintf(desc + offset, sizeof(desc) - offset,
                         COLOR_STRING(COLOR_RED, " FAILED %d."), failedCount);
    }

    TRACE(BM_TEST_INF, "%s \n", desc);
  }

  char totalDesc[100] = "";

  snprintf(totalDesc, sizeof(totalDesc), "PASS %d/%d: INVALID %d, FAILED %d",
           passCount, caseCount, invalidCount, failedCount);

  cJSON_AddStringToObject(pReport, "total", totalDesc);

  RES_OutputResult(pReport);

  RES_OutputFailedList(pReport);

  cJSON_Delete(pReport);

  CL_FreeCaseList(ppCaseList, caseCount);
}

static CaseResult_t test(TestCtx_t *pCtx, TestCase_t *pTestCase, cJSON *pCase) {
  cJSON *pFrames = cJSON_CreateArray();
  cJSON_AddItemToObject(pCase, JA_FRAMES, pFrames);

  BM_TestCase(pCtx, pFrames);

  int32_t verifyCount = 0;

  for (int32_t i = 0; i < cJSON_GetArraySize(pFrames); i++) {
    cJSON *pFrame = cJSON_GetArrayItem(pFrames, i);
    cJSON *pFiles = NULL;

    if ((pFiles = cJSON_GetObjectItem(pFrame, JA_REGISTER))) {
      if (!isAllFilesExist(pFiles)) {
        verifyCount++; // skip test
      }
    }

    if ((pFiles = cJSON_GetObjectItem(pFrame, JA_INPUT))) {
      if (!isAllFilesExist(pFiles)) {
        return CASE_INVALID;
      }
    }

    if ((pFiles = cJSON_GetObjectItem(pFrame, JA_OUTPUT))) {
      if (!isAllFilesExist(pFiles)) {
        return CASE_INVALID;
      }

      cJSON *pItem = NULL;

      if ((pItem = cJSON_GetObjectItem(pFiles, JA_IS_VERIFY)) &&
          pItem->type == cJSON_True) {
        verifyCount++;
      }
    }
  }

  return (verifyCount == cJSON_GetArraySize(pFrames)) ? CASE_PASS : CASE_FAILED;
}

static bool isAllFilesExist(cJSON *pFiles) {
  cJSON *pChild = pFiles->child;

  while (pChild) {
    cJSON *pIsExist = NULL;
    cJSON *pIsValid = NULL;

    if ((pIsExist = cJSON_GetObjectItem(pChild, JA_IS_EXIST)) &&
        pIsExist->type == cJSON_False) {
      return false;
    }

    if ((pIsValid = cJSON_GetObjectItem(pChild, JA_IS_VALID)) &&
        pIsValid->type == cJSON_False) {
      return false;
    }

    pChild = pChild->next;
  }

  return true;
}
