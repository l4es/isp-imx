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

#include "assert.h"
#include "bash-color.h"
#include "bm/base.h"

static uint32_t OpsCount = 0;

void REG_Reset() { OpsCount = 0; }

uint32_t REG_Read(TestCtx_t *pCtx, uint32_t address) {
  assert(pCtx);

  uint32_t value = HalReadReg(pCtx->hHal, address);

  TRACE(BM_TEST_INF, "[%03d] 0x%08X >> 0x%08X \n", OpsCount++, address, value);

  return value;
}

void REG_Write(TestCtx_t *pCtx, uint32_t address, uint32_t value) {
  assert(pCtx);

  uint32_t value2 = value;

  HalWriteReg(pCtx->hHal, address, value);

  TRACE(BM_TEST_INF, "[%03d] 0x%08X << 0x%08X \n", OpsCount, address, value);

#if defined CHECK_WRITE_REG_RESULT
  value = HalReadReg(pCtx->hHal, address);

  if (value != value2) {
    static uint32_t previousAddress = 0;

    if (previousAddress != address) {
      previousAddress = address;

      TRACE(BM_TEST_INF, COLOR_STRING(COLOR_BLUE, "[%03d] 0x%08X >> 0x%08X \n"),
            OpsCount, address, value);
    }
  }
#endif

  OpsCount++;
}
