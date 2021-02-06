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
#include "file/file.h"
#include "json/json-app.h"
#include <assert.h>
#include <dlfcn.h>
#include <ebase/types.h>
#include <libgen.h>
#include <stdio.h>

static void *pLib;
static void (*pInitCModel)(int32_t);
static void (*pDestoryCModel)();
static void (*pPipeLine)();
static void (*pRegisterRead)(uint32_t, uint32_t *);
static void (*pRegisterWrite)(uint32_t, uint32_t);
static void (*pInputData)(uint32_t id, uint32_t mask, uint32_t *pData,
                          uint32_t index);
static void (*pInputMap)(uint32_t id, uint32_t mask, uint32_t *pData,
                         uint32_t index);

void CMDA_Create() {
  if (!pLib) {
    pLib = dlopen(CMODEL_SO_PATH, RTLD_LAZY);
    assert(pLib);

    TRACE(BM_TEST_INF, "dlopen() OK: %s\n", CMODEL_SO_PATH);
  }

  if (!pInitCModel) {
    *(void **)(&pInitCModel) = dlsym(pLib, "initialCmodel");
    assert(pInitCModel);

    (*pInitCModel)(0);
  }

  if (!pInputData) {
    *(void **)(&pInputData) = dlsym(pLib, "DataRequest");
    assert(pInputData);
  }

  if (!pInputMap) {
    *(void **)(&pInputMap) = dlsym(pLib, "MapRequest");
    assert(pInputMap);
  }

  if (!pPipeLine) {
    *(void **)(&pPipeLine) = dlsym(pLib, "ISPpipeline");
    assert(pPipeLine);
  }

  if (!pRegisterRead) {
    *(void **)(&pRegisterRead) = dlsym(pLib, "readRegister");
    assert(pRegisterRead);
  }

  if (!pRegisterWrite) {
    *(void **)(&pRegisterWrite) = dlsym(pLib, "writeRegister");
    assert(pRegisterWrite);
  }

  if (!pDestoryCModel) {
    *(void **)(&pDestoryCModel) = dlsym(pLib, "destoryCmodel");
    assert(pDestoryCModel);
  }
}

void CMDA_Free() {
  int32_t ret = 0;

  if (pDestoryCModel) {
    (*pDestoryCModel)();
  }

  if (pRegisterWrite) {
    pRegisterWrite = NULL;
  }

  if (pRegisterRead) {
    pRegisterRead = NULL;
  }

  if (pPipeLine) {
    pPipeLine = NULL;
  }

  if (pInputMap) {
    pInputMap = NULL;
  }

  if (pInputData) {
    pInputData = NULL;
  }

  if (pInitCModel) {
    pInitCModel = NULL;
  }

  if (pLib) {
    ret = dlclose(pLib);
    assert(!ret);
    pLib = NULL;

    TRACE(BM_TEST_INF, "dlclose() OK: %s \n", CMODEL_SO_PATH);
  }
}

void CMDA_Input(const char *pDataFileName, const char *pDataFileName1,
                const char *pMapFileName, const char *pMapFileName1) {
  uint64_t lineCount = 0;

  int32_t ret = FLE_Lines(pDataFileName, &lineCount);
  assert(!ret);

  char *pData = NULL;

  if (lineCount) {
    pData = calloc(lineCount, 1);
    assert(pData);

    assert(FLE_LoadTxtBuffer(pDataFileName, pData, lineCount) == lineCount);

    (*pInputData)(0, 1, (uint32_t *)pData, 0); // FIXME

    free(pData);
    pData = NULL;
  }

  ret = FLE_Lines(pDataFileName1, &lineCount);
  assert(!ret);

  if (lineCount) {
    pData = calloc(lineCount, 1);
    assert(pData);

    assert(FLE_LoadTxtBuffer(pDataFileName1, pData, lineCount) == lineCount);

    (*pInputData)(1, 1, (uint32_t *)pData, 0); // FIXME

    free(pData);
    pData = NULL;
  }

  ret = FLE_Lines(pMapFileName, &lineCount);
  assert(!ret);

  if (lineCount) {
    pData = calloc(lineCount, 1);
    assert(pData);

    assert(FLE_LoadTxtBuffer(pMapFileName, pData, lineCount) == lineCount);

    (*pInputMap)(1, 1, (uint32_t *)pData, 0); // FIXME

    free(pData);
    pData = NULL;
  }

  ret = FLE_Lines(pMapFileName1, &lineCount);
  assert(!ret);

  if (lineCount) {
    pData = calloc(lineCount, 1);
    assert(pData);

    assert(FLE_LoadTxtBuffer(pMapFileName1, pData, lineCount) == lineCount);

    (*pInputMap)(1, 1, (uint32_t *)pData, 0); // FIXME

    free(pData);
    pData = NULL;
  }
}

void CMDA_Output() { (*pPipeLine)(); }

int32_t CMDA_Refresh(int32_t ispIndex, char *pCmdFile, cJSON *pReport) {
  assert(pCmdFile);

  TRACE(BM_TEST_INF, "Load register file, %s\n", pCmdFile);

  cJSON *pItem = cJSON_CreateObject();
  cJSON_AddItemToObject(pReport, basename(pCmdFile), pItem);

  FILE *pF = FLE_Open(pCmdFile, "r");
  if (!pF) {
    cJSON_AddBoolToObject(pItem, JA_IS_EXIST, false);
    return -ENFILE;
  }

  uint32_t ops = 0, address = 0, value = 0;

  while (fscanf(pF, "%u %x %x", &ops, &address, &value) > 0) {
    if (ops == REG_WRITE) {
      (*pRegisterWrite)(address, value);
    } else {
      (*pRegisterRead)(address, &value);
    }
  }

  FLE_Close(pF);

  return 0;
}
