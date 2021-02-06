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
#include "json/cJSON.h"
#include "json/json-app.h"
#include <assert.h>
#include <libgen.h>

int32_t IP_Parse(TestCtx_t *pCtx, cJSON *pJObject) {
  assert(pCtx);
  assert(pJObject);

  TestCase_t *pTestCase = &pCtx->testCase;

  snprintf(pTestCase->pathInput, sizeof(pTestCase->pathInput), "%s/data/%s",
           pCtx->pathInputRoot, pTestCase->name);

  for (int32_t i = 0; i < cJSON_GetArraySize(pJObject) && i < FRAME_MAX; i++) {
    cJSON *pChildItem = cJSON_GetArrayItem(pJObject, i);

    BMTestConfig_t *pTestConfig = &pTestCase->config[i].bm;

    cJSON *pJItem = NULL;

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_INPUT))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        cJSON_DeleteItemFromArray(pJItem, 2); // remove y base2

        for (int32_t j = 0; j < cJSON_GetArraySize(pJItem); j++) {
          snprintf(pTestConfig->fileNameInput[0][0][j], MAX_LEN, "%s.txt",
                   cJSON_GetObjectString(cJSON_GetArrayItem(pJItem, j)));
        }
      }
    }

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_OUTPUT))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        for (int32_t j = 0; j < cJSON_GetArraySize(pJItem); j++) {
          snprintf(pTestConfig->fileNameOutput[0][0][j], MAX_LEN, "%s.txt",
                   cJSON_GetObjectString(cJSON_GetArrayItem(pJItem, j)));
        }
      }
    }

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_REGISTER))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        snprintf(pTestConfig->fileNameRegister[0], MAX_LEN, "%s.txt",
                 cJSON_GetObjectString(cJSON_GetArrayItem(pJItem, 0)));
      }
    }

    pTestCase->configCount++;
  }

  return 0;
}
