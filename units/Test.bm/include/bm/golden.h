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

#ifndef _BM_GOLDEN_h
#define _BM_GOLDEN_h

#ifdef __cplusplus
extern "C" {
#endif

#include <ebase/types.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  uint64_t startAddr;
  uint64_t endAddr;
  uint64_t minAddr;
  uint64_t maxAddr;
  uint64_t bufferBytes;
  uint64_t bufferLength;
  uint64_t bytes;
  bool isNonlinear;
  bool isRollback;
  uint32_t rollbackCount;
} BMG_AnalyzeResult_t;

int BMG_AnalyzeBuffer(const char *, BMG_AnalyzeResult_t *);
int BMG_SeekBufferStart(FILE *, BMG_AnalyzeResult_t *);

#ifdef __cplusplus
}
#endif

#endif
