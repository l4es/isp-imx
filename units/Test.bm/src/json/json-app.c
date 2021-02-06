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

#include "file/file.h"
#include "json/cJSON.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

cJSON *JA_ReadFromFile(const char *pFilename) {
  assert(pFilename);

  cJSON *json = NULL;

  FILE *pF = FLE_Open(pFilename, "r");
  if (!pF) {
    return NULL;
  }

  fseek(pF, 0, SEEK_END);
  size_t len = ftell(pF);

  char *pData = (char *)calloc(1, len + 1);
  assert(pData);

  rewind(pF);

  size_t num = fread(pData, len, 1, pF);
  assert(num == 1);

  pData[len] = '\0';

  json = cJSON_Parse(pData);
  assert(json);

  free(pData);

  FLE_Close(pF);

  return json;
}
