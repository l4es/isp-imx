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
#include "bm/bm.h"
#include "json/cJSON.h"
#include "json/json-app.h"
#include <assert.h>
#include <libgen.h>

int PAS_ParseTestCase(TestCtx_t *pCtx, cJSON *pJObject) {
  assert(pCtx);
  assert(pJObject);

  pJObject = cJSON_GetObjectItem(pJObject, JA_CASE_CONFIG);
  assert(pJObject);

  TestCase_t *pTestCase = &pCtx->testCase;
  cJSON *pJItem = NULL;

  if ((pJItem = cJSON_GetObjectItem(pJObject, JA_PATH))) {
    strncpy(pTestCase->name, cJSON_GetObjectString(pJItem), MAX_LEN);
  }

  if ((pJItem = cJSON_GetObjectItem(pJObject, JA_BM_CONFIG))) {
    BM_ParseConfig(pCtx, pJItem);
  }

  return 0;
}
