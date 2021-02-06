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

#include "commitf_event.hpp"
#include "commitf_citf.hpp"

using namespace camdev;


#ifdef TRACE_CMD
#undef TRACE_CMD
#endif

#define TRACE_CMD\
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__)

USE_TRACER(CITF_INF);
USE_TRACER(CITF_ERR);

CitfEvent::CitfEvent(){
    TRACE(CITF_INF, "CommItf Event instance created %s!\n", __func__);

    TRACE(CITF_INF, "CommItf create done %s!\n", __func__);
}

CitfEvent::~CitfEvent(){
    TRACE(CITF_INF, "CommItf release done %s!\n", __func__);

    TRACE(CITF_INF, "CommItf release done %s!\n", __func__);
}
RESULT CitfEvent::citfUsrEventCbRegister(CITFEventCallback eventCb, void* pContext){
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);

    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
     return RET_SUCCESS;

}

RESULT CitfEvent::citfUsrEventCbDeregister(void){
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);

    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
     return RET_SUCCESS;

}
