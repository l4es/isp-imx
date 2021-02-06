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

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_OUTPUT))) {
      for (int32_t k = 0; k < CORE_MAX; k++) {
        cJSON *pFileItem = NULL;

        if ((pFileItem = cJSON_GetArrayItem(pJItem, k))) {
          if ((pFileItem = cJSON_GetObjectItem(pFileItem, JA_FILE))) {
            for (int32_t j = 0; j < cJSON_GetArraySize(pFileItem); j++) {
              snprintf(pTestConfig->fileNameOutput[k][0][j], MAX_LEN, "%s",
                       cJSON_GetObjectString(cJSON_GetArrayItem(pFileItem, j)));
            }
          }
        }
      }
    }

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_REGISTER))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        for (int32_t i = 0; i < CORE_MAX; i++) {
          snprintf(pTestConfig->fileNameRegister[i], MAX_LEN, "%s",
                   cJSON_GetObjectString(cJSON_GetArrayItem(pJItem, i)));
        }
      }
    }

    pTestCase->configCount++;
  }

  return 0;
}
