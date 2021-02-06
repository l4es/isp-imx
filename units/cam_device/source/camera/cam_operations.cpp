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

#include "cam_operations.hpp"
#include "calib_calibration.hpp"
#include "calib_features.hpp"
#include "ispcore_holder.hpp"
#include "cam_mapbuffer.hpp"
#include "cam_mapcaps.hpp"
#include "macros.hpp"
#include <cameric_reg_drv/cameric_reg_description.h>
#include <version/version.h>

#include "exception.hpp"
#include "cam_common.hpp"
#include "calib_calibration.hpp"
#include "calib_features.hpp"
#include "ispcore_holder.hpp"
#include "cam_mapcaps.hpp"
#include "exception.hpp"
#include "macros.hpp"
#include <algorithm>
#include <functional>
#include <list>

using namespace camdev;

static CamEngineWbGains_t defaultWbGains = {1.0f, 1.0f, 1.0f, 1.0f};
static CamEngineCcMatrix_t defaultCcMatrix = { {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f} };

static CamEngineCcOffset_t defaultCcOffset = {0, 0, 0};

static CamEngineBlackLevel_t defaultBlvl = {0U, 0U, 0U, 0U};

Operation::Operation(Operation_Handle * pOperationHandle_in){
    TRACE_IN;

    pOperationHandle = pOperationHandle_in;
    DCT_ASSERT(pOperationHandle != nullptr);
    DCT_ASSERT(pOperationHandle->pCalibration != nullptr);
    DCT_ASSERT(pOperationHandle->pHalHolder != nullptr);
    
    pBufferCbContext = new BufferCbContext();

    camdev::CalibSensors &calibrationSensors = pOperationHandle->pCalibration->module<camdev::CalibSensors>();

    for (uint i = 0; i < calibrationSensors.sensors.size(); i++) {
        camdev::CalibSensor &calibrationSensor = calibrationSensors.sensors[i];

        sensors.push_back(new SensorOps(calibrationSensor, pOperationHandle->pHalHolder));

        if (!calibrationSensor.config.driverFileName.empty() &&
                !calibrationSensor.config.calibFileName.empty()) {
            sensors[i]->driverChange(calibrationSensor.config.driverFileName, calibrationSensor.config.calibFileName);
        }
    }

#if 0
    camdev::CalibImages &calibrationImages = pOperationHandle->pCalibration->module<camdev::CalibImages>();

    for (uint i = 0; i < calibrationImages.images.size(); i++) {
        images.push_back(new Image());

        if (!calibrationImages.images[i].config.fileName.empty()) {
            images[i]->load(calibrationImages.images[i].config.fileName);
        }
    }
#endif


    TRACE_OUT;
}

Operation::~Operation() {
    TRACE_IN;



    for (auto p : images) {
        delete p;
    }

    images.clear();

    for (auto p : sensors) {
        delete p;
    }

    sensors.clear();

    pOperationHandle = nullptr;



    for (int i = 0; i < CAM_METADATA_CHANNEL; i++) {
        for (auto item : mMetaList[i])
            delete item;
        mMetaList[i].clear();
    }

    delete pBufferCbContext;
    pBufferCbContext = nullptr;

    TRACE_OUT;
}

int Operation::updateMetadata(CamEnginePathType_t path) {
    std::lock_guard<std::mutex> lock(mMetaLock);
    for (auto item : mMetaList[path]) {
        if (!item->filled) {
            mMetaList[path].remove(item);
            pEngine->getMetadata(path, item);
            mMetaList[path].push_back(item);
            item->filled = true;
#if 0
            printf("trace meta aec: %d %d %d %d %d %d\nmean: ", item->exp.enable,
                item->exp.mode, item->exp.rc.x, item->exp.rc.y, item->exp.rc.w, item->exp.rc.h);
            for (int i = 0; i < 25; i++) {
                printf("%d ", item->exp.mean[i]);
            }
            printf("\n");
#endif
            return RET_SUCCESS;
        }
    }

    return RET_FAILURE;
}

struct isp_metadata* Operation::getMetadata(int path) {
    std::lock_guard<std::mutex> lock(mMetaLock);
    for (auto item : mMetaList[path]) {
        if (item->filled) {
            mMetaList[path].remove(item);
            return item;
        }
    }
    return NULL;
}

void Operation::freeMetadata(struct isp_metadata* meta, int path) {
    std::lock_guard<std::mutex> lock(mMetaLock);
    meta->filled = false;
    mMetaList[path].push_back(meta);
}

int32_t Operation::afpsResChangeRequestCb(uint16_t width, uint16_t height, const void *pParam) {
    DCT_ASSERT(pParam);

    int32_t ret = RET_SUCCESS;

    Operation *pCamItf = (Operation *)pParam;

    CamStatus::State oldState = pCamItf->state;

    if (oldState == Running) {
        ret = pCamItf->streamingStop();
        REPORT(ret);
    }

    ret = pCamItf->resolutionSet(width, height);
    REPORT(ret);

    if (oldState == Running) {
        ret = pCamItf->streamingStart();
        REPORT(ret);
    }

    if (pCamItf->pAfpsResChangeCb) {
        pCamItf->pAfpsResChangeCb(pCamItf->pUserCbCtx);
    }

    return RET_SUCCESS;
}

uint Operation::bitstreamId() const {
    return HalReadSysReg(pOperationHandle->pHalHolder->hHal, 0x0000);
}

int32_t Operation::bufferMap(const MediaBuffer_t *pSrcBuffer, PicBufMetaData_t *pDstBuffer) {
    DCT_ASSERT(pSrcBuffer);
    DCT_ASSERT(pDstBuffer);

    PicBufMetaData_t *pMetaData = (PicBufMetaData_t *)(pSrcBuffer->pMetaData);
    DCT_ASSERT(pMetaData);

    int32_t ret = PicBufIsConfigValid(pMetaData);
    REPORT(ret);

    switch (pMetaData->Type) {
    case PIC_BUF_TYPE_RAW8:
    case PIC_BUF_TYPE_RAW16:
        return mapRawBuffer(pOperationHandle->pHalHolder->hHal, pMetaData, pDstBuffer);

    case PIC_BUF_TYPE_YCbCr444:
    case PIC_BUF_TYPE_YCbCr422:
    case PIC_BUF_TYPE_YCbCr420:
    case PIC_BUF_TYPE_YCbCr32:
        return mapYCbCrBuffer(pOperationHandle->pHalHolder->hHal, pMetaData, pDstBuffer);

    case PIC_BUF_TYPE_RGB888:
    case PIC_BUF_TYPE_RGB32:
    case PIC_BUF_TYPE_DATA:
    case PIC_BUF_TYPE_JPEG:
    default:
        break;
    }

    return RET_FAILURE;
}

int32_t Operation::bufferUnmap(PicBufMetaData_t *pData) {
    DCT_ASSERT(pData);

    int32_t ret = PicBufIsConfigValid(pData);
    REPORT(ret);

    switch (pData->Type) {
    case PIC_BUF_TYPE_RAW8:
    case PIC_BUF_TYPE_RAW16:
        return unmapRawBuffer(pOperationHandle->pHalHolder->hHal, pData);

    case PIC_BUF_TYPE_YCbCr444:
    case PIC_BUF_TYPE_YCbCr422:
    case PIC_BUF_TYPE_YCbCr420:
    case PIC_BUF_TYPE_YCbCr32:
        return unmapYCbCrBuffer(pOperationHandle->pHalHolder->hHal, pData);

    case PIC_BUF_TYPE_DATA:
    case PIC_BUF_TYPE_JPEG:
    case PIC_BUF_TYPE_RGB888:
    case PIC_BUF_TYPE_RGB32:
    default:
        break;
    }

    return RET_FAILURE;
}

uint Operation::camerIcId() const {
    uint numRegisters = 0;
    RegDescription_t *pRegistersMap = NULL;

    int32_t ret = CamerIcGetRegisterDescription(CAMERIC_MODULE_MAIN_CONTROL, &numRegisters, &pRegistersMap);
    REPORT(ret);

    uint id = 0;

    ret = CamerIcGetRegister(&pEngine->hCamerIcRegCtx, pRegistersMap[1].Address, &id);
    REPORT(ret);

    return id;
}

int32_t Operation::ecmSet(bool isRestoreState) {

    int32_t ret = RET_SUCCESS;

    State oldState = state;

    if (state == Running) {
        ret = streamingStop();
        REPORT(ret);
    }

    camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

    ret = pEngine->aeEcmSet(ae.ecm);
    REPORT(ret);

    if (isRestoreState && oldState == Running) {
        ret = streamingStart();
        REPORT(ret);
    }

    return RET_SUCCESS;
}

int32_t Operation::inputConnect() {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    CamEngineConfig_t *pCamEngineConfig = &pEngine->camEngineConfig;
    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();
    camdev::CalibAe &ae = pOperationHandle->pCalibration->module<camdev::CalibAe>();

    inputs.input().config.type = camdev::CalibInput::Sensor;
    if (inputs.input().config.type == camdev::CalibInput::Sensor) {
        IsiSensorConfig_t *pSensorConfig = &sensor().config;
        
        pSensorConfig->enableHdr = mConnectInfo.hdr;
        ret = sensor().setup();
        REPORT(ret);

        pCamEngineConfig->type = CAM_ENGINE_CONFIG_SENSOR;

        struct CamEngineConfig_s::CamEngineConfigData_u::CamEngineConfigSensor_s
                *pCamEngineConfigSensor = &pCamEngineConfig->data.sensor;

        pCamEngineConfigSensor->hSensor = sensor().hSensor;
        // pCamEngineConfigSensor->hSubSensor = sensor().holders[1]->hSensor;
        pCamEngineConfigSensor->hCamCalibDb = sensor().calibDb.GetCalibDbHandle();
        if (pSensorConfig->SmiaMode != ISI_SMIA_OFF) {
            pCamEngineConfigSensor->ifSelect = CAMERIC_ITF_SELECT_SMIA;
        } else if (pSensorConfig->SmiaMode != ISI_MIPI_OFF) {
            pCamEngineConfigSensor->ifSelect = CAMERIC_ITF_SELECT_MIPI;
        } else {
            pCamEngineConfigSensor->ifSelect = CAMERIC_ITF_SELECT_PARALLEL;
        }
        if (mConnectInfo.hdr) {
            pCamEngineConfigSensor->ifSelect = CAMERIC_ITF_SELECT_HDR;
        }

        if (sensor().SensorHdrMode == SENSOR_MODE_HDR_STITCH) 
        {
            pCamEngineConfigSensor->ifSelect = CAMERIC_ITF_SELECT_HDR;
            pCamEngineConfigSensor->stitchingMode = sensor().SensorHdrStichMode;
        }

        if (sensor().expand_curve.enable)
        {
            pCamEngineConfigSensor->expand_curve.enable = 1;
            pCamEngineConfigSensor->expand_curve.in_bit = sensor().expand_curve.in_bit;
            pCamEngineConfigSensor->expand_curve.out_bit = sensor().expand_curve.out_bit;
            memcpy(pCamEngineConfigSensor->expand_curve.px, sensor().expand_curve.px,sizeof(sensor().expand_curve.px));
            memcpy(pCamEngineConfigSensor->expand_curve.x_data, sensor().expand_curve.x_data,sizeof(sensor().expand_curve.x_data));
            memcpy(pCamEngineConfigSensor->expand_curve.y_data, sensor().expand_curve.y_data,sizeof(sensor().expand_curve.y_data));
        }else 
        {
            pCamEngineConfigSensor->expand_curve.enable = 0;
        }
    
        pCamEngineConfigSensor->compress_curve.enable = 0;

        IsiResolution_t ImageResolution;
        resolutionGet(ImageResolution.width, ImageResolution.height);

        REFSET(pCamEngineConfigSensor->acqWindow, 0);
        pCamEngineConfigSensor->acqWindow.width = ImageResolution.width;
        pCamEngineConfigSensor->acqWindow.height = ImageResolution.height;
        REFSET(pCamEngineConfigSensor->outWindow, 0);
        pCamEngineConfigSensor->outWindow.width = ImageResolution.width;
        pCamEngineConfigSensor->outWindow.height = ImageResolution.height;
        REFSET(pCamEngineConfigSensor->isWindow, 0);
        pCamEngineConfigSensor->isWindow.width = ImageResolution.width;
        pCamEngineConfigSensor->isWindow.height = ImageResolution.height;

        isiCapValue(pCamEngineConfigSensor->sampleEdge, pSensorConfig->Edge);
        isiCapValue(pCamEngineConfigSensor->hSyncPol, pSensorConfig->HPol);
        isiCapValue(pCamEngineConfigSensor->vSyncPol, pSensorConfig->VPol);
        isiCapValue(pCamEngineConfigSensor->bayerPattern, pSensorConfig->BPat);
        isiCapValue(pCamEngineConfigSensor->subSampling, pSensorConfig->Conv422);
        isiCapValue(pCamEngineConfigSensor->seqCCIR, pSensorConfig->YCSequence);
        isiCapValue(pCamEngineConfigSensor->fieldSelection, pSensorConfig->FieldSelection);
        isiCapValue(pCamEngineConfigSensor->inputSelection, pSensorConfig->BusWidth);
        isiCapValue(pCamEngineConfigSensor->mode, pSensorConfig->Mode);
        isiCapValue(pCamEngineConfigSensor->mipiMode, pSensorConfig->MipiMode);
#ifdef ISP_DVP_PINMAPPING
        switch(sensor().config.BusWidth)
        {
        case 8:
            pCamEngineConfigSensor->dvpPinMapping = CAMERIC_ISP_DVP_PIN_MAPPING_L8TOH8;
            break;
        case 10:
            pCamEngineConfigSensor->dvpPinMapping = CAMERIC_ISP_DVP_PIN_MAPPING_L10TOH10;
            break;
        default:
            pCamEngineConfigSensor->dvpPinMapping = CAMERIC_ISP_DVP_PIN_MAPPING_12TO12;
            break;
        }
#endif

        pCamEngineConfigSensor->enable3D = BOOL_FALSE;
        pCamEngineConfigSensor->enableTestpattern = sensor().isTestPattern() ? BOOL_TRUE : BOOL_FALSE;
        pCamEngineConfigSensor->flickerPeriod = ae.ecm.flickerPeriod;
        pCamEngineConfigSensor->enableAfps = ae.ecm.isAfps ? BOOL_TRUE : BOOL_FALSE;

        pCamEngineConfigSensor->csiPad = 0;
        pCamEngineConfigSensor->csiFormat = sensor().csiFormat;

    } else if (inputs.input().config.type == camdev::CalibInput::Image) {
        pCamEngineConfig->type = CAM_ENGINE_CONFIG_IMAGE;

        struct CamEngineConfig_s::CamEngineConfigData_u::CamEngineConfigImage_s
                *pCamEngineConfigImage = &pCamEngineConfig->data.image;

        pCamEngineConfigImage->pWbGains = &defaultWbGains;
        pCamEngineConfigImage->pCcMatrix = &defaultCcMatrix;
        pCamEngineConfigImage->pCcOffset = &defaultCcOffset;
        pCamEngineConfigImage->pBlvl = &defaultBlvl;

        PicBufMetaData_t *pPicBufMetaData = &image().picBufMetaData;

        pCamEngineConfigImage->fieldSelection = CAMERIC_ISP_FIELD_SELECTION_BOTH;
        pCamEngineConfigImage->is_lsb_align = 1; // pgm file all lsb aligned.

        switch (pPicBufMetaData->Type) {
        case PIC_BUF_TYPE_RAW8:
        case PIC_BUF_TYPE_RAW10:
        case PIC_BUF_TYPE_RAW16:
            pCamEngineConfigImage->pBuffer = pPicBufMetaData->Data.raw.pData;
            pCamEngineConfigImage->width = pPicBufMetaData->Data.raw.PicWidthPixel;
            pCamEngineConfigImage->height = pPicBufMetaData->Data.raw.PicHeightPixel;

            if (pPicBufMetaData->Type == PIC_BUF_TYPE_RAW8) {
                pCamEngineConfigImage->inputSelection = CAMERIC_ISP_INPUT_8BIT_EX;
            } else if (pPicBufMetaData->Type == PIC_BUF_TYPE_RAW16) {
                pCamEngineConfigImage->inputSelection = CAMERIC_ISP_INPUT_12BIT;
            } else {
                pCamEngineConfigImage->inputSelection = CAMERIC_ISP_INPUT_10BIT_EX;
            }
            break;

        default:
            DCT_ASSERT(!"Not supported buffer type");
            break;
        }

        pCamEngineConfigImage->expand_curve.enable = 0;
        pCamEngineConfigImage->compress_curve.enable = 0;

        pCamEngineConfigImage->type = pPicBufMetaData->Type;
        pCamEngineConfigImage->layout = pPicBufMetaData->Layout;
    } else {
        DCT_ASSERT(!"INPUT is NULL");
    }

    ret = pEngine->start();
    REPORT(ret);

    if (inputs.input().config.type == camdev::CalibInput::Sensor) {
        try {
            ret = pEngine->aeEnableSet(ae.isEnable);
            REPORT(ret);

            camdev::CalibAe::Config aeconfig;
            ret = pEngine->aeConfigGet(aeconfig);
            REPORT(ret);

            ret = pEngine->aeConfigSet(aeconfig);
            REPORT(ret);
        } catch (const std::exception &e) {
            TRACE(CITF_ERR, "%s \n", e.what());
        }

        try {
            camdev::CalibAwb &awb = pOperationHandle->pCalibration->module<camdev::CalibAwb>();

            ret = pEngine->awbConfigSet(awb.config);
            REPORT(ret);


            if (!awb.isEnable) {
                camdev::CalibWb &wb = pOperationHandle->pCalibration->module<camdev::CalibWb>();

                ret = pEngine->wbConfigSet(wb.config);
                REPORT(ret);
            }
            ret = pEngine->awbEnableSet(awb.isEnable);
            REPORT(ret);
        } catch (const std::exception &e) {
            TRACE(CITF_ERR, "%s \n", e.what());
        }

        try {
            bool isAfAvailable = false;

            ret = pEngine->afAvailableGet(isAfAvailable);
            REPORT(ret);

            if (isAfAvailable) {
                camdev::CalibAf &af = pOperationHandle->pCalibration->module<camdev::CalibAf>();

                ret = pEngine->afConfigSet(af.config);
                REPORT(ret);

                ret = pEngine->afEnableSet(af.isEnable);
                REPORT(ret);
            }
        } catch (const std::exception &e) {
            TRACE(CITF_ERR, "%s \n", e.what());
        }
    }

    try {
        camdev::CalibDpf &dpf = pOperationHandle->pCalibration->module<camdev::CalibDpf>();

        ret = pEngine->dpfConfigSet(dpf.config);
        REPORT(ret);

        ret = pEngine->dpfEnableSet(dpf.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibDpcc &dpcc = pOperationHandle->pCalibration->module<camdev::CalibDpcc>();

        ret = pEngine->dpccEnableSet(dpcc.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibAvs &avs = pOperationHandle->pCalibration->module<camdev::CalibAvs>();

        ret = pEngine->avsConfigSet(avs.config);
        REPORT(ret);

        ret = pEngine->avsEnableSet(avs.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibHdr &hdr = pOperationHandle->pCalibration->module<camdev::CalibHdr>();
        hdr.isEnable = (sensor().SensorHdrMode == SENSOR_MODE_HDR_STITCH) ? true : false;

        ret = pEngine->hdrConfigSet(hdr.config);
        REPORT(ret);

        ret = pEngine->hdrEnableSet(hdr.isEnable);
        REPORT(ret);

    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibBls &bls = pOperationHandle->pCalibration->module<camdev::CalibBls>();
        ret = pEngine->blsConfigGet(bls.config);
        REPORT(ret);

        ret = pEngine->blsConfigSet(bls.config);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibLsc &lsc = pOperationHandle->pCalibration->module<camdev::CalibLsc>();

        ret = pEngine->lscEnableSet(lsc.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

        ret = pEngine->wdrEnableSet(wdr.holders[camdev::CalibWdr::Wdr2].isEnable, camdev::CalibWdr::Wdr2);
        REPORT(ret);

        ret = pEngine->wdrConfigSet(wdr.holders[camdev::CalibWdr::Wdr2].config, camdev::CalibWdr::Wdr2);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

        ret = pEngine->wdrTableSet(wdr.holders[camdev::CalibWdr::Wdr3].table.jTable, camdev::CalibWdr::Wdr3);
        REPORT(ret);

        ret = pEngine->wdrConfigSet(wdr.holders[camdev::CalibWdr::Wdr3].config, camdev::CalibWdr::Wdr3);
        REPORT(ret);

        ret = pEngine->wdrEnableSet(wdr.holders[camdev::CalibWdr::Wdr3].isEnable, camdev::CalibWdr::Wdr3);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }
#if 0
    try {
        camdev::CalibDemosaic &demosaic = pOperationHandle->pCalibration->module<camdev::CalibDemosaic>();

        ret = pEngine->demosaicConfigSet(demosaic.config);
        REPORT(ret);

        ret = pEngine->demosaicEnableSet(demosaic.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
      camdev::CalibFilter &filter = pOperationHandle->pCalibration->module<camdev::CalibFilter>();

      ret = pEngine->filterTableSet(filter.table.jTable);
      REPORT(ret);

      ret = pEngine->filterConfigSet(filter.config);
      REPORT(ret);

      ret = pEngine->filterEnableSet(filter.isEnable);
      REPORT(ret);
    } catch (const std::exception &e) {
      TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibCac &cac = pOperationHandle->pCalibration->module<camdev::CalibCac>();

        //ret = pEngine->cacConfigSet(cac.config);
        //REPORT(ret);

        ret = pEngine->cacEnableSet(cac.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibCnr &cnr = pOperationHandle->pCalibration->module<camdev::CalibCnr>();

        ret = pEngine->cnrConfigSet(cnr.config);
        REPORT(ret);

        ret = pEngine->cnrEnableSet(cnr.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibNr2d &nr2d = pOperationHandle->pCalibration->module<camdev::CalibNr2d>();

        ret = pEngine->nr2dTableSet(nr2d.holders[camdev::CalibNr2d::Nr2Dv1].table.jTable, camdev::CalibNr2d::Nr2Dv1);
        REPORT(ret);

        ret = pEngine->nr2dConfigSet(nr2d.holders[camdev::CalibNr2d::Nr2Dv1].config, camdev::CalibNr2d::Nr2Dv1);
        REPORT(ret);

        ret = pEngine->nr2dEnableSet(nr2d.holders[camdev::CalibNr2d::Nr2Dv1].isEnable, camdev::CalibNr2d::Nr2Dv1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

        ret = pEngine->nr3dTableSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv1].table.jTable, camdev::CalibNr3d::Nr3Dv1);
        REPORT(ret);

        ret = pEngine->nr3dConfigSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv1].config, camdev::CalibNr3d::Nr3Dv1);
        REPORT(ret);

        ret = pEngine->nr3dEnableSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv1].isEnable, camdev::CalibNr3d::Nr3Dv1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

        ret = pEngine->nr3dConfigSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv2].config, camdev::CalibNr3d::Nr3Dv2);
        REPORT(ret);

        ret = pEngine->nr3dEnableSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv2].isEnable, camdev::CalibNr3d::Nr3Dv2);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibNr3d &nr3d = pOperationHandle->pCalibration->module<camdev::CalibNr3d>();

        ret = pEngine->nr3dConfigSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv3].config, camdev::CalibNr3d::Nr3Dv3);
        REPORT(ret);

        ret = pEngine->nr3dEnableSet(nr3d.holders[camdev::CalibNr3d::Nr3Dv3].isEnable, camdev::CalibNr3d::Nr3Dv3);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibWdr &wdr = pOperationHandle->pCalibration->module<camdev::CalibWdr>();

        ret = pEngine->wdrConfigSet(wdr.holders[camdev::CalibWdr::Wdr1].config, camdev::CalibWdr::Wdr1);
        REPORT(ret);

        ret = pEngine->wdrEnableSet(wdr.holders[camdev::CalibWdr::Wdr1].isEnable, camdev::CalibWdr::Wdr1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }
#endif

    try {
        camdev::CalibGc &gc = pOperationHandle->pCalibration->module<camdev::CalibGc>();

        ret = pEngine->gcConfigSet(gc.config);
        REPORT(ret);

        ret = pEngine->gcEnableSet(gc.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

#if 0
    try {
        camdev::CalibEe &ee = pOperationHandle->pCalibration->module<camdev::CalibEe>();

        ret = pEngine->eeTableSet(ee.table.jTable);
        REPORT(ret);

        ret = pEngine->eeEnableSet(ee.isEnable);
        REPORT(ret);

        if (ee.isEnable) {
            ret = pEngine->eeConfigSet(ee.config);
            REPORT(ret);
        }
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibCproc &cproc = pOperationHandle->pCalibration->module<camdev::CalibCproc>();

        ret = pEngine->cprocConfigSet(cproc.config);
        REPORT(ret);

        ret = pEngine->cprocEnableSet(cproc.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }


    try {
        camdev::CalibIe &ie = pOperationHandle->pCalibration->module<camdev::CalibIe>();

        ret = pEngine->ieConfigSet(ie.config);
        REPORT(ret);

        ret = pEngine->ieEnableSet(ie.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        camdev::CalibSimp &simp = pOperationHandle->pCalibration->module<camdev::CalibSimp>();

        ret = pEngine->simpConfigSet(simp.config);
        REPORT(ret);

        ret = pEngine->simpEnableSet(simp.isEnable);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }
#endif
    state = Idle;

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::inputDisconnect() {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    pOperationHandle->pCalibration->isReadOnly = true;

    try {
        ret = pEngine->simpEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->ieEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->cprocEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->gcEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

#if 0
    try {
        ret = pEngine->eeEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->wdrEnableSet(false, camdev::CalibWdr::Wdr1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->nr3dEnableSet(false, camdev::CalibNr3d::Nr3Dv3);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->nr3dEnableSet(false, camdev::CalibNr3d::Nr3Dv2);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->nr3dEnableSet(false, camdev::CalibNr3d::Nr3Dv1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->nr2dEnableSet(false, camdev::CalibNr2d::Nr2Dv1);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->cnrEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->cacEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
      ret = pEngine->filterEnableSet(false);
      REPORT(ret);
    } catch (const std::exception &e) {
      TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->demosaicEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->wdrEnableSet(false, camdev::CalibWdr::Wdr3);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->wdrEnableSet(false, camdev::CalibWdr::Wdr2);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->lscEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->hdrEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->avsEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->dpccEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }

    try {
        ret = pEngine->dpfEnableSet(false);
        REPORT(ret);
    } catch (const std::exception &e) {
        TRACE(CITF_ERR, "%s \n", e.what());
    }
#endif
    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    if (inputs.input().config.type == camdev::CalibInput::Sensor) {
        bool isAfAvailable = false;

        ret = pEngine->afAvailableGet(isAfAvailable);
        REPORT(ret);

        if (isAfAvailable) {
            ret = pEngine->afEnableSet(false);
            REPORT(ret);
        }

        ret = pEngine->awbEnableSet(false);
        REPORT(ret);

        ret = pEngine->aeEnableSet(false);
        REPORT(ret);
    }

    pOperationHandle->pCalibration->isReadOnly = false;

    ret = pEngine->stop();
    REPORT(ret);

    state = Init;

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::inputSwitch(int32_t index) {
    TRACE_IN;

    if (index >= ISP_INPUT_MAX) {
        return RET_INVALID_PARM;
    }

    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    inputs.config.index = index;

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::reset() {
    TRACE_IN;

    pEngine->reset();

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::resolutionGet(uint16_t &width, uint16_t &height) {
    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    if (inputs.input().config.type == camdev::CalibInput::Sensor) {
        
        width = sensor().config.Resolution.width;
        height = sensor().config.Resolution.height;

        return RET_SUCCESS;
    } else if (inputs.input().config.type == camdev::CalibInput::Image) {
        PicBufPlane_t *pRaw = &image().picBufMetaData.Data.raw;

        width = pRaw->PicWidthPixel;
        height = pRaw->PicHeightPixel;

        return RET_SUCCESS;
    } else {
        return RET_FAILURE;
    }
}

int32_t Operation::resolutionSet(uint16_t width, uint16_t height) {

    int32_t ret = RET_SUCCESS;
    uint8_t numFramesToSkip = 0;

    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    if (inputs.input().config.type == camdev::CalibInput::Sensor) {
        ret = sensor().resolutionSet(width, height);
        if (ret != RET_SUCCESS) {
            TRACE(CITF_ERR, "Sensor resolution set error: %d \n", ret);
        }
    }

    CamEngineWindow_t acqWindow;
    REFSET(acqWindow, 0);
    acqWindow.width = width;
    acqWindow.height = height;

    CamEngineWindow_t outWindow;
    REFSET(outWindow, 0);
    outWindow.width = width;
    outWindow.height = height;

    CamEngineWindow_t isWindow;
    REFSET(isWindow, 0);
    isWindow.width = width;
    isWindow.height = height;

    ret = pEngine->resolutionSet(acqWindow, outWindow, isWindow, numFramesToSkip);
    if (ret != RET_SUCCESS) {
        TRACE(CITF_ERR, "Engine resolution set error: %d \n", ret);
    }

    return RET_SUCCESS;
}

int32_t Operation::streamingStart(uint frames) {
    TRACE_IN;

    if (state >= Running) {
        return RET_SUCCESS;
    }

    int32_t ret = RET_SUCCESS;

    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    ret = pEngine->streamingStart(frames);
    REPORT(ret);

    if (inputs.input().config.type == camdev::CalibInput::Sensor &&
            sensor().state < Running) {
        ret = sensor().streamEnableSet(true);
        REPORT(ret);
    }

    state = Running;

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::streamingStop() {
    TRACE_IN;
    int32_t ret = RET_SUCCESS;

    ret = pEngine->streamingStop();
    REPORT(ret);

    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();

    if (inputs.input().config.type == camdev::CalibInput::Sensor &&
            sensor().state >= Running) {
        ret = sensor().streamEnableSet(false);
        REPORT(ret);
    }

    state = Idle;

    TRACE_OUT;

    return RET_SUCCESS;
}




Image::Image() {
    TRACE_IN;

    REFSET(picBufMetaData, 0);

    TRACE_OUT;
}

Image::~Image() {
    TRACE_IN;

    clean();

    TRACE_OUT;
}

void Image::clean() {
    if (picBufMetaData.Data.raw.pData) {
        free(picBufMetaData.Data.raw.pData);

        picBufMetaData.Data.raw.pData = nullptr;
    }

    REFSET(picBufMetaData, 0);
}

void Image::load(std::string fileName) {
    if (!fs::isExists(fileName)) {
        throw exc::LogicError(RET_INVALID_PARM, "No such file" + fileName);
    }

    clean();

    if (!PGM_ReadRaw(fileName.c_str(), &picBufMetaData)) {
        throw exc::LogicError(RET_FAILURE, "Load file failed: " + fileName);
    }
}

void Operation::bufferCb(CamEnginePathType_t path, MediaBuffer_t *pMediaBuffer, void *pCtx) {

    if (!pMediaBuffer->pOwner) {
        TRACE_OUT;
        return;
    }

    std::list<ItfBufferCb *> *pList = nullptr;
    Operation* pOperation = static_cast<Operation *>(pCtx);
    BufferCbContext *pBufferCbContext = pOperation->pBufferCbContext;

    if (path == CAM_ENGINE_PATH_MAIN) {
        pList = &pBufferCbContext->mainPath;
    } else if (path == CAM_ENGINE_PATH_SELF) {
        pList = &pBufferCbContext->selfPath1;
    } else if (path == CAM_ENGINE_PATH_SELF2_BP) {
        pList = &pBufferCbContext->selfPath2;
    }

    // pOperation->updateMetadata(path);

    std::for_each(pList->begin(), pList->end(), std::bind(std::mem_fun(&ItfBufferCb::bufferCb), std::placeholders::_1, pMediaBuffer));
}

int32_t Operation::cameraConnect(bool preview, ItfBufferCb * bufferCb, const ConnectInfo& connectInfo) {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    bool HdrIsEnable = (sensor().SensorHdrMode ==SENSOR_MODE_HDR_STITCH) ? true : false;

    DCT_ASSERT(pEngine = new Engine(pOperationHandle,HdrIsEnable));
    pEngine->pAfpsResChangeRequestCb = afpsResChangeRequestCb;
    pEngine->pUserCbCtx = this;

    state = Init;


    pEngine->bufferCbRegister(this->bufferCb, (void *)this);

    for (int i = 0; i < CAM_METADATA_CHANNEL; i++) {
        for (int j = 0; j < CAM_METADATA_NUMBER; j++) {
            struct isp_metadata *meta = new struct isp_metadata;
            memset(meta, 0, sizeof(*meta));
            mMetaList[i].push_back(meta);
        }
    }

    camdev::CalibInputs &inputs = pOperationHandle->pCalibration->module<camdev::CalibInputs>();
    inputs.input().config.type = camdev::CalibInput::Sensor;
    
    camdev::CalibInput::Type type = pOperationHandle->pCalibration->module<camdev::CalibInputs>().input().config.type;
    if (preview) {
        if (type == camdev::CalibInput::Sensor) {
            sensor().checkValid().reset();
        }
        
        IsiResolution_t ImageResolution;
        resolutionGet(ImageResolution.width, ImageResolution.height);
        
        camdev::CalibPaths::Config config;

        switch (connectInfo.format) {
        case CAMERA_PIX_FMT_YUV422SP:
            miMode = CAMERIC_MI_DATAMODE_YUV422;
            miLayout = CAMERIC_MI_DATASTORAGE_SEMIPLANAR;
            break;
        case CAMERA_PIX_FMT_YUV422I:
            miMode = CAMERIC_MI_DATAMODE_YUV422;
            miLayout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
            break;
        case CAMERA_PIX_FMT_YUV420SP:
            miMode = CAMERIC_MI_DATAMODE_YUV420;
            miLayout = CAMERIC_MI_DATASTORAGE_SEMIPLANAR;
            break;
        case CAMERA_PIX_FMT_RAW8:
            miMode = CAMERIC_MI_DATAMODE_RAW8;
            miLayout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
            break;
        case CAMERA_PIX_FMT_RAW10:
            miMode = CAMERIC_MI_DATAMODE_RAW10;
            miLayout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
            break;
        case CAMERA_PIX_FMT_RAW12:
            miMode = CAMERIC_MI_DATAMODE_RAW12;
            miLayout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
            break;
        default:
            break;
        }

        type = camdev::CalibInput::Sensor;
        using modeType = std::remove_reference<decltype((config.config[CAMERIC_MI_PATH_MAIN].mode))>::type;
        using layoutType = std::remove_reference<decltype((config.config[CAMERIC_MI_PATH_MAIN].layout))>::type;
        if (type == camdev::CalibInput::Sensor) {
            config.config[CAMERIC_MI_PATH_MAIN].mode = (modeType)miMode;
            config.config[CAMERIC_MI_PATH_MAIN].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_MAIN].width = ImageResolution.width;
            config.config[CAMERIC_MI_PATH_MAIN].height = ImageResolution.height;
            config.config[CAMERIC_MI_PATH_MAIN].alignMode = CAMERIC_MI_PIXEL_UN_ALIGN;

            config.config[CAMERIC_MI_PATH_SELF].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_SELF].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_SELF].width = 0;
            config.config[CAMERIC_MI_PATH_SELF].height = 0;
            config.config[CAMERIC_MI_PATH_SELF].alignMode = CAMERIC_MI_PIXEL_UN_ALIGN;

            config.config[CAMERIC_MI_PATH_SELF2_BP].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_SELF2_BP].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_SELF2_BP].width = 0;
            config.config[CAMERIC_MI_PATH_SELF2_BP].height = 0;
            config.config[CAMERIC_MI_PATH_SELF2_BP].alignMode = CAMERIC_MI_PIXEL_UN_ALIGN;

            config.config[CAMERIC_MI_PATH_RDI].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_RDI].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_RDI].width = 0;
            config.config[CAMERIC_MI_PATH_RDI].height = 0;

            config.config[CAMERIC_MI_PATH_META].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_META].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_META].width = 0;
            config.config[CAMERIC_MI_PATH_META].height = 0;

        } else {
            config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_MAIN].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_MAIN].width = 0;
            config.config[CAMERIC_MI_PATH_MAIN].height = 0;

            config.config[CAMERIC_MI_PATH_SELF].mode = (modeType)miMode;
            config.config[CAMERIC_MI_PATH_SELF].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_SELF].width = ImageResolution.width;
            config.config[CAMERIC_MI_PATH_SELF].height = ImageResolution.height;

            config.config[CAMERIC_MI_PATH_SELF2_BP].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_SELF2_BP].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_SELF2_BP].width = 0;
            config.config[CAMERIC_MI_PATH_SELF2_BP].height = 0;

            config.config[CAMERIC_MI_PATH_RDI].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_RDI].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_RDI].width = 0;
            config.config[CAMERIC_MI_PATH_RDI].height = 0;

            config.config[CAMERIC_MI_PATH_META].mode = CAMERIC_MI_DATAMODE_DISABLED;
            config.config[CAMERIC_MI_PATH_META].layout = (layoutType)miLayout;
            config.config[CAMERIC_MI_PATH_META].width = 0;
            config.config[CAMERIC_MI_PATH_META].height = 0;
        }

        CamEngineConfig_t *pCamEngineConfig = &pEngine->camEngineConfig;

        camdev::CalibPaths &paths = pOperationHandle->pCalibration->module<camdev::CalibPaths>();

        memcpy(pCamEngineConfig->pathConfig, config.config, sizeof(CamEnginePathConfig_t) * CAMERIC_MI_PATH_MAX);
        memcpy(paths.config.config, config.config, sizeof(CamEnginePathConfig_t) * CAMERIC_MI_PATH_MAX);
        ret = pEngine->pathConfigSet(config);
        REPORT(ret);
    }

    std::list<ItfBufferCb *> *pList = nullptr;

    if (type == camdev::CalibInput::Sensor) {
        pList = &pBufferCbContext->mainPath;
    } else {
        pList = &pBufferCbContext->selfPath1;
    }
     if (bufferCb) {
        pList->push_back(bufferCb);
    }

    ret = inputConnect();
    REPORT(ret);

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::cameraDisconnect() {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    ret = inputDisconnect();
    REPORT(ret);

    ret = pEngine->bufferCbUnregister();
    DCT_ASSERT(ret == RET_SUCCESS);

    if (pBufferCbContext) {
        pBufferCbContext->mainPath.clear();
        pBufferCbContext->selfPath1.clear();
        pBufferCbContext->selfPath2.clear();
        pBufferCbContext->rdiPath.clear();
    }

    if (pEngine) {
        delete pEngine;
    }

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::cameraReset() {
    TRACE_IN;

//    ret = cameraDisconnect();
    //REPORT(ret);

 // ret = reset();
 // REPORT(ret);

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::captureDma(std::string filename, CAMDEV_SnapshotType snapshotType) {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    // CamEnginePathConfig_t curMpConfig;

    // ret = pEngine->pathConfigGet(curMpConfig);
    // REPORT(ret);

    // CamEnginePathConfig_t mpConfig;
    // REFSET(mpConfig, 0);

    // switch (snapshotType) {
    // case RGB:
    //     mpConfig.mode = CAMERIC_MI_DATAMODE_YUV422;
    //     mpConfig.layout = CAMERIC_MI_DATASTORAGE_SEMIPLANAR;
    //     break;

    // case RAW8:
    //     mpConfig.mode = CAMERIC_MI_DATAMODE_RAW8;
    //     mpConfig.layout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
    //     break;

    // case RAW12:
    //     mpConfig.mode = CAMERIC_MI_DATAMODE_RAW12;
    //     mpConfig.layout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
    //     break;

    // case JPEG:
    //     mpConfig.mode = CAMERIC_MI_DATAMODE_JPEG;
    //     mpConfig.layout = CAMERIC_MI_DATASTORAGE_INTERLEAVED;
    //     break;

    // default:
    //     return RET_NOTSUPP;
    // }

    // uint16_t width = 0;
    // uint16_t height = 0;

    // ret = resolutionGet(width, height);
    // REPORT(ret);

    // if (snapshotType == JPEG) {
    //     JPE.config.width = width; // TODO
    //     JPE.config.height = height;

    //     ret = pEngine->jpeConfigSet(JPE.config);
    //     REPORT(ret);

    //     ret = pEngine->jpeEnableSet(true);
    //     REPORT(ret);
    // }

    // ret = pEngine->pathConfigSet(mpConfig);
    // REPORT(ret);

    // remove callbacks, add som ctrl callback
    std::list<ItfBufferCb *> pathBackup = pBufferCbContext->mainPath;

    pBufferCbContext->mainPath.clear();



    ret = streamingStart(1);
    REPORT(ret);

    pBufferCbContext->mainPath = pathBackup;

    // ret = pEngine->pathConfigSet(curMpConfig);
    // REPORT(ret);

    // if (JPEG == snapshotType) {
    //     ret = pEngine->jpeEnableSet(false);
    //     REPORT(ret);
    // }

    TRACE_OUT;

    return ret;
}

int32_t Operation::captureSensor(std::string fileName, CAMDEV_SnapshotType snapshotType, uint32_t resolution, CamEngineLockType_t lockType) {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    if (resolution && state != Running) {
        throw exc::LogicError(RET_WRONG_STATE, "Start preview first, then capture it");
    }

    IsiResolution_t ImageResolution;
    resolutionGet(ImageResolution.width, ImageResolution.height);

    // pCurSensorConfig->Resolution == 0 if DMA mode

    camdev::CalibInput::Type &type = pOperationHandle->pCalibration->module<camdev::CalibInputs>().input().config.type;

    ret = streamingStop();
    REPORT(ret);

    ret = resolutionSet(ImageResolution.width,ImageResolution.height);
    REPORT(ret);

    camdev::CalibPaths::Config curConfig;

    ret = pEngine->pathConfigGet(curConfig);
    REPORT(ret);

    camdev::CalibPaths::Config config;

    REFSET(config, 0);

    switch (snapshotType) {
    case CAMDEV_RGB:
        config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_YUV420;
        config.config[CAMERIC_MI_PATH_MAIN].layout =
                CAMERIC_MI_DATASTORAGE_SEMIPLANAR;
        break;

    case CAMDEV_RAW8:
        config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_RAW8;
        config.config[CAMERIC_MI_PATH_MAIN].layout =
                CAMERIC_MI_DATASTORAGE_INTERLEAVED;
        break;

    case CAMDEV_RAW10:
        config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_RAW10;
        config.config[CAMERIC_MI_PATH_MAIN].layout =
                CAMERIC_MI_DATASTORAGE_INTERLEAVED;
        break;

    case CAMDEV_RAW12:
        config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_RAW12;
        config.config[CAMERIC_MI_PATH_MAIN].layout =
                CAMERIC_MI_DATASTORAGE_INTERLEAVED;
        break;

    case CAMDEV_JPEG:
        config.config[CAMERIC_MI_PATH_MAIN].mode = CAMERIC_MI_DATAMODE_JPEG;
        config.config[CAMERIC_MI_PATH_MAIN].layout =
                CAMERIC_MI_DATASTORAGE_INTERLEAVED;
        break;

    default:
        return RET_NOTSUPP;
    }

    config.config[CAMERIC_MI_PATH_SELF].mode = CAMERIC_MI_DATAMODE_DISABLED;
    config.config[CAMERIC_MI_PATH_SELF].layout = CAMERIC_MI_DATASTORAGE_PLANAR;

    if (snapshotType == CAMDEV_JPEG) {
        camdev::CalibJpe &jpe = pOperationHandle->pCalibration->module<camdev::CalibJpe>();

        jpe.config.width = ImageResolution.width; // TODO
        jpe.config.height = ImageResolution.height;

        ret = pEngine->jpeConfigSet(jpe.config);
        REPORT(ret);

        ret = pEngine->jpeEnableSet(true);
        REPORT(ret);
    }


    ret = pEngine->pathConfigSet(config);
    REPORT(ret);

    // remove callbacks, add som ctrl callback
    std::list<ItfBufferCb *> mainPathBackup = pBufferCbContext->mainPath;
    std::list<ItfBufferCb *> selfPath1Backup = pBufferCbContext->selfPath1;
    std::list<ItfBufferCb *> selfPath2Backup = pBufferCbContext->selfPath2;
    std::list<ItfBufferCb *> rdiPathBackup = pBufferCbContext->rdiPath;
    std::list<ItfBufferCb *> metaPathBackup = pBufferCbContext->metaPath;

    pBufferCbContext->mainPath.clear();
    pBufferCbContext->selfPath1.clear();
    pBufferCbContext->selfPath2.clear();
    pBufferCbContext->rdiPath.clear();
    pBufferCbContext->metaPath.clear();

    ret = streamingStart();
    REPORT(ret);

    if (type == camdev::CalibInput::Sensor && !sensor().isTestPattern()) {
        ret = pEngine->searchAndLock(lockType);
        REPORT(ret);
    }




    if (type == camdev::CalibInput::Sensor && !sensor().isTestPattern()) {
        ret = pEngine->unlock(lockType);
        REPORT(ret);
    }

    ret = streamingStop();
    REPORT(ret);

    ret = resolutionSet(ImageResolution.width,ImageResolution.height);
    REPORT(ret);

    pBufferCbContext->mainPath = mainPathBackup;
    pBufferCbContext->selfPath1 = selfPath1Backup;
    pBufferCbContext->selfPath2 = selfPath2Backup;
    pBufferCbContext->rdiPath = rdiPathBackup;
    pBufferCbContext->metaPath = metaPathBackup;

    if (CAMDEV_JPEG == snapshotType) {
        ret = pEngine->jpeEnableSet(false);
        REPORT(ret);
    }

    ret = pEngine->pathConfigSet(curConfig);
    REPORT(ret);

    ret = streamingStart();
    REPORT(ret);

    TRACE_OUT;

    return RET_SUCCESS;
}

int32_t Operation::previewStart() {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    ret = streamingStart();
    REPORT(ret);
    TRACE_OUT;

    return RET_SUCCESS;
}


int32_t Operation::previewStop() {
    TRACE_IN;

    int32_t ret = RET_SUCCESS;

    ret = streamingStop();
    REPORT(ret);


    TRACE_OUT;

    return RET_SUCCESS;
}
