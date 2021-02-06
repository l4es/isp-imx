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

#include "bm/ip/parse.h"
#include "bm/base.h"
#include "json/cJSON.h"
#include "json/json-app.h"
#include <assert.h>

int32_t IP_Parse(cJSON *pJObject, TestCase_t *pTestCase) {
  assert(pJObject);
  assert(pTestCase);

  char casePath[MAX_LEN] = {0};

  snprintf(casePath, sizeof(casePath), "%s/data/%s", pTestCase->path,
           pTestCase->pathBase);

  for (int32_t i = 0; i < cJSON_GetArraySize(pJObject) && i < CONFIG_MAX; i++) {
    cJSON *pChildItem = cJSON_GetArrayItem(pJObject, i);

    BMTestConfig_t *pTestConfig = &pTestCase->config[i].bm;

    cJSON *pJItem = NULL;

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_INPUT))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        IPPAS_ParseFiles(pJItem, casePath, CHANNEL_INPUT_MAX,
                         pTestConfig->fileNameInput[0]);
      }
    }

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_OUTPUT))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        IPPAS_ParseFiles(pJItem, casePath, CHANNEL_OUTPUT_MAX,
                         pTestConfig->fileNameOutput[0]);
      } else {
        // TODO
      }
    }

    if ((pJItem = cJSON_GetObjectItem(pChildItem, JA_REGISTER))) {
      if ((pJItem = cJSON_GetObjectItem(pJItem, JA_FILE))) {
        snprintf(pTestConfig->fileNameRegister, MAX_LEN, "%s/%s" TXT, casePath,
                 cJSON_GetObjectString(cJSON_GetArrayItem(pJItem, 0)));
      }
    }

    pTestCase->configCount++;
  }

  return 0;
}
