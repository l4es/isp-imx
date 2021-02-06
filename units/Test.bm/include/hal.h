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

#if !defined __HAL_H__
#define __HAL_H__

#include "bm/base.h"

uint32_t REG_Read(TestCtx_t *pCtx, uint32_t address);

void REG_Reset();

void REG_Write(TestCtx_t *pCtx, uint32_t address, uint32_t value);

#endif
