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

#include "result.h"
#include "file/file.h"
#include "json/cJSON.h"
#include "json/json-app.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LEN 256

#define FILENAME_TEST_RESULT "test-result"
#define FILENAME_TEST_RESULT_EXT ".json"

#define FILENAME_FAILED "failed-list"
#define FILENAME_FAILED_EXT ".txt"

static void getTimestampDesc(char *, int);

const char *RES_GetResultDescription(CaseResult_t caseResult) {
  if (caseResult == CASE_INVALID) {
    return "INVALID";
  } else if (caseResult == CASE_PASS) {
    return "PASS";
  } else if (caseResult == CASE_FAILED) {
    return "FAILED";
  } else {
    return "UNKNOWN";
  }
}

void RES_OutputResult(cJSON *pReport) {
  char timestampDesc[30] = {0};

  getTimestampDesc(timestampDesc, sizeof(timestampDesc));

  char file[MAX_LEN] = {0};

  sprintf(file, FILENAME_TEST_RESULT "-%s" FILENAME_TEST_RESULT_EXT,
          timestampDesc);

  FILE *pF = FLE_Open(file, "w");
  assert(pF);

  char *pDesc = cJSON_Print(pReport);

  fwrite(pDesc, strlen(pDesc), 1, pF);

  free(pDesc);

  FLE_Close(pF);
}

void RES_OutputFailedList(cJSON *pReport) {
  char timestampDesc[30] = {0};

  getTimestampDesc(timestampDesc, sizeof(timestampDesc));

  char file[MAX_LEN] = {0};

  sprintf(file, FILENAME_FAILED "-%s" FILENAME_FAILED_EXT, timestampDesc);

  FILE *pF = FLE_Open(file, "w");
  assert(pF);

  int size = cJSON_GetArraySize(pReport);

  char desc[MAX_LEN] = "";

  for (int i = 0; i < size; i++) {
    cJSON *pItem = cJSON_GetArrayItem(pReport, i);

    cJSON *pVerify = cJSON_GetObjectItem(pItem, JA_VERIFY);
    if (pVerify) {
      if (strcmp("FAILED", cJSON_GetObjectString(pVerify))) {
        continue;
      }

      cJSON *pName = cJSON_GetObjectItem(pItem, JA_NAME);
      if (pName) {
        snprintf(desc, sizeof(desc), "%s\n", cJSON_GetObjectString(pName));
        fwrite(desc, strlen(desc), 1, pF);
      }
    }
  }

  FLE_Close(pF);
}

static void getTimestampDesc(char *pDesc, int length) {
  time_t timer;

  time(&timer);

  struct tm *pTm = localtime(&timer);

  strftime(pDesc, length, "%Y%m%d%H%M%S", pTm);
}
