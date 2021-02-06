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

#ifndef _JSON_APP_H_
#define _JSON_APP_H_

#include "json/cJSON.h"

#define JA_BM_CONFIG "bmConfig"
#define JA_BUFFER_LENGTH "buffer.length"
#define JA_CASE_CONFIG "caseConfig"
#define JA_CORE_INDEX "core.index"
#define JA_DESCRIPTIONS "descriptions"
#define JA_FILE "file"
#define JA_FRAME "frame"
#define JA_FRAMES "frames"
#define JA_GOLDEN "golden"
#define JA_GOLDEN_BASE_ADDRESS "golden.base.address"
#define JA_INPUT "input"
#define JA_IS_CONTAIN_DUMMY "is.contain.dummy"
#define JA_IS_DIFFERENT_BUFFER "is.different.buffer"
#define JA_IS_DST_AUTO_SHADOW "is.dst.auto.shadow"
#define JA_IS_EMPTY_FILE "is.empty.file"
#define JA_IS_EXIST "is.exist"
#define JA_IS_JSON_VALID "is.json.valid"
#define JS_IS_NONLINEAR "is.nonlinear"
#define JA_IS_PING_PANG_BUFFER "is.ping.pang.buffer"
#define JA_IS_ROLLBACK "is.rollback"
#define JA_IS_VALID "is.valid"
#define JA_IS_VERIFY "is.verify"
#define JA_MATCH_RATE "match.rate"
#define JA_MEDIA_BUFFER_ADDRESS "media.buffer.address"
#define JA_MIN_ADDR "min.address"
#define JA_NAME "name"
#define JA_OUTPUT "output"
#define JA_PATH "path"
#define JA_REGISTER "register"
#define JA_REGISTER_COUNT "register.count"
#define JA_VERIFY "verify"

cJSON *JA_ReadFromFile(const char *);

#endif
