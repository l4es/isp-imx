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

#ifndef _BARE_METAL_h
#define _BARE_METAL_h

#ifdef __cplusplus
extern "C" {
#endif

#include "bm/base.h"
#include "json/cJSON.h"

void BM_ParseConfig(TestCtx_t *pCtx, cJSON *pJObject);
void BM_TestCase(TestCtx_t *pCtx, cJSON *pReport);

#ifdef __cplusplus
}
#endif

#endif
