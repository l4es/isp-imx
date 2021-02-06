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

#include "cam_device_api.hpp"
#include "ispcore_holder.hpp"
#include "macros.hpp"

using namespace camdev;

Ispcore_Holder::Ispcore_Holder() {
    TRACE_IN;
    TRACE(CITF_INF, "%s: ISPcore camdevice startup.. Initialize \n", __PRETTY_FUNCTION__);
    int i = 0;
    for (i = 0; i < CAM_ISPCORE_ID_MAX; i ++) {
        ispcore[i].ctx = nullptr;
        ispcore[i].handle = nullptr;
    }
}

Ispcore_Holder::~Ispcore_Holder() {
    TRACE_IN;
}

Ispcore_Holder::Inst_Creator Ispcore_Holder::inst_creator;
