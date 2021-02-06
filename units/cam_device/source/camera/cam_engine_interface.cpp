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

#include "cam_engine_interface.hpp"
#include "ispcore_holder.hpp"
#include "exception.hpp"
#include "macros.hpp"

#include "calib_calibration.hpp"

using namespace camdev;

#define CAM_API_CAMENGINE_TIMEOUT 30000U // 30 s

Engine::Engine(Operation_Handle *pOperationHandle_in, bool hdr) {
    pOperationHandle = pOperationHandle_in;

    REFSET(camEngineConfig, 0);

    REFSET(regDescriptionConfig, 0);
    DCT_ASSERT(pOperationHandle != nullptr);
    DCT_ASSERT(pOperationHandle->pHalHolder != nullptr);
    DCT_ASSERT(pOperationHandle->pCalibration != nullptr);

    regDescriptionConfig.HalHandle = pOperationHandle->pHalHolder->hHal;

    int32_t ret = CamerIcInitRegDescriptionDrv(&regDescriptionConfig, &hCamerIcRegCtx);
    DCT_ASSERT(ret == RET_SUCCESS);

    REFSET(instanceConfig, 0);

    instanceConfig.maxPendingCommands = 4;
    instanceConfig.cbCompletion = cbCompletion;
    instanceConfig.cbAfpsResChange = afpsResChangeCb;
    instanceConfig.pUserCbCtx = (void *)this;
    instanceConfig.hHal = pOperationHandle->pHalHolder->hHal;
    instanceConfig.isSystem3D = BOOL_FALSE; // TODO:
#ifdef SUBDEV_V4L2
    instanceConfig.hdr = hdr;
#endif
    ret = CamEngineInit(&instanceConfig);
    ret = RET_SUCCESS;
    //DCT_ASSERT(ret == RET_SUCCESS);

    hCamEngine = instanceConfig.hCamEngine;

    ret = osEventInit(&eventStart, 1, 0);
    ret |= osEventInit(&eventStop, 1, 0);
    ret |= osEventInit(&eventStartStreaming, 1, 0);
    ret |= osEventInit(&eventStopStreaming, 1, 0);
    ret |= osEventInit(&eventAcquireLock, 1, 0);
    ret |= osEventInit(&eventReleaseLock, 1, 0);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osQueueInit(&queueAfpsResChange, 1, sizeof(uint32_t));
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osThreadCreate(&threadAfpsResChange, entryAfpsResChange, this);
    DCT_ASSERT(ret == OSLAYER_OK);
}

Engine::~Engine() {
    int32_t ret = 0;

    // clean request
    do {
        uint32_t dummy = 0;

        ret = osQueueTryRead(&queueAfpsResChange, &dummy);
    } while (ret == OSLAYER_OK);

    uint32_t newRes = 0;
    ret = osQueueWrite(&queueAfpsResChange, &newRes); // send stop request
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osThreadClose(&threadAfpsResChange);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osQueueDestroy(&queueAfpsResChange);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = osEventDestroy(&eventStart);
    ret |= osEventDestroy(&eventStop);
    ret |= osEventDestroy(&eventStartStreaming);
    ret |= osEventDestroy(&eventStopStreaming);
    ret |= osEventDestroy(&eventAcquireLock);
    ret |= osEventDestroy(&eventReleaseLock);
    DCT_ASSERT(ret == OSLAYER_OK);

    ret = CamEngineShutDown(hCamEngine);
    DCT_ASSERT(ret == RET_SUCCESS);
}

int32_t Engine::aeConfigGet(camdev::CalibAe::Config &config) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAecSemMode_t mode = CAM_ENGINE_AEC_SCENE_EVALUATION_INVALID;
    float setPoint = 0;
    float clmTolerance = 0;
    float dampOver = 0;
    float dampUnder = 0;

    int32_t ret = CamEngineAecStatus(hCamEngine, &isRunning, &mode, &setPoint, &clmTolerance, &dampOver, &dampUnder);
    REPORT(ret);

    camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

    ae.isEnable = isRunning == BOOL_TRUE;

    ae.config.mode = mode;
    ae.config.dampingOver = dampOver;
    ae.config.dampingUnder = dampUnder;
    ae.config.setPoint = setPoint;
    ae.config.tolerance = clmTolerance;

    config = ae.config;

    return RET_SUCCESS;
}

int32_t Engine::aeConfigSet(camdev::CalibAe::Config config) {
    CamEngineAecSemMode_t mode = config.mode;
    float setPoint = config.setPoint;
    float clmTolerance = config.tolerance;
    float dampOver = config.dampingOver;
    float dampUnder = config.dampingUnder;

    int32_t ret = CamEngineAecConfigure(hCamEngine, mode, setPoint, clmTolerance, dampOver, dampUnder);
    REPORT(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

        ae.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::getMetadata(CamEnginePathType_t path, struct isp_metadata *meta) {
    return CamEngineGetFullMetadata(hCamEngine, (int)path, meta);
}

int32_t Engine::aeEcmGet(camdev::CalibAe::Ecm &ecm) {
    camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

    ecm = ae.ecm;

    return RET_SUCCESS;
}

int32_t Engine::aeEcmSet(camdev::CalibAe::Ecm ecm) {
    int32_t ret = CamEngineSetEcm(hCamEngine, ecm.flickerPeriod, ecm.isAfps ? BOOL_TRUE : BOOL_FALSE);
    REPORT(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

        ae.ecm = ecm;
    }

    return RET_SUCCESS;
}

int32_t Engine::aeEnableGet(bool &isEnable) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAecSemMode_t mode;
    float setPoint = 0;
    float clmTolerance = 0;
    float dampOver = 0;
    float dampUnder = 0;

    int32_t ret = CamEngineAecStatus(hCamEngine, &isRunning, &mode, &setPoint, &clmTolerance, &dampOver, &dampUnder);
    REPORT(ret);

    isEnable = isRunning == BOOL_TRUE;

    pOperationHandle->pCalibration->module<camdev::CalibAe>().isEnable = isEnable;

    return RET_SUCCESS;
}

int32_t Engine::aeEnableSet(bool isEnable) {
    int32_t ret = RET_SUCCESS;

    if (isEnable) {
        ret = CamEngineAecStart(hCamEngine);
        REPORT(ret);
    } else {
        ret = CamEngineAecStop(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        pOperationHandle->pCalibration->module<camdev::CalibAe>().isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::aeFlickerPeriodGet(float &flickerPeriod) const {
    switch (pOperationHandle->pCalibration->module<camdev::CalibAe>().ecm.flickerPeriod) {
    case CAM_ENGINE_FLICKER_OFF:
        flickerPeriod = (0.0000001);
        break; // this small value virtually turns of flicker avoidance

    case CAM_ENGINE_FLICKER_100HZ:
        flickerPeriod = (1.0 / 100.0);
        break;

    case CAM_ENGINE_FLICKER_120HZ:
        flickerPeriod = (1.0 / 120.0);
        break;

    default:
        flickerPeriod = (0.0000001);
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

int32_t Engine::aeStatus(camdev::CalibAe::Status &status) {
    int32_t ret = CamEngineAecGetHistogram(hCamEngine, &status.histogram);
    ret |= CamEngineAecGetLuminance(hCamEngine, &status.luminance);
    ret |= CamEngineAecGetObjectRegion(hCamEngine, &status.objectRegion);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t Engine::aeReset() { return CamEngineAecReset(hCamEngine); }

int32_t Engine::afAvailableGet(bool &isAvailable) {
    bool_t isAvailable2 = BOOL_FALSE;

    int32_t ret = CamEngineAfAvailable(hCamEngine, &isAvailable2);
    REPORT(ret);

    isAvailable = isAvailable2 == BOOL_TRUE;

    return RET_SUCCESS;
}

int32_t Engine::afConfigGet(camdev::CalibAf::Config &config) {
    bool_t isRunning = BOOL_FALSE;

    int32_t ret =
            CamEngineAfStatus(hCamEngine, &isRunning, &config.searchAlgorithm);
    REPORT(ret);

    camdev::CalibAf &af = pOperationHandle->pCalibration->module<camdev::CalibAf>();

    af.config = config;
    af.isEnable = isRunning == BOOL_TRUE;

    return RET_SUCCESS;
}

int32_t Engine::afConfigSet(camdev::CalibAf::Config config) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAf &af = pOperationHandle->pCalibration->module<camdev::CalibAf>();

        af.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::afEnableGet(bool &isEnable) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAfSearchAlgorithm_t searchAlgorithm;

    int32_t ret = CamEngineAfStatus(hCamEngine, &isRunning, &searchAlgorithm);
    REPORT(ret);

    isEnable = isRunning == BOOL_TRUE;

    camdev::CalibAf &af = pOperationHandle->pCalibration->module<camdev::CalibAf>();

    af.config.searchAlgorithm = searchAlgorithm;
    af.isEnable = isEnable;

    return RET_SUCCESS;
}

int32_t Engine::afEnableSet(bool isEnable) {
    camdev::CalibAf &af = pOperationHandle->pCalibration->module<camdev::CalibAf>();

    if (isEnable) {
        if (af.config.isOneshot) {
            int32_t ret = CamEngineAfOneShot(hCamEngine, af.config.searchAlgorithm);
            REPORT(ret);
        } else {
            int32_t ret = CamEngineAfStart(hCamEngine, af.config.searchAlgorithm);
            REPORT(ret);
        }
    } else {
        int32_t ret = CamEngineAfStop(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        af.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

void Engine::afpsResChangeCb(uint32_t newRes, const void *pUserContext) {
    DCT_ASSERT(pUserContext);

    Engine *pEngine = (Engine *)pUserContext;

    int32_t ret = OSLAYER_OK;

    // do {
    //     uint32_t dummy;

    //     ret = osQueueTryRead(&pEngine->queueAfpsResChange, &dummy);
    // } while (ret == OSLAYER_OK);

    ret = osQueueWrite(&pEngine->queueAfpsResChange, &newRes);
    DCT_ASSERT(ret == OSLAYER_OK);
}

int32_t Engine::entryAfpsResChange(void *pParam) {
    DCT_ASSERT(pParam);

    Engine *pEngine = (Engine *)pParam;

    uint32_t newRes = 0;

    while (osQueueRead(&pEngine->queueAfpsResChange, &newRes) == OSLAYER_OK) {
        if (newRes == 0) {
            TRACE(CITF_INF, "AfpsResChangeThread (stopping)\n");
            break;
        }

    }

    TRACE(CITF_INF, "AfpsResChangeThread (stopped)\n");

    return RET_SUCCESS;
}

int32_t Engine::avsConfigGet(camdev::CalibAvs::Config &config) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/AVS not ready"); // TODO

    camdev::CalibAvs &avs = pOperationHandle->pCalibration->module<camdev::CalibAvs>();

    config = avs.config;

    return RET_SUCCESS;
}

int32_t Engine::avsConfigSet(camdev::CalibAvs::Config config) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/AVS not ready"); // TODO

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAvs &avs = pOperationHandle->pCalibration->module<camdev::CalibAvs>();

        avs.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::avsEnableGet(bool &isEnable) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/AVS not ready"); // TODO

    camdev::CalibAvs &avs = pOperationHandle->pCalibration->module<camdev::CalibAvs>();

    isEnable = avs.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::avsEnableSet(bool isEnable) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/AVS not ready"); // TODO

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAvs &avs = pOperationHandle->pCalibration->module<camdev::CalibAvs>();

        avs.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::avsStatusGet(camdev::CalibAvs::Status &status) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/AVS not ready"); // TODO

    return RET_SUCCESS;
}

int32_t Engine::awbConfigGet(camdev::CalibAwb::Config &config) {
    bool_t isRunning = BOOL_FALSE;

    CamEngineAwbMode_t mode = CAM_ENGINE_AWB_MODE_INVALID;
    uint32_t index = 0;
    bool_t isDamping = BOOL_FALSE;
    CamEngineAwbRgProj_t rgProj;

    int32_t ret = CamEngineAwbStatus(hCamEngine, &isRunning, &mode, &index, &rgProj, &isDamping);
    REPORT(ret);

    camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

    awb.config.mode = mode;
    awb.config.index = index;
    awb.config.isDamping = isDamping == BOOL_TRUE;

    config = awb.config;

    return RET_SUCCESS;
}

int32_t Engine::awbConfigSet(camdev::CalibAwb::Config config) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

        awb.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::awbEnableGet(bool &isEnable) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAwbMode_t mode;
    uint32_t index = 0;
    bool_t isDamping = BOOL_FALSE;
    CamEngineAwbRgProj_t rgProj;

    int32_t ret = CamEngineAwbStatus(hCamEngine, &isRunning, &mode, &index, &rgProj, &isDamping);
    REPORT(ret);

    camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

    awb.isEnable = isRunning == BOOL_TRUE;

    awb.config.mode = mode;
    awb.config.index = index;
    awb.config.isDamping = isDamping == BOOL_TRUE;

    isEnable = awb.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::awbEnableSet(bool isEnable) {
    camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

    if (isEnable) {
        CamEngineAwbMode_t mode = awb.config.mode;
        uint32_t index = awb.config.index;
        bool_t isDamping = awb.config.isDamping ? BOOL_TRUE : BOOL_FALSE;

        int32_t ret = CamEngineAwbStart(hCamEngine, mode, index, isDamping);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineAwbStop(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        awb.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::awbGainSet(float* gain) {

    CamEngineWbSetGains(hCamEngine, (CamEngineWbGains_t*)gain);
    return RET_SUCCESS;
}
int32_t Engine::awbReset() { return CamEngineAwbReset(hCamEngine); }

int32_t Engine::awbStatusGet(camdev::CalibAwb::Status &status) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAwbMode_t mode;
    uint32_t index = 0;
    bool_t isDamping = BOOL_FALSE;
    CamEngineAwbRgProj_t rgProj;

    int32_t ret = CamEngineAwbStatus(hCamEngine, &isRunning, &mode, &index, &rgProj, &isDamping);
    REPORT(ret);

    camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

    awb.isEnable = isRunning == BOOL_TRUE;

    awb.config.mode = mode;
    awb.config.index = index;
    awb.config.isDamping = isDamping == BOOL_TRUE;

    status.rgProj = rgProj;

    return RET_SUCCESS;
}

int32_t Engine::blsConfigGet(camdev::CalibBls::Config &config) {
    uint16_t red = 0;
    uint16_t greenR = 0;
    uint16_t greenB = 0;
    uint16_t blue = 0;

    int32_t ret = CamEngineBlsGet(hCamEngine, &red, &greenR, &greenB, &blue);
    REPORT(ret);

    camdev::CalibBls &bls = pOperationHandle->pCalibration->module<camdev::CalibBls>();

    bls.config.blue = blue;
    bls.config.greenR = greenR;
    bls.config.greenB = greenB;
    bls.config.red = red;

    config = bls.config;

    return RET_SUCCESS;
}

int32_t Engine::blsConfigSet(camdev::CalibBls::Config config) {
    uint16_t red = config.red;
    uint16_t greenR = config.greenR;
    uint16_t greenB = config.greenB;
    uint16_t blue = config.blue;

    int32_t ret = CamEngineBlsSet(hCamEngine, red, greenR, greenB, blue);
    REPORT_WORET(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibBls &bls = pOperationHandle->pCalibration->module<camdev::CalibBls>();

        bls.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::bufferCbRegister(CamEngineBufferCb_t fpCallback, void *pBufferCbCtx) {
    int32_t ret = CamEngineRegisterBufferCb(hCamEngine, fpCallback, pBufferCbCtx);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t Engine::bufferCbUnregister() {
    int32_t ret = CamEngineDeRegisterBufferCb(hCamEngine);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t Engine::bufferCbGet(CamEngineBufferCb_t *pCallback, void **ppBufferCbCtx) {
    return CamEngineGetBufferCb(hCamEngine, pCallback, ppBufferCbCtx);
}

int32_t Engine::cacConfigGet(camdev::CalibCac::Config &config) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/CAC not ready"); // TODO

    camdev::CalibCac &cac = pOperationHandle->pCalibration->module<camdev::CalibCac>();

    config = cac.config;

    return RET_SUCCESS;
}

int32_t Engine::cacConfigSet(camdev::CalibCac::Config config) {
    throw exc::LogicError(RET_NOTAVAILABLE, "Engint/CAC not ready"); // TODO

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibCac &cac = pOperationHandle->pCalibration->module<camdev::CalibCac>();

        cac.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::cacEnableGet(bool &isEnable) {

    camdev::CalibCac &cac = pOperationHandle->pCalibration->module<camdev::CalibCac>();

    cac.isEnable = isEnable;

    return RET_SUCCESS;
}

int32_t Engine::cacEnableSet(bool isEnable) {
    int32_t ret = RET_SUCCESS;
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibCac &cac = pOperationHandle->pCalibration->module<camdev::CalibCac>();
        cac.isEnable = isEnable;
        if (isEnable)
            CamEngineCacEnable(hCamEngine);
        else
            CamEngineCacDisable(hCamEngine);
    }

    return ret;
}

void Engine::cbCompletion(CamEngineCmdId_t cmdId, int32_t result, const void *pUserContext) {
    DCT_ASSERT(pUserContext);

    Engine *pEngine = (Engine *)pUserContext;

    int32_t ret = RET_SUCCESS;

    switch (cmdId) {
    case CAM_ENGINE_CMD_START:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_START, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventStart);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case CAM_ENGINE_CMD_STOP:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_STOP, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventStop);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case CAM_ENGINE_CMD_START_STREAMING:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_START_STREAMING, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventStartStreaming);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case CAM_ENGINE_CMD_STOP_STREAMING:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_STOP_STREAMING, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventStopStreaming);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case CAM_ENGINE_CMD_ACQUIRE_LOCK:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_ACQUIRE_LOCK, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventAcquireLock);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case CAM_ENGINE_CMD_RELEASE_LOCK:
        TRACE(CITF_INF, "CAM_ENGINE_CMD_RELEASE_LOCK, ret = 0x%X \n", ret);
        ret = osEventSignal(&pEngine->eventReleaseLock);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    default:
        break;
    }
}

int32_t Engine::cnrConfigGet(camdev::CalibCnr::Config &config) {
    camdev::CalibCnr &cnr = pOperationHandle->pCalibration->module<camdev::CalibCnr>();
    config = cnr.config;
    return RET_SUCCESS;
}

int32_t Engine::cnrConfigSet(camdev::CalibCnr::Config config) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibCnr &cnr = pOperationHandle->pCalibration->module<camdev::CalibCnr>();

        cnr.config = config;
        CamEngineCnrSetThresholds(hCamEngine, cnr.config.tc1, cnr.config.tc2);
    }

    return RET_SUCCESS;
}

int32_t Engine::cnrEnableGet(bool &isEnable) {

    camdev::CalibCnr &cnr = pOperationHandle->pCalibration->module<camdev::CalibCnr>();
    isEnable = cnr.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::cnrEnableSet(bool isEnable) {

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibCnr &cnr = pOperationHandle->pCalibration->module<camdev::CalibCnr>();
        cnr.isEnable = isEnable;
        CamEngineCnrSetThresholds(hCamEngine, cnr.config.tc1, cnr.config.tc2);
        CamEngineCnrEnable(hCamEngine);
    }

    return RET_SUCCESS;
}

int32_t Engine::cprocConfigGet(camdev::CalibCproc::Config &config) {
    camdev::CalibCproc &cproc = pOperationHandle->pCalibration->module<camdev::CalibCproc>();

    bool_t isRunning = cproc.isEnable ? BOOL_TRUE : BOOL_FALSE;
    CamEngineCprocConfig_t cprocConfig = cproc.config.config;

    int32_t ret = CamEngineCprocStatus(hCamEngine, &isRunning, &cprocConfig);
    REPORT(ret);

    cproc.isEnable = isRunning == BOOL_TRUE;
    cproc.config.config = cprocConfig;

    config = cproc.config;

    return RET_SUCCESS;
}

int32_t Engine::cprocConfigSet(camdev::CalibCproc::Config config) {
    camdev::CalibCproc &cproc = pOperationHandle->pCalibration->module<camdev::CalibCproc>();

    int32_t ret = CamEngineCprocSetBrightness(hCamEngine, config.config.brightness);
    REPORT(ret);

    ret = CamEngineCprocSetContrast(hCamEngine, config.config.contrast);
    REPORT(ret);

    ret = CamEngineCprocSetHue(hCamEngine, config.config.hue);
    REPORT(ret);

    ret = CamEngineCprocSetSaturation(hCamEngine, config.config.saturation);
    REPORT(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        cproc.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::cprocEnableGet(bool &isEnable) {
    camdev::CalibCproc &cproc = pOperationHandle->pCalibration->module<camdev::CalibCproc>();

    bool_t isRunning = cproc.isEnable ? BOOL_TRUE : BOOL_FALSE;
    CamEngineCprocConfig_t cprocConfig = cproc.config.config;

    int32_t ret = CamEngineCprocStatus(hCamEngine, &isRunning, &cprocConfig);
    REPORT(ret);

    cproc.isEnable = isRunning == BOOL_TRUE;
    cproc.config.config = cprocConfig;

    isEnable = cproc.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::cprocEnableSet(bool isEnable) {
    camdev::CalibCproc &cproc = pOperationHandle->pCalibration->module<camdev::CalibCproc>();

    if (isEnable) {
        int32_t ret = CamEngineEnableCproc(hCamEngine, &cproc.config.config);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineDisableCproc(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        cproc.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::demosaicConfigGet(camdev::CalibDemosaic::Config &config) {
    camdev::CalibDemosaic &demosaic = pOperationHandle->pCalibration->module<camdev::CalibDemosaic>();

    config = demosaic.config;

    return RET_SUCCESS;
}

int32_t Engine::demosaicConfigSet(camdev::CalibDemosaic::Config config) {
#ifndef ISP_DEMOSAIC2
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibDemosaic &demosaic = pOperationHandle->pCalibration->module<camdev::CalibDemosaic>();

        demosaic.config = config;
        CamEngineDemosaicSet(hCamEngine, config.mode == CalibDemosaic::Config::Normal ? BOOL_FALSE : BOOL_TRUE, config.threshold);
    }
#endif
    return RET_SUCCESS;
}

int32_t Engine::demosaicEnableGet(bool &isEnable) {

    camdev::CalibDemosaic &demosaic = pOperationHandle->pCalibration->module<camdev::CalibDemosaic>();

    isEnable = demosaic.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::demosaicEnableSet(bool isEnable) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibDemosaic &demosaic = pOperationHandle->pCalibration->module<camdev::CalibDemosaic>();

        demosaic.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::nr2dConfigGet(camdev::CalibNr2d::Config &config, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA2dnrMode_t mode = CAM_ENGINE_A2DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      float sigma = 0;
      uint8_t strength = 0;
      uint8_t pregamaStrength = 0;

      auto ret = CamEngineA2dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &sigma, &strength,
                                      &pregamaStrength);
      REPORT(ret);

      camdev::CalibNr2d::Config::Nr2Dv1 &v1Config = nr2d.holders[generation].config.v1;

      v1Config.isAuto = mode == CAM_ENGINE_A2DNR_MODE_AUTO;
      v1Config.sigma = sigma;
      v1Config.denoiseStrength = strength;
      v1Config.denoisePregamaStrength = pregamaStrength;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)nr2d;
#endif
    }

    config = nr2d.holders[generation].config;

    return RET_SUCCESS;
}

int32_t Engine::nr2dConfigSet(camdev::CalibNr2d::Config config,
                            camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      uint8_t denoisePregamaStrength = static_cast<uint8_t>(config.v1.denoisePregamaStrength);
      uint8_t denoiseStrength = static_cast<uint8_t>(config.v1.denoiseStrength);
      float sigma = static_cast<float>(config.v1.sigma);

      auto ret = CamEngineA2dnrConfigure(hCamEngine, sigma, denoiseStrength,
                                         denoisePregamaStrength);
      REPORT(ret);
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)nr2d;
#endif
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        nr2d.holders[generation].config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::nr2dEnableGet(bool &isEnable, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA2dnrMode_t mode = CAM_ENGINE_A2DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      float sigma = 0;
      uint8_t strength = 0;
      uint8_t pregamaStrength = 0;

      auto ret = CamEngineA2dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &sigma, &strength,
                                      &pregamaStrength);
      REPORT(ret);

      nr2d.holders[generation].isEnable = isRunning == BOOL_TRUE;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)nr2d;
#endif
    }

    isEnable = nr2d.holders[generation].isEnable;

    return RET_SUCCESS;
}

int32_t Engine::nr2dEnableSet(bool isEnable, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      if (isEnable) {
        CamEngineA2dnrMode_t mode = nr2d.holders[camdev::CalibNr2d::Nr2Dv1].config.v1.isAuto
                                        ? CAM_ENGINE_A2DNR_MODE_AUTO
                                        : CAM_ENGINE_A2DNR_MODE_MANUAL;
        auto ret = CamEngineA2dnrStart(hCamEngine, mode);
        REPORT(ret);
      } else {
        auto ret = CamEngineA2dnrStop(hCamEngine);
        REPORT(ret);
      }
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)nr2d;
#endif
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
      nr2d.holders[generation].isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::nr2dReset(camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      nr2d.holders[generation].config.v1.reset();

      auto ret = nr2dConfigSet(nr2d.holders[generation].config, generation);
      REPORT(ret);
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)nr2d;
#endif
    }

    camdev::CalibNr2d noUnusedValue = nr2d;

    return RET_SUCCESS;
}

int32_t Engine::nr2dStatusGet(camdev::CalibNr2d::Status &status, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA2dnrMode_t mode = CAM_ENGINE_A2DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      float sigma = 0;
      uint8_t strength = 0;
      uint8_t pregamaStrength = 0;

      auto ret = CamEngineA2dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &sigma, &strength,
                                      &pregamaStrength);
      REPORT(ret);

      status.gain = gain;
      status.intergrationTime = intergrationTime;
      (void)nr2d;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)generation;
      (void)status;
#endif
   }

   return RET_SUCCESS;
}

int32_t Engine::nr2dTableGet(Json::Value &jTable, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      jTable = nr2d.holders[generation].table.jTable;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)jTable;
      (void)nr2d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::nr2dTableSet(Json::Value jTable, camdev::CalibNr2d::Generation generation) {
    camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

    if (generation ==camdev::CalibNr2d::Nr2Dv1) {
#if defined CTRL_2DNR
      nr2d.holders[generation].table.jTable = jTable;

      Json::Value &jRows = jTable[ROWS];

      auto rowCount = jRows.size();

      auto *pNodes = static_cast<CamEngineA2dnrParamNode_t *>(
          calloc(rowCount, sizeof(CamEngineA2dnrParamNode_t)));

      auto rowCountMatchHdr = 0;

      camdev::CalibHdr &hdr = pCalibration->module<camdev::CalibHdr>();

      for (uint8_t i = 0; i < rowCount; i++) {
        Json::Value &jRow = jRows[i];

        if ((hdr.isEnable && jRow[camdev::CalibNr2d::Table::ColumnV1::Hdr].asInt()) ||
            (!hdr.isEnable && !jRow[camdev::CalibNr2d::Table::ColumnV1::Hdr].asInt())) {
        } else {
          continue;
        }

        auto *pNode = &pNodes[rowCountMatchHdr++];

        pNode->gain = jRow[camdev::CalibNr2d::Table::ColumnV1::Gain].asFloat();
        pNode->integrationTime =
            jRow[camdev::CalibNr2d::Table::ColumnV1::IntergrationTime].asFloat();
        pNode->pregmaStrength =
            jRow[camdev::CalibNr2d::Table::ColumnV1::PregammaStrength].asUInt();
        pNode->strength =
            jRow[camdev::CalibNr2d::Table::ColumnV1::DenoiseStrength].asUInt();
        pNode->sigma = jRow[camdev::CalibNr2d::Table::ColumnV1::Sigma].asFloat();
      }

      if (rowCountMatchHdr) {
        auto ret =
            CamEngineA2dnrSetAutoTable(hCamEngine, pNodes, rowCountMatchHdr);
        free(pNodes);

        REPORT(ret);
      } else {
        free(pNodes);
      }
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/2DNR not support");
      (void)jTable;
      (void)nr2d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dConfigGet(camdev::CalibNr3d::Config &config, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA3dnrMode_t mode = CAM_ENGINE_A3DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      uint8_t strength = 0;
      uint16_t motionFactor = 0;
      uint16_t deltaFactor = 0;

      auto ret = CamEngineA3dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &strength, &motionFactor,
                                      &deltaFactor);
      REPORT(ret);

      camdev::CalibNr3d::Config &nr3dConfig = nr3d.holders[generation].config;

      nr3dConfig.v1.isAuto = mode == CAM_ENGINE_A3DNR_MODE_AUTO;
      nr3dConfig.v1.strength = strength;
      nr3dConfig.v1.motionFactor = motionFactor;
      nr3dConfig.v1.deltaFactor = deltaFactor;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    config = nr3d.holders[generation].config;

    return RET_SUCCESS;
}

int32_t Engine::nr3dConfigSet(camdev::CalibNr3d::Config config, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      uint8_t strength = static_cast<uint8_t>(config.v1.strength);
      uint16_t motionFactor = static_cast<uint16_t>(config.v1.motionFactor);
      uint16_t deltaFactor = static_cast<uint16_t>(config.v1.deltaFactor);

      auto ret = CamEngineA3dnrConfigure(hCamEngine, strength, motionFactor,
                                       deltaFactor);
      REPORT(ret);
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
      nr3d.holders[generation].config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dEnableGet(bool &isEnable, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA3dnrMode_t mode = CAM_ENGINE_A3DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      uint8_t strength = 0;
      uint16_t motionFactor = 0;
      uint16_t deltaFactor = 0;

      auto ret = CamEngineA3dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &strength, &motionFactor,
                                      &deltaFactor);
      REPORT(ret);

      nr3d.holders[generation].isEnable = isRunning == BOOL_TRUE;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    isEnable = nr3d.holders[generation].isEnable;

    return RET_SUCCESS;
}

int32_t Engine::nr3dEnableSet(bool isEnable, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      if (isEnable) {
        CamEngineA3dnrMode_t mode = nr3d.holders[generation].config.v1.isAuto
                                        ? CAM_ENGINE_A3DNR_MODE_AUTO
                                        : CAM_ENGINE_A3DNR_MODE_MANUAL;

        auto ret = CamEngineA3dnrStart(hCamEngine, mode);
        REPORT(ret);
      } else {
        auto ret = CamEngineA3dnrStop(hCamEngine);
        REPORT(ret);
      }
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
      nr3d.holders[generation].isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dReset(camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      nr3d.holders[generation].config.v1.reset();

      auto ret = nr3dConfigSet(nr3d.holders[generation].config, generation);
      REPORT(ret);
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dStatusGet(camdev::CalibNr3d::Status &status, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      bool_t isRunning = BOOL_FALSE;
      CamEngineA3dnrMode_t mode = CAM_ENGINE_A3DNR_MODE_INVALID;
      float gain = 0;
      float intergrationTime = 0;
      uint8_t strength = 0;
      uint16_t motionFactor = 0;
      uint16_t deltaFactor = 0;

      auto ret = CamEngineA3dnrStatus(hCamEngine, &isRunning, &mode, &gain,
                                      &intergrationTime, &strength, &motionFactor,
                                      &deltaFactor);
      REPORT(ret);

      status.gain = gain;
      status.intergrationTime = intergrationTime;
      (void)nr3d;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)nr3d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dTableGet(Json::Value &jTable, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      jTable = nr3d.holders[generation].table.jTable;
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)jTable;
      (void)nr3d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::nr3dTableSet(Json::Value jTable, camdev::CalibNr3d::Generation generation) {
    camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
#if defined CTRL_3DNR
      nr3d.holders[generation].table.jTable = jTable;

      Json::Value &jRows = jTable[ROWS];

      auto rowCount = jRows.size();

      auto *pNodes = static_cast<CamEngineA3dnrParamNode_t *>(
          calloc(rowCount, sizeof(CamEngineA3dnrParamNode_t)));

      auto rowCountMatchHdr = 0;

      camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

      for (uint8_t i = 0; i < rowCount; i++) {
          Json::Value &jRow = jRows[i];

          if ((hdr.isEnable && jRow[camdev::CalibNr3d::Table::ColumnV1::Hdr].asInt()) ||
              (!hdr.isEnable && !jRow[camdev::CalibNr3d::Table::ColumnV1::Hdr].asInt())) {
          } else {
            continue;
          }

          auto *pNode = &pNodes[rowCountMatchHdr++];

          pNode->gain = jRow[camdev::CalibNr3d::Table::ColumnV1::Gain].asFloat();
          pNode->integrationTime =
              jRow[clb::Dnr3::Table::IntergrationTime].asFloat();
          pNode->strength = jRow[camdev::CalibNr3d::Table::ColumnV1::Strength].asUInt();
          pNode->motionFactor =
              jRow[camdev::CalibNr3d::Table::ColumnV1::MotionFactor].asUInt();
          pNode->deltaFactor =
              jRow[camdev::CalibNr3d::Table::ColumnV1::DeltaFactor].asUInt();
      }

      if (rowCountMatchHdr) {
          auto ret = CamEngineA3dnrSetAutoTable(hCamEngine, pNodes, rowCountMatchHdr);
          free(pNodes);

          REPORT(ret);
      } else {
          free(pNodes);
      }
#else
      throw exc::LogicError(RET_NOTSUPP, "Engine/3DNR not support");
      (void)jTable;
      (void)nr3d;
#endif
    }

    return RET_SUCCESS;
}

int32_t Engine::dpccEnableGet(bool &isEnable) {
#if 0
    bool_t isRunning = BOOL_FALSE;
    int32_t ret = CamEngineAdpccStatus(hCamEngine, &isRunning);
    REPORT(ret);

    isEnable = isRunning == BOOL_TRUE;

    camdev::CalibDpcc &dpcc = pOperationHandle->pCalibration->module<camdev::CalibDpcc>();

    dpcc.isEnable = isEnable;
#endif
    return RET_SUCCESS;
}

int32_t Engine::dpccEnableSet(bool isEnable) {
    if (isEnable) {
        int32_t ret = CamEngineAdpccStart(hCamEngine);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineAdpccStop(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibDpcc &dpcc = pOperationHandle->pCalibration->module<camdev::CalibDpcc>();

        dpcc.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::dpfConfigGet(camdev::CalibDpf::Config &config) {
    bool_t isRunning = BOOL_FALSE;

    float gradient = 0;
    float offset = 0;
    float min = 0;
    float div = 0;

    uint8_t sigmaGreen = 0;
    uint8_t sigmaRedBlue = 0;

    int32_t ret = CamEngineAdpfStatus(hCamEngine, &isRunning, &gradient, &offset, &min, &div, &sigmaGreen, &sigmaRedBlue);
    REPORT(ret);

    camdev::CalibDpf &dpf = pOperationHandle->pCalibration->module<camdev::CalibDpf>();

    dpf.isEnable = isRunning == BOOL_TRUE;

    dpf.config.divisionFactor = div;
    dpf.config.gradient = gradient;
    dpf.config.minimumBound = min;
    dpf.config.offset = offset;
    dpf.config.sigmaGreen = sigmaGreen;
    dpf.config.sigmaRedBlue = sigmaRedBlue;

    config = dpf.config;

    return RET_SUCCESS;
}

int32_t Engine::dpfConfigSet(camdev::CalibDpf::Config config) {
    float gradient = config.gradient;
    float offset = config.offset;
    float min = config.minimumBound;
    float div = config.divisionFactor;

    uint8_t sigmaGreen = config.sigmaGreen;
    uint8_t sigmaRedBlue = config.sigmaRedBlue;

    int32_t ret = CamEngineAdpfConfigure(hCamEngine, gradient, offset, min, div, sigmaGreen, sigmaRedBlue);
    REPORT(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibDpf &dpf = pOperationHandle->pCalibration->module<camdev::CalibDpf>();

        dpf.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::dpfEnableGet(bool &isEnable) {
    bool_t isRunning = BOOL_FALSE;

    float gradient = 0;
    float offset = 0;
    float min = 0;
    float div = 0;

    uint8_t sigmaGreen = 0;
    uint8_t sigmaRedBlue = 0;

    int32_t ret = CamEngineAdpfStatus(hCamEngine, &isRunning, &gradient, &offset, &min, &div, &sigmaGreen, &sigmaRedBlue);
    REPORT(ret);

    isEnable = isRunning == BOOL_TRUE;

    camdev::CalibDpf &dpf = pOperationHandle->pCalibration->module<camdev::CalibDpf>();

    dpf.isEnable = isEnable;

    dpf.config.divisionFactor = div;
    dpf.config.gradient = gradient;
    dpf.config.minimumBound = min;
    dpf.config.offset = offset;
    dpf.config.sigmaGreen = sigmaGreen;
    dpf.config.sigmaRedBlue = sigmaRedBlue;

    return RET_SUCCESS;
}

int32_t Engine::dpfEnableSet(bool isEnable) {
    if (isEnable) {
        int32_t ret = CamEngineAdpfStart(hCamEngine);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineAdpfStop(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        pOperationHandle->pCalibration->module<camdev::CalibDpf>().isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::eeConfigGet(camdev::CalibEe::Config &config) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

#if defined CTRL_EE
  bool_t isRunning = BOOL_FALSE;
  CamEngineAeeMode_t mode = CAM_ENGINE_AEE_MODE_INVALID;
  float gain = 0;
  float intergrationTime = 0;
  uint8_t strength = 0;
  uint16_t yUpGain = 0;
  uint16_t yDownGain = 0;
  uint16_t uvGain = 0;
  uint16_t edgeGain = 0;

  auto ret = CamEngineAeeStatus(hCamEngine, &isRunning, &mode, &gain,
                                &intergrationTime, &strength, &yUpGain,
                                &yDownGain, &uvGain, &edgeGain);
  REPORT(ret);

  ee.config.isAuto = mode == CAM_ENGINE_AEE_MODE_AUTO;

  CamEngineEeConfig_t &eeConfig = ee.config.config;

  eeConfig.strength = strength;
  eeConfig.yUpGain = yUpGain;
  eeConfig.yDownGain = yDownGain;
  eeConfig.uvGain = uvGain;
  eeConfig.edgeGain = edgeGain;
#else
  throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
#endif
    config = ee.config;

    return RET_SUCCESS;
}

int32_t Engine::eeConfigSet(camdev::CalibEe::Config config) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

#if defined CTRL_EE
  CamEngineEeConfig_t &eeConfig = config.config;

  uint8_t strength = eeConfig.strength;
  uint16_t yUpGain = eeConfig.yUpGain;
  uint16_t yDownGain = eeConfig.yDownGain;
  uint16_t uvGain = eeConfig.uvGain;
  uint16_t edgeGain = eeConfig.edgeGain;

  auto ret = CamEngineAeeConfigure(hCamEngine, strength, yUpGain, yDownGain,
                                   uvGain, edgeGain);
  REPORT(ret);
#else
  throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
#endif

    if (!pOperationHandle->pCalibration->isReadOnly) {
        ee.config = config;
    }
    return RET_SUCCESS;
}

int32_t Engine::eeEnableGet(bool &isEnable) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

    isEnable = ee.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::eeEnableSet(bool isEnable) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

#if defined CTRL_EE
  if (isEnable) {
    CamEngineAeeMode_t mode = ee.config.isAuto ? CAM_ENGINE_AEE_MODE_AUTO
                                               : CAM_ENGINE_AEE_MODE_MANUAL;

    auto ret = CamEngineAeeStart(hCamEngine, mode);
    REPORT(ret);

    ret = eeConfigSet(ee.config);
    REPORT(ret);
  } else {
    auto ret = CamEngineAeeStop(hCamEngine);

    REPORT_WORET(ret);
  }
#else
  throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
#endif

    if (!pOperationHandle->pCalibration->isReadOnly) {

        ee.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::eeReset() {
#if defined CTRL_EE
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

    ee.config.reset();

    auto ret = eeConfigSet(ee.config);
    REPORT(ret);
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
#endif

    return RET_SUCCESS;
}

int32_t Engine::eeStatusGet(camdev::CalibEe::Status &status) {
#if defined CTRL_EE
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

    bool_t isRunning = BOOL_FALSE;
    CamEngineAeeMode_t mode = CAM_ENGINE_AEE_MODE_INVALID;
    float gain = 0;
    float intergrationTime = 0;
    uint8_t strength = 0;
    uint16_t yUpGain = 0;
    uint16_t yDownGain = 0;
    uint16_t uvGain = 0;
    uint16_t edgeGain = 0;

    auto ret = CamEngineAeeStatus(hCamEngine, &isRunning, &mode, &gain,
                                  &intergrationTime, &strength, &yUpGain,
                                  &yDownGain, &uvGain, &edgeGain);
    REPORT(ret);

    status.gain = gain;
    status.intergrationTime = intergrationTime;

    (void)ee;
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
#endif

    return RET_SUCCESS;
}

int32_t Engine::eeTableGet(Json::Value &jTable) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

#if defined CTRL_EE
    jTable = ee.table.jTable;
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
    (void)jTable;
    (void)ee;
#endif

    return RET_SUCCESS;
}

int32_t Engine::eeTableSet(Json::Value jTable) {
    camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

#if defined CTRL_EE
    ee.table.jTable = jTable;

    Json::Value &jRows = jTable[ROWS];

    auto rowCount = jRows.size();

    auto *pNodes = static_cast<CamEngineAeeParamNode_t *>(
        calloc(rowCount, sizeof(CamEngineAeeParamNode_t)));

    auto rowCountMatchHdr = 0;

    camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    for (uint8_t i = 0; i < rowCount; i++) {
      Json::Value &jRow = jRows[i];

      if ((hdr.isEnable && jRow[camdev::CalibEe::Table::Column::Hdr].asInt()) ||
          (!hdr.isEnable && !jRow[camdev::CalibEe::Table::Column::Hdr].asInt())) {
      } else {
        continue;
      }

      auto *pNode = &pNodes[rowCountMatchHdr++];

      pNode->gain = jRow[camdev::CalibEe::Table::Column::Gain].asFloat();
      pNode->integrationTime = jRow[camdev::CalibEe::Table::IntergrationTime].asFloat();
      pNode->edgeGain = jRow[camdev::CalibEe::Table::Column::EdgeGain].asUInt();
      pNode->strength = jRow[camdev::CalibEe::Table::Column::Strength].asUInt();
      pNode->uvGain = jRow[camdev::CalibEe::Table::Column::UvGain].asUInt();
      pNode->yUpGain = jRow[camdev::CalibEe::Table::Column::YGainUp].asUInt();
      pNode->yDownGain = jRow[camdev::CalibEe::Table::Column::YGainDown].asUInt();
    }

    if (rowCountMatchHdr) {
      auto ret = CamEngineAeeSetAutoTable(hCamEngine, pNodes, rowCountMatchHdr);
      free(pNodes);

      REPORT(ret);
    } else {
      free(pNodes);
    }
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/EE not support");
    (void)jTable;
    (void)ee;
#endif

    return RET_SUCCESS;
}

int32_t Engine::gcConfigGet(camdev::CalibGc::Config &config) {
    camdev::CalibGc &gc = pOperationHandle->pCalibration->module<camdev::CalibGc>();

    config = gc.config;

    return RET_SUCCESS;
}

int32_t Engine::gcConfigSet(camdev::CalibGc::Config config) {

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibGc &gc = pOperationHandle->pCalibration->module<camdev::CalibGc>();
        gc.config = config;
        CamEngineGammaSetCurve(hCamEngine, *(::CamEngineGammaOutCurve_t*)&config.curve);
    }

    return RET_SUCCESS;
}

int32_t Engine::gcEnableGet(bool &isEnable) {
    isEnable = pOperationHandle->pCalibration->module<camdev::CalibGc>().isEnable;
    return RET_SUCCESS;
}

int32_t Engine::gcEnableSet(bool isEnable) {
    if (!pOperationHandle->pCalibration->isReadOnly) {

        pOperationHandle->pCalibration->module<camdev::CalibGc>().isEnable = isEnable;
        if (isEnable) {
            CamEngineGammaEnable(hCamEngine);
        } else {
            CamEngineGammaDisable(hCamEngine);
        }
    }

    return RET_SUCCESS;
}

int32_t Engine::hdrConfigGet(camdev::CalibHdr::Config &config) {
    camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    if (camEngineConfig.type == CAM_ENGINE_CONFIG_SENSOR)
    {
        struct vvcam_mode_info sensor_mode;
        IsiGetSensorModeIss(camEngineConfig.data.sensor.hSensor,&sensor_mode);
        if (sensor_mode.hdr_mode == SENSOR_MODE_HDR_STITCH)
        {
            bool_t isRunning = BOOL_FALSE;
            uint8_t extBit = 0;
            float hdrRatio = 0;

            int32_t ret = CamEngineAhdrStatus(hCamEngine, &isRunning, &extBit, &hdrRatio);
            REPORT(ret);

            hdr.config.extensionBit = extBit;
            hdr.config.exposureRatio = hdrRatio;
        }
    }else{
        throw exc::LogicError(RET_NOTSUPP, "Engine/HDR not support");
    }

	config = hdr.config;

    return RET_SUCCESS;
}

int32_t Engine::hdrConfigSet(camdev::CalibHdr::Config config) {

    camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    if (camEngineConfig.type == CAM_ENGINE_CONFIG_SENSOR)
    {
        struct vvcam_mode_info sensor_mode;
        IsiGetSensorModeIss(camEngineConfig.data.sensor.hSensor,&sensor_mode);
        if (sensor_mode.hdr_mode == SENSOR_MODE_HDR_STITCH)
        {
            uint8_t extBit = config.extensionBit;
            float hdrRatio = config.exposureRatio;
            int32_t ret = CamEngineAhdrConfigure(hCamEngine, extBit, hdrRatio);
            REPORT(ret);
        }
    }else
    {
        throw exc::LogicError(RET_NOTSUPP, "Engine/HDR not support");
    }
    
	if (!pOperationHandle->pCalibration->isReadOnly) {
        hdr.config = config;
	}
    return RET_SUCCESS;
}

int32_t Engine::hdrEnableGet(bool &isEnable) {
    camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    if (camEngineConfig.type == CAM_ENGINE_CONFIG_SENSOR) 
    {
        struct vvcam_mode_info sensor_mode;
        IsiGetSensorModeIss(camEngineConfig.data.sensor.hSensor,&sensor_mode);
        if (sensor_mode.hdr_mode == SENSOR_MODE_HDR_STITCH)
        {
            bool_t isRunning = BOOL_FALSE;
            uint8_t extBit = 0;
            float hdrRatio = 0;

            int32_t ret = CamEngineAhdrStatus(hCamEngine, &isRunning, &extBit, &hdrRatio);
            REPORT(ret);

            hdr.isEnable = isRunning == BOOL_TRUE;
        }
    }   
    else
    {
        throw exc::LogicError(RET_NOTSUPP, "Engine/HDR not support");
    }
    
	isEnable = hdr.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::hdrEnableSet(bool isEnable) {
    camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    if (camEngineConfig.type == CAM_ENGINE_CONFIG_SENSOR) 
    {
        struct vvcam_mode_info sensor_mode;
        IsiGetSensorModeIss(camEngineConfig.data.sensor.hSensor,&sensor_mode);
        if (sensor_mode.hdr_mode == SENSOR_MODE_HDR_STITCH)
        {
            if (isEnable) {
                    int32_t ret = CamEngineAhdrStart(hCamEngine);
                    REPORT(ret);
            } else {
                    int32_t ret = CamEngineAhdrStop(hCamEngine);
                    REPORT(ret);
            }
        }
    }else{
        throw exc::LogicError(RET_NOTSUPP, "Engine/HDR not support");
    }

	if (!pOperationHandle->pCalibration->isReadOnly) {
        hdr.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::hdrReset() {
	//camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();
    if (camEngineConfig.type == CAM_ENGINE_CONFIG_SENSOR)
    {
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/HDR not available");
    }else{
        throw exc::LogicError(RET_NOTSUPP, "Engine/HDR not support");
    }

    return RET_SUCCESS;
}

int32_t Engine::ieConfigGet(camdev::CalibIe::Config &config) {
    camdev::CalibIe &ie = pOperationHandle->pCalibration->module<camdev::CalibIe>();

    config = ie.config;

    return RET_SUCCESS;
}

int32_t Engine::ieConfigSet(camdev::CalibIe::Config config) {
#if defined CTRL_IE
    if (config.config.mode == CAMERIC_IE_MODE_COLOR) {
        int32_t ret = CamEngineImageEffectSetColorSelection(
                hCamEngine, config.config.ModeConfig.ColorSelection.col_selection,
                config.config.ModeConfig.ColorSelection.col_threshold);
        REPORT(ret);
    } else if (config.config.mode == CAMERIC_IE_MODE_SEPIA) {
        int32_t ret = CamEngineImageEffectSetTintCb(
                hCamEngine, config.config.ModeConfig.Sepia.TintCb);
        REPORT(ret);

        ret = CamEngineImageEffectSetTintCr(hCamEngine, config.config.ModeConfig.Sepia.TintCr);
        REPORT(ret);
    } else if (config.config.mode == CAMERIC_IE_MODE_SHARPEN) {
        int32_t ret = CamEngineImageEffectSetSharpen(
                hCamEngine, config.config.ModeConfig.Sharpen.factor,
                config.config.ModeConfig.Sharpen.threshold);
        REPORT(ret);
    }
#else
    //throw exc::LogicError(RET_NOTAVAILABLE, "Engine/IE not open");
#endif

    camdev::CalibIe &ie = pOperationHandle->pCalibration->module<camdev::CalibIe>();

    if (!pOperationHandle->pCalibration->isReadOnly) {
        ie.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::ieEnableGet(bool &isEnable) {
    camdev::CalibIe &ie = pOperationHandle->pCalibration->module<camdev::CalibIe>();

    isEnable = ie.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::ieEnableSet(bool isEnable) {
#if defined CTRL_IE
    if (isEnable) {
        int32_t ret = CamEngineEnableImageEffect(hCamEngine, &ie.config.config);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineDisableImageEffect(hCamEngine);
        REPORT(ret);
    }
#else
    throw exc::LogicError(RET_NOTAVAILABLE, "Engine/IE not open");
#endif

    camdev::CalibIe &ie = pOperationHandle->pCalibration->module<camdev::CalibIe>();

    if (!pOperationHandle->pCalibration->isReadOnly) {
        ie.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::jpeConfigGet(camdev::CalibJpe::Config &config) {
    camdev::CalibJpe &jpe = pOperationHandle->pCalibration->module<camdev::CalibJpe>();

    config = jpe.config;

    return RET_SUCCESS;
}

int32_t Engine::jpeConfigSet(camdev::CalibJpe::Config config) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibJpe &jpe = pOperationHandle->pCalibration->module<camdev::CalibJpe>();

        jpe.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::jpeEnableGet(bool &isEnable) {
    camdev::CalibJpe &jpe = pOperationHandle->pCalibration->module<camdev::CalibJpe>();

    isEnable = jpe.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::jpeEnableSet(bool isEnable) {
    camdev::CalibJpe &jpe = pOperationHandle->pCalibration->module<camdev::CalibJpe>();

    if (isEnable) {
        CamerIcJpeConfig_t JpeConfig = {CAMERIC_JPE_MODE_LARGE_CONTINUOUS,
                                        CAMERIC_JPE_COMPRESSION_LEVEL_HIGH,
                                        CAMERIC_JPE_LUMINANCE_SCALE_DISABLE,
                                        CAMERIC_JPE_CHROMINANCE_SCALE_DISABLE,
                                        jpe.config.width,
                                        jpe.config.height};

        int32_t ret = CamEngineEnableJpe(hCamEngine, &JpeConfig);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineDisableJpe(hCamEngine);
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        jpe.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::lscEnableGet(bool &isEnable) {
    camdev::CalibLsc &lsc = pOperationHandle->pCalibration->module<camdev::CalibLsc>();

    isEnable = lsc.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::lscEnableSet(bool isEnable) {
    int ret = RET_SUCCESS;
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibLsc &lsc = pOperationHandle->pCalibration->module<camdev::CalibLsc>();
        lsc.isEnable = isEnable;
		if (isEnable)
			ret = CamEngineLscEnable(hCamEngine);
		else
			ret = CamEngineLscDisable(hCamEngine);
    }
    return ret;
}

int32_t Engine::pathConfigGet(camdev::CalibPaths::Config &config) {
    camdev::CalibPaths &paths = pOperationHandle->pCalibration->module<camdev::CalibPaths>();

    config = paths.config;

    return RET_SUCCESS;
}

int32_t Engine::pathConfigSet(const camdev::CalibPaths::Config& config) {
    TRACE(CITF_INF, "%s in, set path, state: %d\n", __func__, state);
    if (state >= Idle) {
        int32_t ret =
                CamEngineSetPathConfig(hCamEngine, &config.config[CAM_ENGINE_PATH_MAIN],
                                        &config.config[CAM_ENGINE_PATH_SELF],
                                        &config.config[CAM_ENGINE_PATH_SELF2_BP],
                                        &config.config[CAM_ENGINE_PATH_RDI],
                                        &config.config[CAM_ENGINE_PATH_META]
                                        );
        REPORT(ret);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibPaths &paths = pOperationHandle->pCalibration->module<camdev::CalibPaths>();

        paths.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::pictureOrientationSet(CamerIcMiOrientation_t orientation) {
    switch (orientation) {
    case CAMERIC_MI_ORIENTATION_ORIGINAL:
        return CamEngineOriginal(hCamEngine);

    case CAMERIC_MI_ORIENTATION_HORIZONTAL_FLIP:
        return CamEngineHorizontalFlip(hCamEngine);

    case CAMERIC_MI_ORIENTATION_VERTICAL_FLIP:
        return CamEngineVerticalFlip(hCamEngine);

    case CAMERIC_MI_ORIENTATION_ROTATE90:
        return CamEngineRotateLeft(hCamEngine);

    case CAMERIC_MI_ORIENTATION_ROTATE270:
        return CamEngineRotateRight(hCamEngine);

    default:
        DCT_ASSERT(!"UNKOWN CamerIcMiOrientation_t");
    }

    return RET_SUCCESS;
}

int32_t Engine::reset() {
    camdev::CalibPaths &paths = pOperationHandle->pCalibration->module<camdev::CalibPaths>();

    //paths.reset();

    TRACE(CITF_INF, "%s execute\n", __func__);
    MEMCPY(camEngineConfig.pathConfig, paths.config.config,
                 sizeof(CamEnginePathConfig_t) * CAMERIC_MI_PATH_MAX);

    int32_t ret = CamEngineSetPathConfig(
            hCamEngine, &camEngineConfig.pathConfig[CAM_ENGINE_PATH_MAIN],
            &camEngineConfig.pathConfig[CAM_ENGINE_PATH_SELF],
            &camEngineConfig.pathConfig[CAM_ENGINE_PATH_SELF2_BP],
            &camEngineConfig.pathConfig[CAM_ENGINE_PATH_RDI],
            &camEngineConfig.pathConfig[CAM_ENGINE_PATH_META]
            );
    REPORT_WORET(ret);

    return RET_SUCCESS;
}

int32_t Engine::resolutionSet(CamEngineWindow_t acqWindow,
                            CamEngineWindow_t outWindow,
                            CamEngineWindow_t isWindow,
                            uint32_t numFramesToSkip) {
    int32_t ret = CamEngineSetAcqResolution(hCamEngine, acqWindow, outWindow, isWindow, numFramesToSkip);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t Engine::searchAndLock(CamEngineLockType_t locks) {
    int32_t ret = CamEngineSearchAndLock(hCamEngine, locks);
    REPORT(ret);

    DCT_ASSERT(osEventWait(&eventAcquireLock) == OSLAYER_OK);

    return RET_SUCCESS;
}

int32_t Engine::simpConfigGet(camdev::CalibSimp::Config &config) {
    camdev::CalibSimp &simp = pOperationHandle->pCalibration->module<camdev::CalibSimp>();

    config = simp.config;

    return RET_SUCCESS;
}

int32_t Engine::simpConfigSet(camdev::CalibSimp::Config config) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibSimp &simp = pOperationHandle->pCalibration->module<camdev::CalibSimp>();

        simp.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::simpEnableGet(bool &isEnable) {
    camdev::CalibSimp &simp = pOperationHandle->pCalibration->module<camdev::CalibSimp>();

    isEnable = simp.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::simpEnableSet(bool isEnable) {
    camdev::CalibSimp &simp = pOperationHandle->pCalibration->module<camdev::CalibSimp>();

    if (isEnable) {
        pSimpImage = new Image();

        pSimpImage->load(simp.config.fileName);

        simp.config.config.pPicBuffer = &pSimpImage->picBufMetaData;

        int32_t ret = CamEngineEnableSimp(hCamEngine, &simp.config.config);
        REPORT(ret);
    } else {
        int32_t ret = CamEngineDisableSimp(hCamEngine);
        REPORT(ret);

        if (pSimpImage) {
            delete pSimpImage;
            pSimpImage = nullptr;
        }

        simp.config.config.pPicBuffer = nullptr;
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        simp.isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::start() {
    if (state != Invalid) {
        REPORT(RET_WRONG_STATE);
    }
    // if (camEngineConfig.type == 0)
            reset();

    CamEngineStart(hCamEngine, &camEngineConfig);
    //REPORT(ret);

    DCT_ASSERT(osEventWait(&eventStart) == OSLAYER_OK);

    state = Idle;

    return RET_SUCCESS;
}

int32_t Engine::stop() {
    if (state != Idle) {
        REPORT(RET_WRONG_STATE);
    }

    int32_t ret = CamEngineStop(hCamEngine);
    REPORT(ret);

    DCT_ASSERT(osEventWait(&eventStop) == OSLAYER_OK);

    state = Invalid;

    return RET_SUCCESS;
}

int32_t Engine::streamingStart(uint32_t frames) {
 // if (state != Idle) {
    //    REPORT(RET_WRONG_STATE);
 // }


    int32_t ret = CamEngineStartStreaming(hCamEngine, frames);
    REPORT(ret);

    DCT_ASSERT(osEventWait(&eventStartStreaming) == OSLAYER_OK);

    state = Running;

    return RET_SUCCESS;
}

int32_t Engine::streamingStop() {
    if (state != Running) {
        REPORT(RET_WRONG_STATE);
    }

    int32_t ret = CamEngineStopStreaming(hCamEngine);
    REPORT(ret);

    DCT_ASSERT(osEventWait(&eventStopStreaming) == OSLAYER_OK);

    state = Idle;

    return RET_SUCCESS;
}

int32_t Engine::unlock(CamEngineLockType_t locks) {
    int32_t ret = CamEngineUnlock(hCamEngine, locks);
    REPORT(ret);

    DCT_ASSERT(osEventWait(&eventReleaseLock) == OSLAYER_OK);

    return RET_SUCCESS;
}

int32_t Engine::wbConfigGet(camdev::CalibWb::Config &config) {
    camdev::CalibWb &wb = pOperationHandle->pCalibration->module<camdev::CalibWb>();

    CamEngineCcMatrix_t matrix;

    int32_t ret = CamEngineWbGetCcMatrix(hCamEngine, &matrix);
    REPORT(ret);

    wb.config.ccMatrix = matrix;

    CamEngineCcOffset_t offset;

    ret = CamEngineWbGetCcOffset(hCamEngine, &offset);
    REPORT(ret);

    wb.config.ccOffset = offset;

    CamEngineWbGains_t gains;

    ret = CamEngineWbGetGains(hCamEngine, &gains);
    REPORT(ret);

    wb.config.wbGains = gains;

    config = wb.config;

    return RET_SUCCESS;
}

int32_t Engine::wbConfigSet(camdev::CalibWb::Config config) {
    int32_t ret = CamEngineWbSetCcMatrix(hCamEngine, &config.ccMatrix);
    REPORT(ret);

    ret = CamEngineWbSetCcOffset(hCamEngine, &config.ccOffset);
    REPORT(ret);

    ret = CamEngineWbSetGains(hCamEngine, &config.wbGains);
    REPORT(ret);

    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibWb &wb = pOperationHandle->pCalibration->module<camdev::CalibWb>();

        wb.config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::wdrConfigGet(camdev::CalibWdr::Config &config, camdev::CalibWdr::Generation generation) {
    // throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR3 not ready"); // TODO

    camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

    if (generation == camdev::CalibWdr::Wdr1) {
        config = wdr.holders[camdev::CalibWdr::Wdr1].config;
    } else if (generation == camdev::CalibWdr::Wdr2) {
        config = wdr.holders[camdev::CalibWdr::Wdr2].config;
    } else if (generation == camdev::CalibWdr::Wdr3) {
        config = wdr.holders[camdev::CalibWdr::Wdr3].config;
    } else {
        return RET_INVALID_PARM;
    }

    return RET_SUCCESS;
}

int32_t Engine::wdrConfigSet(camdev::CalibWdr::Config config, camdev::CalibWdr::Generation generation) {
    camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

    if (generation == camdev::CalibWdr::Wdr1) {
        int32_t ret = CamEngineWdrSetCurve(hCamEngine, config.wdr1.curve);
        REPORT(ret);
    } else if (generation == camdev::CalibWdr::Wdr2) {
#if defined ISP_WDR_V2
        int32_t ret = CamEngineWdr2SetStrength(hCamEngine, config.wdr2.strength);
        REPORT(ret);
#else
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR2 not open");
#endif
    } else if (generation == camdev::CalibWdr::Wdr3) {
#if defined ISP_WDR_V3
        int32_t ret = CamEngineWdr3SetStrength(hCamEngine, config.wdr3.strength,
                                                config.wdr3.gainMax,
                                                config.wdr3.strengthGlobal);
        REPORT(ret);
#else
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR3 not open");
#endif
    } else {
        REPORT(RET_INVALID_PARM);
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        wdr.holders[generation].config = config;
    }

    return RET_SUCCESS;
}

int32_t Engine::wdrEnableGet(bool &isEnable, camdev::CalibWdr::Generation generation) {
    camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

    isEnable = wdr.holders[generation].isEnable;

    return RET_SUCCESS;
}

int32_t Engine::wdrEnableSet(bool isEnable, camdev::CalibWdr::Generation generation) {
    camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

    if (generation == camdev::CalibWdr::Wdr1) {
#if defined ISP_WDR_V1 || defined ISP_GWDR
        if (isEnable) {
            int32_t ret = CamEngineEnableWdr(hCamEngine);
            REPORT(ret);
        } else {
            int32_t ret = CamEngineDisableWdr(hCamEngine);
            REPORT(ret);
        }
#else
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/GWDR not open");
#endif
    } else if (generation == camdev::CalibWdr::Wdr2) {
#if defined ISP_WDR_V2
        if (isEnable) {
            int32_t ret = CamEngineEnableWdr2(hCamEngine);
            REPORT(ret);
        } else {
            int32_t ret = CamEngineDisableWdr2(hCamEngine);
            REPORT(ret);
        }
#else
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR2 not open");
#endif
    } else if (generation == camdev::CalibWdr::Wdr3) {
#if defined ISP_WDR_V3
        if (isEnable) {
            int32_t ret = CamEngineEnableWdr3(hCamEngine);
            REPORT(ret);
        } else {
            int32_t ret = CamEngineDisableWdr3(hCamEngine);
            REPORT(ret);
        }
#else
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR3 not open");
#endif
    } else {
        return RET_INVALID_PARM;
    }

    if (!pOperationHandle->pCalibration->isReadOnly) {
        wdr.holders[generation].isEnable = isEnable;
    }

    return RET_SUCCESS;
}

int32_t Engine::wdrReset(camdev::CalibWdr::Generation generation) {
    camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();
    if (generation == camdev::CalibWdr::Wdr1) {
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/GWDR not ready"); // TODO
    } else if (generation == camdev::CalibWdr::Wdr2) {
        throw exc::LogicError(RET_NOTAVAILABLE, "Engine/WDR2 not ready"); // TODO
    } else if (generation == camdev::CalibWdr::Wdr3) {
#if defined CTRL_WDR3
        wdr.holders[generation].config.wdr3.reset();

        wdrConfigSet(wdr.holders[generation].config, generation);
#else
        throw exc::LogicError(RET_NOTSUPP, "Engine/WDR3 not support");
#endif
    } else {
        return RET_INVALID_PARM;
    }

    camdev::CalibWdr noUnusedValue = wdr;

    return RET_SUCCESS;
}


int32_t Engine::wdrStatusGet(camdev::CalibWdr::Status &status,
                             camdev::CalibWdr::Generation generation) {
  camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

  if (generation == camdev::CalibWdr::Wdr1) {
  } else if (generation == camdev::CalibWdr::Wdr2) {
#if defined CTRL_WDR2
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR2 not support");
#endif
  } else if (generation == camdev::CalibWdr::Wdr3) {
#if defined CTRL_WDR3
    bool_t isRunning = BOOL_FALSE;
    CamEngineAwdr3Mode_t mode = CAM_ENGINE_AWDR3_MODE_INVALID;
    float gain = 0;
    float intergrationTime = 0;
    uint8_t strength = 0;
    uint8_t globalStrength = 0;
    uint8_t maxGain = 0;

    auto ret = CamEngineAwdr3Status(hCamEngine, &isRunning, &mode, &gain,
                                    &intergrationTime, &strength,
                                    &globalStrength, &maxGain);
    REPORT(ret);

    status.gain = gain;
    status.intergrationTime = intergrationTime;

    (void)wdr;
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR3 not support");
#endif
  } else {
    return RET_INVALID_PARM;
  }

  return RET_SUCCESS;
}

int32_t Engine::wdrTableGet(Json::Value &jTable,
                            camdev::CalibWdr::Generation generation) {
  camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

  if (generation == camdev::CalibWdr::Wdr1) {
    throw exc::LogicError(RET_NOTSUPP, "Engine/GWDR not support");
  } else if (generation == camdev::CalibWdr::Wdr2) {
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR2 not support");
  } else if (generation == camdev::CalibWdr::Wdr3) {
#if defined ISP_WDR_V3
    jTable = wdr.holders[camdev::CalibWdr::Wdr3].table.jTable;
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR3 not support");
    (void)jTable;
    (void)generation;
#endif
  }

  return RET_SUCCESS;
}

int32_t Engine::wdrTableSet(Json::Value jTable,
                            camdev::CalibWdr::Generation generation) {
  camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

  if (generation == camdev::CalibWdr::Wdr1) {
    throw exc::LogicError(RET_NOTSUPP, "Engine/GWDR not support");
  } else if (generation == camdev::CalibWdr::Wdr2) {
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR2 not support");
  } else if (generation == camdev::CalibWdr::Wdr3) {
#if  defined ISP_WDR_V3
    wdr.holders[camdev::CalibWdr::Wdr3].table.jTable = jTable;

    Json::Value &jRows = jTable[ROWS];

    auto rowCount = jRows.size();

    auto *pNodes = static_cast<CamEngineAwdr3ParamNode_t *>(
        calloc(rowCount, sizeof(CamEngineAwdr3ParamNode_t)));

    auto rowCountMatchHdr = 0;

    camdev::CalibHdr &hdr =pOperationHandle->pCalibration->module<camdev::CalibHdr>();

    for (uint8_t i = 0; i < rowCount; i++) {
      Json::Value &jRow = jRows[i];

      if ((hdr.isEnable && jRow[camdev::CalibWdr::Table::ColumnV3::Hdr].asInt()) ||
          (!hdr.isEnable && !jRow[camdev::CalibWdr::Table::ColumnV3::Hdr].asInt())) {
      } else {
        continue;
      }

      auto *pNode = &pNodes[rowCountMatchHdr++];

      pNode->gain = jRow[camdev::CalibWdr::Table::ColumnV3::Gain].asFloat();
      pNode->integrationTime =
          jRow[camdev::CalibWdr::Table::IntergrationTime].asFloat();
      pNode->strength = jRow[camdev::CalibWdr::Table::ColumnV3::Strength].asUInt();
      pNode->maxGain = jRow[camdev::CalibWdr::Table::ColumnV3::MaxGain].asUInt();
      pNode->globalStrength =
          jRow[camdev::CalibWdr::Table::ColumnV3::GlobalCurve].asUInt();
    }

    if (rowCountMatchHdr) {
      auto ret =
          CamEngineAwdr3SetAutoTable(hCamEngine, pNodes, rowCountMatchHdr);
      free(pNodes);

      REPORT(ret);
    } else {
      free(pNodes);
    }
#else
    throw exc::LogicError(RET_NOTSUPP, "Engine/WDR3 not support");
    (void)jTable;
    (void)generation;
#endif
  }

  return RET_SUCCESS;
}

int32_t Engine::filterConfigGet(camdev::CalibFilter::Config &config) {
    camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();
    config = filter.config;


    return RET_SUCCESS;
}

int32_t Engine::filterConfigSet(camdev::CalibFilter::Config& config) {
#ifndef ISP_DEMOSAIC2
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();
        filter.config = config;
        CamEngineFilterSetLevel(hCamEngine, config.denoise, config.sharpen);
    }
#endif

    return RET_SUCCESS;
}

int32_t Engine::filterEnableGet(bool &isEnable) {
    camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();
    isEnable = filter.isEnable;

    return RET_SUCCESS;
}

int32_t Engine::filterEnableSet(bool isEnable) {
    if (!pOperationHandle->pCalibration->isReadOnly) {
        camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();
        filter.isEnable = isEnable;
		if (isEnable)
        	CamEngineFilterEnable(hCamEngine);
		else
			CamEngineFilterDisable(hCamEngine);
    }

    return RET_SUCCESS;
}

int32_t Engine::filterStatusGet(camdev::CalibFilter::Status &status) {
    bool_t isRunning = BOOL_FALSE;
    CamEngineAfltMode_t mode = CAM_ENGINE_AFLT_MODE_INVALID;
    float gain = 0;
    float intergrationTime = 0;
    uint8_t denoise = 0;
    uint8_t sharpen = 0;

    auto ret = CamEngineAfltStatus(hCamEngine, &isRunning, &mode, &gain,
                                   &intergrationTime, &denoise, &sharpen);
    REPORT(ret);

    status.gain = gain;
    status.intergrationTime = intergrationTime;

    return RET_SUCCESS;
}

int32_t Engine::filterTableGet(Json::Value &jTable) {
    camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();

    jTable = filter.table.jTable;

  return RET_SUCCESS;
}

int32_t Engine::filterTableSet(Json::Value jTable) {
    camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();

  filter.table.jTable = jTable;

  Json::Value &jRows = jTable[ROWS];

  auto rowCount = jRows.size();

  auto *pNodes = static_cast<CamEngineAfltParamNode_t *>(
      calloc(rowCount, sizeof(CamEngineAfltParamNode_t)));

  auto rowCountMatchHdr = 0;

  camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();

  for (uint8_t i = 0; i < rowCount; i++) {
    Json::Value &jRow = jRows[i];

    if ((hdr.isEnable && jRow[camdev::CalibFilter::Table::Column::Hdr].asInt()) ||
        (!hdr.isEnable && !jRow[camdev::CalibFilter::Table::Column::Hdr].asInt())) {
    } else {
      continue;
    }

    auto *pNode = &pNodes[rowCountMatchHdr++];

    pNode->gain = jRow[camdev::CalibFilter::Table::Column::Gain].asFloat();
    pNode->integrationTime =
        jRow[camdev::CalibFilter::Table::IntergrationTime].asFloat();
    pNode->denoiseLevel = jRow[camdev::CalibFilter::Table::Column::Denoising].asUInt();
    pNode->sharpenLevel = jRow[camdev::CalibFilter::Table::Column::Sharpening].asUInt();
  }

  if (rowCountMatchHdr) {
    auto ret = CamEngineAfltSetAutoTable(hCamEngine, pNodes, rowCountMatchHdr);
    free(pNodes);

    REPORT(ret);
  } else {
    free(pNodes);
  }

  return RET_SUCCESS;
}


