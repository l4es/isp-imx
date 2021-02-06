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
#include "commitf_citf.hpp"

CREATE_TRACER(CITF_INF, "[CITF][INF]: ", INFO, 1);
CREATE_TRACER(CITF_ERR, "[CITF][ERR]: ", ERROR, 1);

using namespace camdev;

RESULT Citf::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    //int32_t id = jRequest[CITF_MID].asInt();
    int32_t id = (int32_t)ctrlId;
    if(checkIsSubId(id) != RET_SUCCESS){
        throw exc::LogicError(RET_NOTSUPP, "Can't handle command");
    }

    return RET_SUCCESS;
}

RESULT Citf::getModuleVersion(Json::Value &jRequest, Json::Value &jResponse)
{
    TRACE(CITF_INF, "CTIF %s! IN\n", __func__);
    TRACE(CITF_INF, "default version 0\n");
    return RET_SUCCESS;
}

Citf::Citf(Citf_Handle * pCitfHandle_in){
    TRACE(CITF_INF, "CTIF %s! IN\n", __func__);
    TRACE(CITF_INF, "CITF created, set default subID\n");
    subId_Begin = ISPCORE_MODULE_DEFAULT;
    subId_End = ISPCORE_MODULE_DEFAULT;
    pCitfHandle = pCitfHandle_in;
}

Citf::~Citf(){
    TRACE(CITF_INF, "CTIF %s! IN\n", __func__);
    pCitfHandle = nullptr;
}

RESULT Citf::setSubId(int32_t inputId_Begin, int32_t inputId_End)
{
    subId_Begin = inputId_Begin;
    subId_End = inputId_End;

    return RET_SUCCESS;
}

RESULT Citf::checkIsSubId(int32_t inputId){
    return ((subId_Begin <= inputId) && (inputId <= subId_End)) ? RET_SUCCESS : RET_FAILURE;
}

#if 1
camdev::SensorOps &Citf::sensor() {
    if (!pCitfHandle->pOperation->sensors.size()) {
        throw exc::LogicError(RET_WRONG_STATE, "Sensor is never initialized");
    }

    return pCitfHandle->pOperation->sensor();
}
#endif
