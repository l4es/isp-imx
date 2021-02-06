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

#if !defined __DOLPHIN_IP_H__
#define __DOLPHIN_IP_H__

#define REG_BASE(_X_) (_X_ << 20)

#define REG_BASE_TPG(_X_) ((_X_ << 4) | 0x00308300)
#define REG_BASE_TPG_MASK_ISP_RST_N BIT_MASK(24, 24)

#define PATH_INPUT_MAX 1
#define PATH_OUTPUT_MAX 1
#define CORE_MAX 1

/*
Input Buffer Map
      0     1
0     -     -
1     -     -
*/
#define CHANNEL_INPUT_MAX 1

/*
Output Buffer Map
      0     1     2
0     y     cb    cr
1     -     -     -
*/
typedef enum {
  OUTPUT_BUFFER_Y,
  OUTPUT_BUFFER_CB,
  OUTPUT_BUFFER_CR,
  OUTPUT_BUFFER_SUM,
} OutputBufferIndex_t;

#define CHANNEL_OUTPUT_MAX OUTPUT_BUFFER_SUM

#endif
