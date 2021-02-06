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

#include "cam_common.hpp"
#include <ebase/dct_assert.h>

Ctrl::Ctrl() {
    TRACE_IN;

    int32_t ret = 0;

    ret = osEventInit(&eventStarted, 1, 0);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osEventInit(&eventStopped, 1, 0);
    DCT_ASSERT(ret == OSLAYER_OK);

    TRACE_OUT;
}

Ctrl::~Ctrl() {
    TRACE_IN;

    int32_t ret = 0;

    ret = osEventDestroy(&eventStarted);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osEventDestroy(&eventStopped);
    DCT_ASSERT(ret == OSLAYER_OK);

    TRACE_OUT;
}

HalHolder::HalHolder(int ispID) {
    hHal = HalOpen(ispID);
    DCT_ASSERT(hHal);

    // reference marvin software HAL
    HalAddRef(hHal);
    //DCT_ASSERT(ret == RET_SUCCESS);
}

HalHolder::~HalHolder() {
    return;

    DCT_ASSERT(hHal);

    // dereference marvin software HAL
    int32_t ret = HalDelRef(hHal);
    DCT_ASSERT(ret == RET_SUCCESS);

    ret = HalClose(hHal);
    DCT_ASSERT(ret == RET_SUCCESS);

    hHal = NULL;
}
