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

#if !defined __BARE_METAL_BASE_H__
#define __BARE_METAL_BASE_H__

#include IP_DEFINE_HEADER

#include "bm/golden.h"
#include "json/cJSON.h"
#include <bufferpool/media_buffer_pool.h>
#include <ebase/trace.h>
#include <hal/hal_api.h>

#define BIT_MASK(H, L) (((1 << (H - L + 1)) - 1) << L)

#define GOLDEN_BASE_ADDRESS_INIT 0xFFFFFFFFFFFFFFFF

USE_TRACER(BM_TEST_INF);
USE_TRACER(BM_TEST_WAN);
USE_TRACER(BM_TEST_ERR);

#define MAX_LEN 256

#define CHANNEL_COUNT                                                          \
  (PATH_INPUT_MAX * CHANNEL_INPUT_MAX + PATH_OUTPUT_MAX * CHANNEL_OUTPUT_MAX)

typedef struct BMTestConfig_s {
  char fileNameInput[CORE_MAX][PATH_INPUT_MAX][CHANNEL_INPUT_MAX][MAX_LEN];
  char fileNameOutput[CORE_MAX][PATH_OUTPUT_MAX][CHANNEL_OUTPUT_MAX][MAX_LEN];
  char fileNameRegister[CORE_MAX][MAX_LEN];
} BMTestConfig_t;

enum { REG_WRITE, REG_READ };

#define FRAME_MAX 4

typedef struct TestCase_s {
  char pathInput[MAX_LEN];
  char pathOutput[MAX_LEN];
  char name[MAX_LEN];
  char description[MAX_LEN];
  char version[MAX_LEN];
  char date[MAX_LEN];
  union TestConfig_u {
    BMTestConfig_t bm;
  } config[FRAME_MAX];
  char configCount;
} TestCase_t;

typedef struct BMMemPool_s {
  MediaBufPool_t bufPool;
  MediaBufPoolConfig_t bufPoolConfig;
  MediaBufPoolMemory_t bufPoolMemory;
} BMMemPool_t;

typedef struct BMTest_s {
  BMMemPool_t memPool;
  MediaBuffer_t *pInputBuffer[PATH_INPUT_MAX][CHANNEL_INPUT_MAX];
  MediaBuffer_t *pOutputBuffer[PATH_OUTPUT_MAX][CHANNEL_OUTPUT_MAX];
  uint64_t goldenBaseAddress[PATH_OUTPUT_MAX][CHANNEL_OUTPUT_MAX];
  BMG_AnalyzeResult_t goldenAnalyzeResult[PATH_OUTPUT_MAX][CHANNEL_OUTPUT_MAX];
  int32_t coreIndex;
  int32_t frameNumber;
  int32_t frameNumberDstAutoShadow;
  bool isPingPangBuffer;
  bool isDstAutoShadow;
} BMTest_t;

typedef struct TestCtx_s {
  HalHandle_t hHal;
  char pathInputRoot[MAX_LEN];
  TestCase_t testCase;
  BMTest_t *pBmTest;
} TestCtx_t;

#if defined(HAL_ALTERA)
int32_t FpgaResetControl(HalHandle_t HalHandle, int32_t is_reset);
#endif

#endif
