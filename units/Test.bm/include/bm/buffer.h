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

#if !defined __BARE_METAL_BUFFER_H__
#define __BARE_METAL_BUFFER_H__

#include "bm/golden.h"
#include "json/cJSON.h"
#include <bufferpool/media_buffer_pool.h>
#include <hal/hal_api.h>

int32_t BMBUF_Load(HalHandle_t hHal, char *pGoldenFileName,
                   MediaBuffer_t *pMediaBuffer, cJSON *pReport);

int32_t BMBUF_Save(HalHandle_t hHal, char *pGoldenFileName,
                   MediaBuffer_t *pMediaBuffer,
                   BMG_AnalyzeResult_t *pAnalyzeResult,
                   uint64_t *pGoldenBaseAddress, bool isPingPangBuffer,
                   cJSON *pReport);

int32_t BMBUF_Verify(char *pGoldenFileName, BMG_AnalyzeResult_t *pAnalyzeResult,
                     cJSON *pReport);

#endif
