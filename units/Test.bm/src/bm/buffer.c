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

#include "bm/buffer.h"
#include "bash-color.h"
#include "bm/base.h"
#include "file/file.h"
#include "json/json-app.h"
#include <assert.h>
#include <json/cJSON.h>

int32_t BMBUF_Load(HalHandle_t hHal, char *pFileName,
                   MediaBuffer_t *pMediaBuffer, cJSON *pReport) {
  assert(hHal);
  assert(pFileName);
  assert(pMediaBuffer);
  assert(pReport);

  FILE *pF = FLE_Open(pFileName, "r");
  if (!pF) {
    return -ENFILE;
  }

#if defined JUST_VERIFY_CASE_FILES
  FLE_Close(pF);
  return 0;
#endif

  uint64_t length = 0;

  int32_t ret = FLE_GetBufferLength(pFileName, &length);
  if (ret < 0) {
    FLE_Close(pF);
    return ret;
  }

  uint8_t stride = 0;
  ret = FLE_GetBufferStride(pFileName, &stride);
  if (ret < 0) {
    FLE_Close(pF);
    return ret;
  }

  TRACE(BM_TEST_INF, "Load file, %s, length = 0x%lX, stride = %d\n", pFileName,
        length, stride);

  uint8_t *pMapBuffer = NULL;

  ret = HalMapMemory(hHal, pMediaBuffer->buf.base_address, length,
                     HAL_MAPMEM_WRITEONLY, (void *)&pMapBuffer);
  assert(ret == RET_SUCCESS);

  uint64_t addressOffset = 0;
  uint64_t addressStart = 0;
  uint64_t address = 0;
  uint32_t data = 0;
  uint64_t lineNumber = 0;

  for (; fscanf(pF, "%lx %x", &address, &data) > 0; lineNumber++) {
    if (lineNumber == 0) {
      addressStart = address;
    }

    addressOffset = address - addressStart;

    for (uint8_t j = 0; j < stride; j++) {
      pMapBuffer[addressOffset + j] = data;

      if (stride > 1) {
        data = data >> 8;
      }
    }
  }

  FLE_Close(pF);

  if (lineNumber != length) {
    cJSON_AddTrueToObject(pReport, JA_IS_CONTAIN_DUMMY);
  }

  ret = HalUnMapMemory(hHal, pMapBuffer);
  assert(ret == RET_SUCCESS);

  char desc[100] = {0};
  snprintf(desc, sizeof(desc), "0x%lX", length);

  cJSON_AddStringToObject(pReport, JA_BUFFER_LENGTH, desc);

  return ret;
}

int32_t BMBUF_Save(HalHandle_t hHal, char *pGoldenFileName,
                   MediaBuffer_t *pMediaBuffer,
                   BMG_AnalyzeResult_t *pAnalyzeResult,
                   uint64_t *pGoldenBaseAddress, bool isPingPangBuffer,
                   cJSON *pReport) {
  assert(hHal);
  assert(pGoldenFileName);
  assert(pMediaBuffer);
  assert(pAnalyzeResult);
  assert(pGoldenBaseAddress);
  assert(pReport);

  if (*pGoldenBaseAddress == GOLDEN_BASE_ADDRESS_INIT) {
    *pGoldenBaseAddress = pAnalyzeResult->startAddr;

    char desc[20] = {0};
    snprintf(desc, sizeof(desc), "0x%08lX", *pGoldenBaseAddress);
    cJSON_AddStringToObject(pReport, JA_GOLDEN_BASE_ADDRESS, desc);

    snprintf(desc, sizeof(desc), "0x%08lX",
             (uint64_t)pMediaBuffer->buf.base_address);
    cJSON_AddStringToObject(pReport, JA_MEDIA_BUFFER_ADDRESS, desc);
  }

  FILE *pGolden = FLE_Open(pGoldenFileName, "r");
  if (!pGolden) {
    return -ENFILE;
  }

  BMG_SeekBufferStart(pGolden, pAnalyzeResult);

  char out[MAX_LEN] = {0};

  snprintf(out, MAX_LEN, "%s.out", pGoldenFileName);

  FILE *pOut = FLE_Open(out, "w");
  assert(pOut);

  uint8_t *pMapBuffer = NULL;

  int32_t ret =
      HalMapMemory(hHal, pMediaBuffer->buf.base_address, pMediaBuffer->buf.size,
                   HAL_MAPMEM_READONLY, (void *)&pMapBuffer);
  assert(ret == RET_SUCCESS);

  uint64_t address = 0;
  uint32_t value = 0;
  uint64_t offset = 0;

  for (uint32_t i = 0; i < pAnalyzeResult->bufferBytes; i++) {
    if (fscanf(pGolden, "%lx %x", &address, &value) <= 0) {
        break;
    }

    offset = address - *pGoldenBaseAddress;

    if (offset >= pMediaBuffer->buf.size) {
      TRACE(BM_TEST_ERR,
            COLOR_STRING(
                COLOR_YELLOW,
                "buffer error: offset(address 0x%09lx - *pGoldenBaseAddress "
                "0x%09lx) 0x%09lX > size 0x%09lX \n"),
            address, *pGoldenBaseAddress, offset, pMediaBuffer->buf.size);

      cJSON_AddTrueToObject(pReport, JA_IS_DIFFERENT_BUFFER);

      break;
    }

#if defined DUMP_DATA_WIDTH_8
    fprintf(pOut, "%09lx %08x\n", offset, *(pMapBuffer + offset));
#else
    fprintf(pOut, "%09lx %02x\n", offset, *(pMapBuffer + offset));
#endif
  }

  ret = HalUnMapMemory(hHal, pMapBuffer);
  assert(ret == RET_SUCCESS);

  FLE_Close(pOut);
  FLE_Close(pGolden);

  TRACE(BM_TEST_INF, "Save file, %s, length = 0x%lx\n", out,
        offset ? pAnalyzeResult->bufferBytes : 0);

  return 0;
}

int32_t BMBUF_Verify(char *pGoldenFileName, BMG_AnalyzeResult_t *pAnalyzeResult,
                     cJSON *pReport) {
  assert(pGoldenFileName);
  assert(pAnalyzeResult);
  assert(pReport);

  FILE *pGolden = FLE_Open(pGoldenFileName, "r");
  assert(pGolden);

  char out[MAX_LEN] = {0};
  snprintf(out, MAX_LEN, "%s.out", pGoldenFileName);

  FILE *pOut = FLE_Open(out, "r");
  assert(pOut);

  if (pAnalyzeResult->isRollback) {
    uint64_t addr = 0;
    uint32_t value = 0;

    uint64_t previousAddr = 0;
    uint32_t rollbackCount = 0;

    for (uint32_t i = 0; i < pAnalyzeResult->bytes; i++) {
      if (fscanf(pGolden, "%lx %x", &addr, &value) <= 0) {
          break;
      }
      if (addr < previousAddr) {
        rollbackCount++;
      }

      if (rollbackCount == pAnalyzeResult->rollbackCount - 1) {
        if (addr >= pAnalyzeResult->endAddr) {
          char desc = 0;

          do {
            desc = fgetc(pGolden);
          } while (desc != '\n');

          break;
        }
      }

      previousAddr = addr;
    }
  }

  uint32_t percent = 0;

  int32_t ret = FLE_CompareBuffer(pGolden, pOut, &percent);
  if (!ret) {
    TRACE(BM_TEST_INF, "Compare %s " COLOR_STRING(COLOR_GREEN, "OK \n"),
          pGoldenFileName);
  } else if (ret == -ENOENT) {
    TRACE(BM_TEST_ERR,
          "Compare %s " COLOR_STRING(COLOR_YELLOW, "file is empty \n"),
          pGoldenFileName);

    percent = 100;
  } else {
    TRACE(BM_TEST_ERR,
          "Compare %s " COLOR_STRING(COLOR_RED, "match rate = %d%% \n"),
          pGoldenFileName, percent);
  }

  char desc[20] = "";

  if (percent == 100) {
    snprintf(desc, sizeof(desc), "OK");
  } else {
    snprintf(desc, sizeof(desc), "%d%%", percent);
  }

  cJSON_AddStringToObject(pReport, JA_MATCH_RATE, desc);

  if (pGolden) {
    FLE_Close(pGolden);
    pGolden = NULL;
  }

  if (pOut) {
    FLE_Close(pOut);
    pOut = NULL;
  }

  return 0;
}
