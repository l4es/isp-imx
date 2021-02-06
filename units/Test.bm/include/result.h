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

#ifndef _CASE_RESULT_H_
#define _CASE_RESULT_H_

#include "json/cJSON.h"

typedef enum {
  CASE_UNKNOWN,
  CASE_INVALID,
  CASE_PASS,
  CASE_FAILED,
} CaseResult_t;

const char *RES_GetResultDescription(CaseResult_t);
void RES_OutputResult(cJSON *);
void RES_OutputFailedList(cJSON *);

#endif
