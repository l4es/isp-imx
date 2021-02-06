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

#ifndef __BARE_METAL_IP_H__
#define __BARE_METAL_IP_H__

#define REG_BASE_DWE_CTRL(_X_) 0x00308250
#define REG_BASE_DWE_CTRL_MASK_DEWARP_RESET BIT_MASK(0, 0) // 0 -> 1
#define REG_BASE_DWE_CTRL_MASK_DEWARP_START BIT_MASK(4, 4) // 1 -> 0

#define REG_BASE(_X_) 0x00380000

#define PATH_INPUT_MAX 1
#define PATH_OUTPUT_MAX 1
#define CORE_MAX 1

#define CHANNEL_INPUT_MAX 4
#define CHANNEL_OUTPUT_MAX 4

#endif
