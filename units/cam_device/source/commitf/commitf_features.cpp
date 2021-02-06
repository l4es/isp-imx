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

#include "calib_features.hpp"
#include "commitf_features.hpp"
#include "meta_builder.hpp"
using namespace camdev;

RESULT CitfDevice::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    switch (ctrlId) {
    case ISPCORE_MODULE_DEVICE_CALIBRATIONSAVE     : return calibrationSave(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CALIBRATIONLOAD     : return calibrationLoad(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_INITENGINEOPERATION : return initEngineOperation(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAPTUREDMA          : return captureDma(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAPTURESENSOR       : return captureSensor(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_INPUTINFO           : return inputInfo(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_INPUTSWITCH         : return inputSwitch(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_PREVIEW             : return preview(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_BITSTREAMID         : return bitstreamId(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAMERIC_ID          : return camerIcId(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_INPUT_CONNECT       : return inputConnect(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_INPUT_DISCONNECT    : return inputDisconnect(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_RESET               : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_RESOLUTION_GET      : return resolutionGet(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_RESOLUTION_SET      : return resolutionSet(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_STREAMING_START     : return streamingStart(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_STREAMING_STOP      : return streamingStop(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT   : return cameraDisconnect(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAMERA_RESET        : return cameraReset(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_PREVIEW_START       : return previewStart(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_PREVIEW_STOP        : return previewStop(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CAMERA_CONNECT      : return cameraConnect(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_GET_HALHANDLE       : return getHalHandle(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_GET_METADATA        : return getMetadata(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT    : return calibrationLoadandInit(jRequest, jResponse);
    case ISPCORE_MODULE_DEVICE_RESOLUTION_SET_SENSOR_ISP : return resolutionSetsSensorAndIsp(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfDevice::calibrationSave(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (!pCitfHandle || !pCitfHandle->pOperation) {
        jResponse[DEVICE_CALIBRATION_FILE_PARAMS] = "";
        return RET_NULL_POINTER;
    }
    std::string fileName =
        pCitfHandle->pOperation->sensor().checkValid().calibrationSensor.config.calibFileName;

    pCitfHandle->pCalibration->load(fileName);

    fileName = "calib.xml";
    pCitfHandle->pCalibration->store(fileName);
    jResponse[DEVICE_CALIBRATION_FILE_PARAMS] = fileName;

    return RET_SUCCESS;
}

RESULT CitfDevice::calibrationLoad(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    std::string fileName = jRequest[DEVICE_CALIBRATION_FILE_PARAMS].asString();

    if (pCitfHandle->pCalibration != nullptr) {
        //delete pCitfHandle->pCalibration;
        return RET_SUCCESS;
    }
    try {
        pCitfHandle->pCalibration = new camdev::Calibration();
        pCitfHandle->pCalibration->load(fileName);
    } catch (const exc::LogicError &e) {
        if (e.error == RET_FAILURE) {
            return false;
        }
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

RESULT CitfDevice::initEngineOperation(Json::Value &jRequest, Json::Value &jResponse) {
      TRACE_CMD;
      if (pCitfHandle->pOperation != nullptr) {
          //delete pCitfHandle->pOperation;
          return RET_SUCCESS;
      }

      if (pCitfHandle->pHalHolder == nullptr) {
          TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
          return RET_NULL_POINTER;
      }

      Operation_Handle * pOperationHandle = (Operation_Handle *)pCitfHandle;
      pCitfHandle->pOperation = new camdev::Operation(pOperationHandle);
      return RET_SUCCESS;
}

RESULT CitfDevice::resolutionSetsSensorAndIsp(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    uint16_t width = jRequest[DEVICE_WIDTH_PARAMS].asUInt();
    uint16_t height = jRequest[DEVICE_HEIGHT_PARAMS].asUInt();
    return pCitfHandle->pOperation->resolutionSet(width,height);
}

RESULT CitfDevice::calibrationLoadandInit(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    if (pCitfHandle->pCalibration != nullptr) {
        //delete pCitfHandle->pCalibration;
        return RET_SUCCESS;
    }

    pCitfHandle->pCalibration = new camdev::Calibration();
    char szCalibFile[256];
    HalGetSensorCalibXmlName(pCitfHandle->pHalHolder->hHal, szCalibFile, 256);
    pCitfHandle->pCalibration->load(szCalibFile);
    if (pCitfHandle->pHalHolder == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    Operation_Handle * pOperationHandle = (Operation_Handle *)pCitfHandle;
    pCitfHandle->pOperation = new camdev::Operation(pOperationHandle);


 // if (deviceStateBackup >= CamStatus::Running) {
     // pCitfHandle->pOperation->previewStart();
 // }

    return 0;
}

RESULT CitfDevice::cameraConnect(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pOperation, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;;
    }

    bool preview = jRequest[DEVICE_CAMCONNECT_PREVIEW].asBool();
    ItfBufferCb* pBufferCb = (ItfBufferCb*)jRequest[DEVICE_CAMCONNECT_PREVIEW_CALLBACK].asUInt64();


    Operation::ConnectInfo info;
    info.width = jRequest[DEVICE_WIDTH_PARAMS].asUInt();
    info.height = jRequest[DEVICE_HEIGHT_PARAMS].asUInt();
    info.format = jRequest[DEVICE_FORMAT_PARAMS].asUInt();

    RESULT ret = pCitfHandle->pOperation->cameraConnect(preview, pBufferCb, info);
    return ret;
}

RESULT CitfDevice::captureDma(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    std::string fileName = jRequest[DEVICE_FILE_PARAMS].asString();

    // if (pOperation
    //   delete pCitfHandle->pOperation // }

    // pCitfHandle->pOperation new Device;

    pCitfHandle->pOperation->cameraDisconnect();

    camdev::CalibImages &images = pCitfHandle->pCalibration->module<camdev::CalibImages>();

    images.images[pCitfHandle->pCalibration->module<camdev::CalibInputs>().config.index]
        .config.fileName = fileName;

    pCitfHandle->pOperation->image().load(fileName);

    camdev::CalibInputs &inputs = pCitfHandle->pCalibration->module<camdev::CalibInputs>();

    // camdev::Input::Config configBackup = input.config;

    inputs.input().config.type = camdev::CalibInput::Image;

    int rc = 0; //pCitfHandle->pOperation->cameraConnect(false);

    CAMDEV_SnapshotType snapshotType =
        (CAMDEV_SnapshotType)jRequest[DEVICE_SNAPSHOT_TYPE_PARAMS].asInt();

    rc = pCitfHandle->pOperation->captureDma(fileName, snapshotType);

    jResponse[DEVICE_FILE_PARAMS] = fileName;

    // input.config = configBackup;

    return rc;
}

RESULT CitfDevice::captureSensor(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;

    std::string fileName = jRequest[DEVICE_FILE_PARAMS].asString();
    CAMDEV_SnapshotType snapshotType = (CAMDEV_SnapshotType)jRequest[DEVICE_SNAPSHOT_TYPE_PARAMS].asInt();
    uint32_t resolution = jRequest[DEVICE_RESOLUTION_PARAMS].asUInt();
    CamEngineLockType_t lockType = (CamEngineLockType_t)jRequest[DEVICE_LOCK_TYPE_PARAMS].asInt();

    int rc = pCitfHandle->pOperation->captureSensor(fileName, snapshotType, resolution, lockType);

    if (snapshotType == CAMDEV_RGB) {
        fileName += ".ppm";
    } else if (snapshotType == CAMDEV_RAW8 ||
               snapshotType == CAMDEV_RAW12) {
        fileName += ".pgm";
    }

    jResponse[DEVICE_FILE_PARAMS] = fileName;
    return rc;
}

RESULT CitfDevice::inputInfo(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (!pCitfHandle || !pCitfHandle->pCalibration)
        return RET_NULL_POINTER;
    int32_t ret = RET_SUCCESS;

    jResponse[DEVICE_COUNT_PARAMS] = static_cast<int32_t>(pCitfHandle->pOperation->sensors.size());
    jResponse[DEVICE_INDEX_PARAMS] = pCitfHandle->pCalibration->module<camdev::CalibInputs>().config.index;

    return ret;
}

RESULT CitfDevice::inputSwitch(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (!pCitfHandle || !pCitfHandle->pCalibration || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    int32_t index = jRequest[DEVICE_INDEX_PARAMS].asInt();

    int rc = pCitfHandle->pOperation->inputSwitch(index);

    jResponse[DEVICE_INPUT_TYPE_PARAMS] =
        pCitfHandle->pCalibration->module<camdev::CalibInputs>().input().config.type;
    jResponse[DEVICE_SENSOR_DRIVER_PARAMS] =
        pCitfHandle->pOperation->sensor().calibrationSensor.config.driverFileName;

    return rc;
}

RESULT CitfDevice::preview(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;

    int32_t ret = RET_SUCCESS;

    pCitfHandle->pOperation->sensor().checkValid();

    if (jRequest[DEVICE_PREVIEW_PARAMS].asBool()) {
        ret = pCitfHandle->pOperation->previewStart();
    } else {
        ret = pCitfHandle->pOperation->previewStop();
    }


    return ret;
}

RESULT CitfDevice::bitstreamId(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    uint32_t ret = pCitfHandle->pOperation->bitstreamId();
    jResponse[DEVICE_BITSTREAM_ID_PARAMS] = ret;
    return RET_SUCCESS;
}

RESULT CitfDevice::camerIcId(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    uint32_t ret = pCitfHandle->pOperation->camerIcId();
    jResponse[DEVICE_CAMER_ID_PARAMS] = ret;
    return RET_SUCCESS;
}

RESULT CitfDevice::inputConnect(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }
    return pCitfHandle->pOperation->inputConnect();
}

RESULT CitfDevice::inputDisconnect(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->inputDisconnect();
}

RESULT CitfDevice::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->reset();
}

RESULT CitfDevice::resolutionGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    uint16_t width = 0, height = 0;

    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->resolutionGet(width, height);
}

RESULT CitfDevice::resolutionSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    uint16_t width = jRequest[DEVICE_WIDTH_PARAMS].asUInt();
    uint16_t height = jRequest[DEVICE_HEIGHT_PARAMS].asUInt();
    return pCitfHandle->pOperation->resolutionSet(width,height);
}

RESULT CitfDevice::streamingStart(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    uint32_t frame = 0;
    return pCitfHandle->pOperation->streamingStart(frame);
}

RESULT CitfDevice::streamingStop(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }
    return pCitfHandle->pOperation->streamingStop();
}

RESULT CitfDevice::cameraDisconnect(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->cameraDisconnect();
}

RESULT CitfDevice::cameraReset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->cameraReset();
}

RESULT CitfDevice::previewStart(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->previewStart();
}

RESULT CitfDevice::previewStop(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    return pCitfHandle->pOperation->previewStop();
}

RESULT CitfDevice::getHalHandle(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    jResponse["hal.handle"] = (uint64_t)pCitfHandle->pHalHolder->hHal;
    return RET_SUCCESS;
}

RESULT CitfDevice::getMetadata(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    if (pCitfHandle->pOperation == nullptr) {
        TRACE(CITF_INF, " NULL pointer of pHal, exit\n", __PRETTY_FUNCTION__);
        return RET_NULL_POINTER;
    }

    int path = jRequest["path"].asUInt();
    struct isp_metadata *meta = pCitfHandle->pOperation->getMetadata(path);
    if (!meta)
        return RET_FAILURE;
    buildMetadata(meta, jResponse);
    pCitfHandle->pOperation->freeMetadata(meta, path);
    return RET_SUCCESS;
}

RESULT CitfAe::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_AE_CFG_GET    : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_CFG_SET    : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_ECM_GET    : return ecmGet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_ECM_SET    : return ecmSet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_AE_RESET      : return reset(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfAe::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAe::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeConfigGet(config);

    jResponse[AE_MODE_PARAMS] = config.mode;
    jResponse[AE_DAMP_OVER_PARAMS] = config.dampingOver;
    jResponse[AE_DAMP_UNDER_PARAMS] = config.dampingUnder;
    jResponse[AE_SET_POINT_PARAMS] = config.setPoint;
    jResponse[AE_CLM_TOLERANCE_PARAMS] = config.tolerance;

    return RET_SUCCESS;
}

RESULT CitfAe::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAe::Config config;

    config.mode = (CamEngineAecSemMode_t)jRequest[AE_MODE_PARAMS].asInt();
    config.setPoint = jRequest[AE_SET_POINT_PARAMS].asFloat();
    config.tolerance = jRequest[AE_CLM_TOLERANCE_PARAMS].asFloat();
    config.dampingOver = jRequest[AE_DAMP_OVER_PARAMS].asFloat();
    config.dampingUnder = jRequest[AE_DAMP_UNDER_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeConfigSet(config);
    return RET_SUCCESS;
}

RESULT CitfAe::ecmGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAe::Ecm ecm;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeEcmGet(ecm);

    jResponse[AE_FLICKER_PERIOD_PARAMS] = ecm.flickerPeriod;
    jResponse[AE_AFPS_PARAMS] = ecm.isAfps;

    return RET_SUCCESS;
}

RESULT CitfAe::ecmSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAe::Ecm ecm;

    ecm.flickerPeriod =
        (CamEngineFlickerPeriod_t)jRequest[AE_FLICKER_PERIOD_PARAMS].asInt();
    ecm.isAfps = jRequest[AE_AFPS_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeEcmSet(ecm);

    return RET_SUCCESS;
}

RESULT CitfAe::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeEnableGet(isEnable);

    jResponse[AE_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfAe::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[AE_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfAe::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->aeReset();

    return RET_SUCCESS;
}

RESULT CitfAwb::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_AWB_CFG_GET     : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_CFG_SET     : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_ENABLE_GET  : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_ENABLE_SET  : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_GAIN_SET    : return gainSet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_ILLUMPRO_GET: return illuminanceProfilesGet(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_RESET       : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_AWB_STATUS_GET  : return statusGet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfAwb::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAwb::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbConfigGet(config);

    jResponse[AWB_MODE_PARAMS] = config.mode;
    jResponse[AWB_INDEX_PARAMS] = config.index;
    jResponse[AWB_DAMPING_PARAMS] = config.isDamping;

    return RET_SUCCESS;
}

RESULT CitfAwb::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAwb::Config config;

    config.mode = static_cast<CamEngineAwbMode_t>(jRequest[AWB_MODE_PARAMS].asInt());
    config.index = jRequest[AWB_INDEX_PARAMS].asUInt();
    config.isDamping = jRequest[AWB_DAMPING_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfAwb::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbEnableGet(isEnable);

    jResponse[AWB_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfAwb::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->awbEnableSet(jRequest[AWB_ENABLE_PARAMS].asBool());

    return RET_SUCCESS;
}

RESULT CitfAwb::gainSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    float gain[4];
    gain[0] = jRequest[WB_RED_PARAMS].asFloat();
    gain[1] = jRequest[WB_GREEN_R_PARAMS].asFloat();
    gain[2] = jRequest[WB_GREEN_B_PARAMS].asFloat();
    gain[3] = jRequest[WB_BLUE_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbGainSet(gain);
    return RET_SUCCESS;
}

RESULT CitfAwb::illuminanceProfilesGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    std::vector<CamIlluProfile_t *> profiles;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->sensor().checkValid().illuminationProfilesGet(profiles);

    for (uint32_t i = 0; i < profiles.size(); i++) {
      CamIlluProfile_t *pProfile = profiles[i];

      Json::Value jProfile;

      jProfile[AWB_NAME_PARAMS] = pProfile->name;
      jProfile[AWB_ID_PARAMS] = pProfile->id;
      jProfile[AWB_DOOR_TYPE_PARAMS] = pProfile->DoorType;
      jProfile[AWB_AWB_TYPE_PARAMS] = pProfile->AwbType;

      Cam3x3FloatMatrix_t *pCrossTalkCoeff = &pProfile->CrossTalkCoeff;

      int32_t count = sizeof(pCrossTalkCoeff->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_AROSS_TALK_COEFF_PARAMS].append(pCrossTalkCoeff->fCoeff[i]);
      }

      Cam1x3FloatMatrix_t *pCrossTalkOffset = &pProfile->CrossTalkOffset;

      count = sizeof(pCrossTalkOffset->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_CROSS_TALK_OFFSET_PARAMS].append(pCrossTalkOffset->fCoeff[i]);
      }

      Cam1x4FloatMatrix_t *pComponentGain = &pProfile->ComponentGain;

      count = sizeof(pComponentGain->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_COMPONENT_GAIN_PARAMS].append(pComponentGain->fCoeff[i]);
      }

      Cam2x1FloatMatrix_t *pGaussMeanValue = &pProfile->GaussMeanValue;

      count = sizeof(pGaussMeanValue->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_GAUSS_MEAN_VALUE_PARAMS].append(pGaussMeanValue->fCoeff[i]);
      }

      Cam2x2FloatMatrix_t *pCovarianceMatrix = &pProfile->CovarianceMatrix;

      count = sizeof(pCovarianceMatrix->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_COVARIANCE_MATRIX_PARAMS].append(pCovarianceMatrix->fCoeff[i]);
      }

      Cam1x1FloatMatrix_t *pGaussFactor = &pProfile->GaussFactor;

      count = sizeof(pGaussFactor->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_GAUSS_FACTOR_PARAMS].append(pGaussFactor->fCoeff[i]);
      }

      Cam2x1FloatMatrix_t *pThreshold = &pProfile->Threshold;

      count = sizeof(pThreshold->fCoeff) / sizeof(float);

      for (int32_t i = 0; i < count; i++) {
          jProfile[AWB_THRESHOLD_PARAMS].append(pThreshold->fCoeff[i]);
      }

      CamSaturationCurve_t *pSaturationCurve = &pProfile->SaturationCurve;

      Json::Value jSaturationCurve;

      for (int32_t i = 0; i < pSaturationCurve->ArraySize; i++) {
          jSaturationCurve[AWB_SENSOR_GAIN_PARAMS].append(
              *(pSaturationCurve->pSensorGain + i));
          jSaturationCurve[AWB_SATURATION_PARAMS].append(
              *(pSaturationCurve->pSaturation + i));
      }

      jProfile[AWB_SATURATION_CURVE_PARAMS] = jSaturationCurve;

      CamVignettingCurve_t *pVignettingCurve = &pProfile->VignettingCurve;

      Json::Value jVignettingCurve;

      for (int32_t i = 0; i < pVignettingCurve->ArraySize; i++) {
          jVignettingCurve[AWB_SENSOR_GAIN_PARAMS].append(
              *(pVignettingCurve->pSensorGain + i));
          jVignettingCurve[AWB_VIGNETTING_PARAMS].append(
              *(pVignettingCurve->pVignetting + i));
      }

      jProfile[AWB_VIGNETTING_CURVE_PARAMS] = jVignettingCurve;

      Json::Value jCcProfileName;

      for (int32_t i = 0; i < pProfile->cc_no; i++) {
          jCcProfileName.append(*(pProfile->cc_profiles + i));
      }

      jProfile[AWB_CC_PROFILES_PARAMS] = jCcProfileName;

      Json::Value jLsc;

      for (int32_t i = 0; i < pProfile->lsc_res_no; i++) {
          jLsc[AWB_NO_PARAMS].append(pProfile->lsc_no[i]);
          jLsc[AWB_PROFILE_NAME_PARAMS].append(*(pProfile->lsc_profiles[i]));
      }

      jProfile[AWB_LSC_PARAMS] = jLsc;

      jResponse[AWB_PROFILES_PARAMS].append(jProfile);
    }

    return RET_SUCCESS;
}

RESULT CitfAwb::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbReset();

    return RET_SUCCESS;
}

RESULT CitfAwb::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAwb::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->awbStatusGet(status);

    jResponse[AWB_F_RG_PROJ_INDOOR_MIN_PARAMS] = status.rgProj.fRgProjIndoorMin;
    jResponse[AWB_F_RG_PROJ_OUTDOOR_MIN_PARAMS] = status.rgProj.fRgProjOutdoorMin;
    jResponse[AWB_F_RG_PROJ_MAX_PARAMS] = status.rgProj.fRgProjMax;
    jResponse[AWB_F_RG_PROJ_MAX_SKY_PARAMS] = status.rgProj.fRgProjMaxSky;

    return RET_SUCCESS;
}

RESULT CitfAf::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_AF_AVI_GET : return availableGet(jRequest, jResponse);
    case ISPCORE_MODULE_AF_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_AF_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_AF_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_AF_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfAf::availableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isAvailable = true;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->afAvailableGet(isAvailable);

    jResponse[AF_AVAILABEL_PARAMS] = isAvailable;

    return RET_SUCCESS;
}

RESULT CitfAf::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAf::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->afConfigGet(config);

    jResponse[AF_ALGORITHM_PARAMS] = config.searchAlgorithm;
    jResponse[AF_ONESHOT_PARAMS] = config.isOneshot;

    return RET_SUCCESS;
}

RESULT CitfAf::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAf::Config config;

    config.searchAlgorithm =
        static_cast<CamEngineAfSearchAlgorithm_t>(jRequest[AF_ALGORITHM_PARAMS].asInt());
    config.isOneshot = jRequest[AF_ONESHOT_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->afConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfAf::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->afEnableGet(isEnable);

    jResponse[AF_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfAf::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[AF_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->afEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfAvs::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_AVS_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_AVS_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_AVS_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_AVS_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_AVS_STATUS_GET : return statusGet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfAvs::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAvs::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->avsConfigGet(config);

    jResponse[AVS_USER_PARAMS] = config.isUseParams;
    jResponse[AVS_ACCELERATION_PARAMS] = config.acceleration;
    jResponse[AVS_BASE_GAIN_PARAMS] = config.baseGain;
    jResponse[AVS_FALL_OFF_PARAMS] = config.fallOff;
    jResponse[AVS_NUM_ITP_POINTS_PARAMS] = config.numItpPoints;
    jResponse[AVS_THETA_PARAMS] = config.theta;

    for (size_t i = 0; i < config.xes.size(); i++) {
        jResponse[AVS_X_PARAMS].append(config.xes[i]);
        jResponse[AVS_Y_PARAMS].append(config.ys[i]);
    }

    return RET_SUCCESS;
}

RESULT CitfAvs::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAvs::Config config;

    config.isUseParams = jRequest[AVS_USER_PARAMS].asBool();
    config.acceleration = jRequest[AVS_ACCELERATION_PARAMS].asFloat();
    config.baseGain = jRequest[AVS_BASE_GAIN_PARAMS].asFloat();
    config.fallOff = jRequest[AVS_FALL_OFF_PARAMS].asFloat();
    config.numItpPoints = jRequest[AVS_NUM_ITP_POINTS_PARAMS].asUInt();
    config.theta = jRequest[AVS_THETA_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->avsConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfAvs::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->avsEnableGet(isEnable);

    jResponse[AVS_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfAvs::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->avsEnableSet(jRequest[AVS_ENABLE_PARAMS].asBool());

    return RET_SUCCESS;
}

RESULT CitfAvs::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibAvs::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->avsStatusGet(status);

    jResponse[AVS_DISPLACEMENT_PARAMS][AVS_X_PARAMS] = status.displacement.first;
    jResponse[AVS_DISPLACEMENT_PARAMS][AVS_Y_PARAMS] = status.displacement.second;
    jResponse[AVS_OFFSET_PARAMS][AVS_X_PARAMS] = status.offset.first;
    jResponse[AVS_OFFSET_PARAMS][AVS_Y_PARAMS] = status.offset.second;

    return RET_SUCCESS;
}

RESULT CitfBls::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_BLS_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_BLS_CFG_SET : return configSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfBls::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibBls::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->blsConfigGet(config);

    jResponse[BLS_RED_PARAMS] = config.red;
    jResponse[BLS_GREEN_R_PARAMS] = config.greenR;
    jResponse[BLS_GREEN_B_PARAMS] = config.greenB;
    jResponse[BLS_BLUE_PARAMS] = config.blue;

    return RET_SUCCESS;
}

RESULT CitfBls::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibBls::Config config;

    config.red = jRequest[BLS_RED_PARAMS].asInt();
    config.greenR = jRequest[BLS_GREEN_R_PARAMS].asInt();
    config.greenB = jRequest[BLS_GREEN_B_PARAMS].asInt();
    config.blue = jRequest[BLS_BLUE_PARAMS].asInt();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->blsConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfCac::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_CAC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_CAC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfCac::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cacEnableGet(isEnable);

    jResponse[CAC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfCac::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[CAC_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cacEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfDemosaic::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_DEMOSAIC_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_DEMOSAIC_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_DEMOSAIC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_DEMOSAIC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfDemosaic::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibDemosaic::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->demosaicConfigGet(config);

    jResponse[DEMOSAIC_MODE_PARAMS] = config.mode;
    jResponse[DEMOSAIC_THRESHOLD_PARAMS] = config.threshold;

    return RET_SUCCESS;
}

RESULT CitfDemosaic::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibDemosaic::Config config;

    config.mode =
        static_cast<camdev::CalibDemosaic::Config::Mode>(jRequest[DEMOSAIC_MODE_PARAMS].asInt());
    config.threshold = jRequest[DEMOSAIC_THRESHOLD_PARAMS].asInt();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->demosaicConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfDemosaic::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->demosaicEnableGet(isEnable);

    jResponse[DEMOSAIC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfDemosaic::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[DEMOSAIC_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->demosaicEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfCnr::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_CNR_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_CNR_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_CNR_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_CNR_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfCnr::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibCnr::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cnrConfigGet(config);

    jResponse[CNR_TC1_PARAMS] = config.tc1;
    jResponse[CNR_TC2_PARAMS] = config.tc2;

    return RET_SUCCESS;
}

RESULT CitfCnr::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibCnr::Config config;

    config.tc1 = jRequest[CNR_TC1_PARAMS].asUInt();
    config.tc2 = jRequest[CNR_TC2_PARAMS].asUInt();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cnrConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfCnr::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cnrEnableGet(isEnable);

    jResponse[CNR_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfCnr::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->cnrEnableSet(jRequest[CNR_ENABLE_PARAMS].asBool());

    return RET_SUCCESS;
}

RESULT CitfCproc::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_CPROC_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_CPROC_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_CPROC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_CPROC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfCproc::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibCproc::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cprocConfigGet(config);

    jResponse[CPROC_BRIGHTNESS_PARAMS] = config.config.brightness;
    jResponse[CPROC_CHROMA_OUT_PARAMS] = config.config.ChromaOut;
    jResponse[CPROC_CONTRAST_PARAMS] = config.config.contrast;
    jResponse[CPROC_HUE_PARAMS] = config.config.hue;
    jResponse[CPROC_LUMA_IN_PARAMS] = config.config.LumaIn;
    jResponse[CPROC_LUMA_OUT_PARAMS] = config.config.LumaOut;
    jResponse[CPROC_SATURATION_PARAMS] = config.config.saturation;

    return RET_SUCCESS;
}

RESULT CitfCproc::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibCproc::Config config;

    config.config.brightness = static_cast<int8_t>(jRequest[CPROC_BRIGHTNESS_PARAMS].asInt());
    config.config.ChromaOut = static_cast<CamerIcCprocChrominaceRangeOut_t>(
        jRequest[CPROC_CHROMA_OUT_PARAMS].asInt());
    config.config.contrast = static_cast<float>(jRequest[CPROC_CONTRAST_PARAMS].asDouble());
    config.config.hue = static_cast<float>(jRequest[CPROC_HUE_PARAMS].asDouble());
    config.config.LumaIn =
        static_cast<CamerIcCprocLuminanceRangeIn_t>(jRequest[CPROC_LUMA_IN_PARAMS].asInt());
    config.config.LumaOut =
        static_cast<CamerIcCprocLuminanceRangeOut_t>(jRequest[CPROC_LUMA_OUT_PARAMS].asInt());
    config.config.saturation =
        static_cast<float>(jRequest[CPROC_SATURATION_PARAMS].asDouble());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cprocConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfCproc::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cprocEnableGet(isEnable);

    jResponse[CPROC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfCproc::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[CPROC_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->cprocEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfEe::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_EE_CFG_GET    : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_CFG_SET    : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_RESET      : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_EE_STATUS_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_TABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_EE_TABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfEe::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibEe::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeConfigGet(config);

    jResponse[EE_AUTO_PARAMS] = config.isAuto;
    binEncode(config.config, jResponse[EE_CONFIG_PARAMS]);

    return RET_SUCCESS;
}

RESULT CitfEe::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibEe::Config config;

    config.isAuto = jRequest[EE_AUTO_PARAMS].asBool();

    binDecode(jRequest[EE_CONFIG_PARAMS], config.config);

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfEe::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeEnableGet(isEnable);

    jResponse[EE_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfEe::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->eeEnableSet(jRequest[EE_ENABLE_PARAMS].asBool());

    return RET_SUCCESS;
}

RESULT CitfEe::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeReset();

    return RET_SUCCESS;
}

RESULT CitfEe::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibEe::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeStatusGet(status);

    jResponse[EE_GAIN_PARAMS] = status.gain;
    jResponse[EE_INTERGRATION_TIME_PARAMS] = status.intergrationTime;

    return RET_SUCCESS;
}

RESULT CitfEe::tableGet(Json::Value &, Json::Value &jResponse) {
  TRACE_CMD;

    Json::Value jTable;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeTableGet(jTable);

    jResponse[EE_TABLE_PARAMS] = jTable;

    return RET_SUCCESS;
}

RESULT CitfEe::tableSet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    Json::Value jTable = jRequest[EE_TABLE_PARAMS];

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->eeTableSet(jTable);

    return RET_SUCCESS;
}

RESULT CitfDpcc::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_DPCC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_DPCC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfDpcc::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpccEnableGet(isEnable);

    jResponse[DPCC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfDpcc::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[DPCC_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpccEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfDpf::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_DPF_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_DPF_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_DPF_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_DPF_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfDpf::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibDpf::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpfConfigGet(config);

    jResponse[DPF_GRADIENT_PARAMS] = config.gradient;
    jResponse[DPF_OFFSET_PARAMS] = config.offset;
    jResponse[DPF_MIN_PARAMS] = config.minimumBound;
    jResponse[DPF_DIV_PARAMS] = config.divisionFactor;
    jResponse[DPF_SIGMA_GREEN_PARAMS] = config.sigmaGreen;
    jResponse[DPF_SIGMA_RED_BLUE_PARAMS] = config.sigmaRedBlue;

    return RET_SUCCESS;
}

RESULT CitfDpf::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibDpf::Config config;

    config.gradient = jRequest[DPF_GRADIENT_PARAMS].asFloat();
    config.offset = jRequest[DPF_OFFSET_PARAMS].asFloat();
    config.minimumBound = jRequest[DPF_MIN_PARAMS].asFloat();
    config.divisionFactor = jRequest[DPF_DIV_PARAMS].asFloat();
    config.sigmaGreen = static_cast<uint8_t>(jRequest[DPF_SIGMA_GREEN_PARAMS].asUInt());
    config.sigmaRedBlue = static_cast<uint8_t>(jRequest[DPF_SIGMA_RED_BLUE_PARAMS].asUInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpfConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfDpf::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpfEnableGet(isEnable);

    jResponse[DPF_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfDpf::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[DPF_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->dpfEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfEc::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_EC_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_EC_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_EC_STATUS_GET : return statusGet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfEc::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibEc::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->sensor().checkValid().ecConfigGet(config);

    jResponse[EC_GAIN_PARAMS] = static_cast<double>(config.gain);
    jResponse[EC_GAIN_MIN_PARAMS] = static_cast<double>(config.minGain);
    jResponse[EC_GAIN_MAX_PARAMS] = static_cast<double>(config.maxGain);
    jResponse[EC_TIME_PARAMS] = static_cast<double>(config.integrationTime);

    return RET_SUCCESS;
}

RESULT CitfEc::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibEc::Config config;

    config.gain = jRequest[EC_GAIN_PARAMS].asFloat();
    config.integrationTime = jRequest[EC_TIME_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->sensor().checkValid().ecConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfEc::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibEc::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->sensor().checkValid().ecStatusGet(status);

    Json::Value &jGain = jResponse[EC_GAIN_PARAMS];

    jGain[EC_MAX_PARAMS] = status.gain.max;
    jGain[EC_MIN_PARAMS] = status.gain.min;
    jGain[EC_STEP_PARAMS] = status.gain.step;

    Json::Value &jIntegrationTime = jResponse[EC_TIME_PARAMS];

    jIntegrationTime[EC_MAX_PARAMS] = status.integrationTime.max;
    jIntegrationTime[EC_MIN_PARAMS] = status.integrationTime.min;
    jIntegrationTime[EC_STEP_PARAMS] = status.integrationTime.step;

    return RET_SUCCESS;
}

RESULT CitfFilter::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_FILTER_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_STATUS_GET : return statusGet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_TABLE_GET : return tableGet(jRequest, jResponse);
    case ISPCORE_MODULE_FILTER_TABLE_SET : return tableSet(jRequest, jResponse);

    };

    return RET_FAILURE;
}

RESULT CitfFilter::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibFilter::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterConfigGet(config);
    jResponse[FILTER_AUTO_PARAMS] = config.isAuto;
    jResponse[FILTER_DENOISE_PARAMS] = config.denoise;
    jResponse[EE_SHARPEN_PARAMS] = config.sharpen;

    return RET_SUCCESS;
}

RESULT CitfFilter::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibFilter::Config config;

    config.isAuto = jRequest[FILTER_AUTO_PARAMS].asBool();
    config.denoise = jRequest[FILTER_DENOISE_PARAMS].asInt();
    config.sharpen = jRequest[EE_SHARPEN_PARAMS].asInt();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfFilter::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterEnableGet(isEnable);

    jResponse[FILTER_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfFilter::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[FILTER_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfFilter::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  camdev::CalibFilter::Status status;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterStatusGet(status);

  jResponse[FILTER_GAIN_PARAMS] = status.gain;
  jResponse[FILTER_INTERGRATION_TIME_PARAMS] = status.intergrationTime;

  return RET_SUCCESS;
}

RESULT CitfFilter::tableGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  Json::Value jTable;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterTableGet(jTable);

  jResponse[FILTER_TABLE_PARAMS] = jTable;

  return RET_SUCCESS;
}

RESULT CitfFilter::tableSet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  Json::Value jTable = jRequest[FILTER_TABLE_PARAMS];

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->filterTableSet(jTable);

  return RET_SUCCESS;
}


RESULT CitfIe::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_IE_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_IE_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_IE_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_IE_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfIe::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibIe::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->ieConfigGet(config);

    jResponse[IE_MODE_PARAMS] = config.config.mode;
    jResponse[IE_RANGE_PARAMS] = config.config.range;

    Json::Value &jModeConfig = jResponse[IE_CONFIG_PARAMS];

    switch (config.config.mode) {
    case CAMERIC_IE_MODE_GRAYSCALE:
    case CAMERIC_IE_MODE_NEGATIVE:
      break;
    case CAMERIC_IE_MODE_SEPIA:
        jModeConfig[IE_TINT_CB_PARAMS] = config.config.ModeConfig.Sepia.TintCb;
        jModeConfig[IE_TINT_CR_PARAMS] = config.config.ModeConfig.Sepia.TintCr;
        break;
    case CAMERIC_IE_MODE_COLOR:
        jModeConfig[IE_SELECTION_PARAMS] =
            config.config.ModeConfig.ColorSelection.col_selection;
        jModeConfig[IE_THRESHOLD_PARAMS] =
            config.config.ModeConfig.ColorSelection.col_threshold;
        break;
    case CAMERIC_IE_MODE_EMBOSS:
        for (int32_t i = 0; i < 9; i++) {
              jModeConfig[IE_EMBOSS_PARAMS][IE_COEFF_PARAMS].append(
                config.config.ModeConfig.Emboss.coeff[i]);
        }
        break;
    case CAMERIC_IE_MODE_SKETCH:
        for (int32_t i = 0; i < 9; i++) {
            jModeConfig[IE_SKETCH_PARAMS][IE_COEFF_PARAMS].append(
                config.config.ModeConfig.Sketch.coeff[i]);
        }
        break;
    case CAMERIC_IE_MODE_SHARPEN:
        jModeConfig[IE_SHARPEN_PARAMS][IE_FACTOR_PARAMS] = config.config.ModeConfig.Sharpen.factor;
        jModeConfig[IE_SHARPEN_PARAMS][IE_THRESHOLD_PARAMS] =
            config.config.ModeConfig.Sharpen.threshold;

        for (int32_t i = 0; i < 9; i++) {
            jModeConfig[IE_SHARPEN_PARAMS][IE_COEFF_PARAMS].append(
                config.config.ModeConfig.Sharpen.coeff[i]);
        }
        break;
    default:
        break;
    }

    return RET_SUCCESS;
}

RESULT CitfIe::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibIe::Config config;

    config.config.mode = static_cast<CamerIcIeMode_t>(jRequest["mode"].asInt());
    config.config.range = static_cast<CamerIcIeRange_t>(jRequest["range"].asInt());

    Json::Value &jModeConfig = jRequest["config"];

    switch (config.config.mode){
    case CAMERIC_IE_MODE_GRAYSCALE:
    case CAMERIC_IE_MODE_NEGATIVE:
        break;
    case CAMERIC_IE_MODE_SEPIA:
        config.config.ModeConfig.Sepia.TintCb =
            static_cast<uint8_t>(jModeConfig["tint.cb"].asUInt());
        config.config.ModeConfig.Sepia.TintCr =
            static_cast<uint8_t>(jModeConfig["tint.cr"].asUInt());
        break;
    case CAMERIC_IE_MODE_COLOR:
        config.config.ModeConfig.ColorSelection.col_selection =
            static_cast<CamerIcIeColorSelection_t>(
                jModeConfig["selection"].asUInt());
        config.config.ModeConfig.ColorSelection.col_threshold =
            static_cast<uint8_t>(jModeConfig["threshold"].asUInt());
        break;
    case CAMERIC_IE_MODE_EMBOSS:
        for (int32_t i = 0; i < 9; i++) {
            config.config.ModeConfig.Emboss.coeff[i] =
                static_cast<int8_t>(jModeConfig["emboss"]["coeff"][i].asInt());
        }
        break;
    case CAMERIC_IE_MODE_SKETCH:
        for (int32_t i = 0; i < 9; i++) {
            config.config.ModeConfig.Sketch.coeff[i] =
                static_cast<int8_t>(jModeConfig["sketch"]["coeff"][i].asInt());
        }
        break;
    case CAMERIC_IE_MODE_SHARPEN:
        config.config.ModeConfig.Sharpen.factor =
            static_cast<uint8_t>(jModeConfig["sharpen"]["factor"].asUInt());
        config.config.ModeConfig.Sharpen.threshold =
            static_cast<uint8_t>(jModeConfig["sharpen"]["threshold"].asUInt());

        for (int32_t i = 0; i < 9; i++) {
            config.config.ModeConfig.Sharpen.coeff[i] =
                static_cast<int8_t>(jModeConfig["sharpen"]["coeff"][i].asInt());
        }
        break;
    default:
        break;
    }

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->ieConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfIe::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->ieEnableGet(isEnable);

    jResponse["enable"] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfIe::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest["enable"].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->ieEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfLsc::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_LSC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_LSC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfLsc::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->lscEnableGet(isEnable);

    jResponse[LSC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfLsc::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[LSC_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->lscEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfFileSystem::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_FILESYSTEM_REMOVE : return remove(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfFileSystem::remove(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    std::string filename = jRequest[FS_FILENAME_PARAMS].asString();

    int32_t ret = RET_SUCCESS;

    if (std::remove(filename.c_str()) != 0) {
      ret = RET_FAILURE;
    }

    jResponse[CITF_RET] = ret;

    return RET_SUCCESS;
}

RESULT CitfGc::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_GC_CURVE_GET : return curveGet(jRequest, jResponse);
    case ISPCORE_MODULE_GC_CURVE_SET : return curveSet(jRequest, jResponse);
    case ISPCORE_MODULE_GC_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_GC_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_GC_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_GC_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfGc::curveGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibGc::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcConfigGet(config);

    binEncode(config.curve, jResponse[GC_CURVE_PARAMS_BASE64]);

    return RET_SUCCESS;
}

RESULT CitfGc::curveSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibGc::Config config;

    readArrayFromNode<JH_GET_TYPE(config.curve.GammaY[0])>(jRequest, GC_CURVE_PARAMS, config.curve.GammaY);//get curve from node cannot config

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfGc::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibGc::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcConfigGet(config);
    addArray(config.curve.GammaY, jResponse, GC_CURVE_PARAMS);
    jResponse[GC_MODE_PARAMS] = config.curve.xScale;

    return RET_SUCCESS;
}

RESULT CitfGc::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibGc::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcConfigGet(config);//get the configured curve value from isp driver

    JH_READ_VAL(config.curve.xScale, jRequest, GC_MODE_PARAMS);
    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfGc::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->gcEnableGet(isEnable);

    jResponse[GC_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfGc::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->gcEnableSet(jRequest[GC_ENABLE_PARAMS].asBool());



    return RET_SUCCESS;
}

RESULT CitfHdr::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_HDR_CFG_GET    : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_HDR_CFG_SET    : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_HDR_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_HDR_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_HDR_RESET      : return reset(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfHdr::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibHdr::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->hdrConfigGet(config);

    jResponse[HDR_EXPOSURE_RATIO_PARAMS] = config.exposureRatio;
    jResponse[HDR_EXTENSION_BIT_PARAMS] = config.extensionBit;
    //jResponse[HDR_RANGE_START_VALUE_PARAMS] = config.rangeStartValue;
    //jResponse[HDR_VERY_SHORT_WEIGHT_PARAMS] = config.veryShortWeight;

    return RET_SUCCESS;
}

RESULT CitfHdr::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibHdr::Config config;

    config.exposureRatio = static_cast<float>(jRequest[HDR_EXPOSURE_RATIO_PARAMS].asFloat());
    config.extensionBit = static_cast<uint8_t>(jRequest[HDR_EXTENSION_BIT_PARAMS].asUInt());
    //config.rangeStartValue = jRequest[HDR_RANGE_START_VALUE_PARAMS].asFloat();
    //config.veryShortWeight = jRequest[HDR_VERY_SHORT_WEIGHT_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->hdrConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfHdr::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->hdrEnableGet(isEnable);

    jResponse[HDR_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfHdr::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] =
        pCitfHandle->pOperation->pEngine->hdrEnableSet(jRequest[HDR_ENABLE_PARAMS].asBool());

    return RET_SUCCESS;
}

RESULT CitfHdr::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->hdrReset();

    return RET_SUCCESS;
}

RESULT CitfReg::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_REG_DESCRIPTION : return description(jRequest, jResponse);
    case ISPCORE_MODULE_REG_GET : return get(jRequest, jResponse);
    case ISPCORE_MODULE_REG_SET : return set(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfReg::description(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    int32_t ret = RET_SUCCESS;

    CamerIcModuleId_t moduleId = (CamerIcModuleId_t)jRequest[REG_MODULE_ID_PARAMS].asInt();

    uint32_t numRegisters = 0;
    RegDescription_t *pRegDescriptions = NULL;

    ret =
        CamerIcGetRegisterDescription(moduleId, &numRegisters, &pRegDescriptions);

    jResponse[CITF_RET] = ret;

    jResponse[REG_COUNT_PARAMS] = numRegisters;

    for (uint32_t i = 0; i < numRegisters; i++) {
        Json::Value jRegister;

        RegDescription_t *pRegDescription = pRegDescriptions + i;

        jRegister[REG_ADDRESS_PARAMS] = pRegDescription->Address;

        std::string permissionDescription;

        switch (pRegDescription->Permission) {
        case PERM_READ_ONLY:
            permissionDescription = "Read Only";
            break;
        case PERM_READ_WRITE:
            permissionDescription = "Read & Write";
            break;
        case PERM_WRITE_ONLY:
            permissionDescription = "Write Only";
            break;
        case PERM_INVALID:
        default:
            permissionDescription = "Invalid";
            break;
        }

      jRegister[REG_PERMISSION_PARAMS] = permissionDescription;
      jRegister[REG_NAME_PARAMS] = pRegDescription->Name;
      jRegister[REG_HINT_PARAMS] = pRegDescription->Hint;
      jRegister[REG_RESET_VALUE_PARAMS] = pRegDescription->ResetValue;
      jRegister[REG_USED_BITS_PARAMS] = pRegDescription->UsedBits;
      jRegister[REG_WRITABLE_BITS_PARAMS] = pRegDescription->WriteAbleBits;

      jResponse[REG_REGISTERS_PARAMS].append(jRegister);
    }

    return RET_SUCCESS;
}

RESULT CitfReg::get(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    int32_t ret = RET_SUCCESS;

    uint32_t address = jRequest[REG_ADDRESS_PARAMS].asUInt();
    uint32_t value = 0;

    ret = CamerIcGetRegister(&pCitfHandle->pOperation->pEngine->hCamerIcRegCtx, address, &value);

    jResponse[CITF_RET] = ret;

    jResponse[REG_VALUE_PARAMS] = value;

    return RET_SUCCESS;
}

RESULT CitfReg::set(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    int32_t ret = RET_SUCCESS;

    uint32_t address = jRequest[REG_ADDRESS_PARAMS].asUInt();
    uint32_t value = jRequest[REG_VALUE_PARAMS].asUInt();

    ret = CamerIcSetRegister(&pCitfHandle->pOperation->pEngine->hCamerIcRegCtx,address, value);

    jResponse[CITF_RET] = ret;

    return RET_SUCCESS;
}

RESULT Citf2Dnr::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_2DNR_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_RESET      : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_STATUS_GET      : return statusGet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_TABLE_GET      : return tableGet(jRequest, jResponse);
    case ISPCORE_MODULE_2DNR_TABLE_SET      : return tableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT Citf2Dnr::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr2d::Config config;
    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dConfigGet(config, generation);

    if (generation == camdev::CalibNr2d::Nr2Dv1) {
      jResponse[NR3D_AUTO_PARAMS] = config.v1.isAuto;
      jResponse[NR2D_DENOISE_PREGAMA_STRENGTH_PARAMS] = config.v1.denoisePregamaStrength;
      jResponse[NR2D_DENOISE_STRENGTH_PARAMS] = config.v1.denoiseStrength;
      jResponse[NR2D_SIGMA_PARAMS] = config.v1.sigma;
    }

    return RET_SUCCESS;
}

RESULT Citf2Dnr::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr2d::Config config;
    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    if (generation == camdev::CalibNr2d::Nr2Dv1) {
      config.v1.isAuto = jRequest[NR3D_AUTO_PARAMS].asBool();
      config.v1.denoisePregamaStrength =
          jRequest[NR2D_DENOISE_PREGAMA_STRENGTH_PARAMS].asInt();
      config.v1.denoiseStrength = jRequest[NR2D_DENOISE_STRENGTH_PARAMS].asInt();
      config.v1.sigma = jRequest[NR2D_SIGMA_PARAMS].asDouble();
    }

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dConfigSet(config, generation);

    return RET_SUCCESS;
}

RESULT Citf2Dnr::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;
    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dEnableGet(isEnable, generation);

    jResponse[NR2D_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT Citf2Dnr::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[NR2D_ENABLE_PARAMS].asBool();
    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dEnableSet(isEnable, generation);

    return RET_SUCCESS;
}

RESULT Citf2Dnr::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dReset(generation);

    return RET_SUCCESS;
}

RESULT Citf2Dnr::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

    camdev::CalibNr2d::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dStatusGet(status, generation);

    jResponse[NR2D_GAIN_PARAMS] = status.gain;
    jResponse[NR2D_INTERGRATION_TIME_PARAMS] = status.intergrationTime;

    return RET_SUCCESS;
}

RESULT Citf2Dnr::tableGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

  Json::Value jTable;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dTableGet(jTable, generation);

  jResponse[NR2D_ENABLE_PARAMS] = jTable;

  return RET_SUCCESS;
}

RESULT Citf2Dnr::tableSet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr2d::Generation generation =
        static_cast<camdev::CalibNr2d::Generation>(jRequest[NR2D_GENERATION_PARAMS].asInt());

  Json::Value jTable = jRequest[NR2D_ENABLE_PARAMS];

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr2dTableSet(jTable, generation);

  return RET_SUCCESS;
}

RESULT Citf3Dnr::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_3DNR_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_RESET      : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_STATUS_GET : return statusGet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_TABLE_GET : return tableGet(jRequest, jResponse);
    case ISPCORE_MODULE_3DNR_TABLE_SET : return tableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT Citf3Dnr::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr3d::Config config;
    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dConfigGet(config, generation);

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
      jResponse[NR3D_AUTO_PARAMS] = config.v1.isAuto;
      jResponse[NR3D_DELTA_FACTOR_PARAMS] = config.v1.deltaFactor;
      jResponse[NR3D_MOTION_FACTOR_PARAMS] = config.v1.motionFactor;
      jResponse[NR3D_STRENGTH_PARAMS] = config.v1.strength;
    }

    return RET_SUCCESS;
}

RESULT Citf3Dnr::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr3d::Config config;
    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    if (generation == camdev::CalibNr3d::Nr3Dv1) {
      config.v1.isAuto = jRequest[NR3D_AUTO_PARAMS].asBool();
      config.v1.deltaFactor = jRequest[NR3D_DELTA_FACTOR_PARAMS].asInt();
      config.v1.motionFactor = jRequest[NR3D_MOTION_FACTOR_PARAMS].asInt();
      config.v1.strength = jRequest[NR3D_STRENGTH_PARAMS].asInt();
    }

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dConfigSet(config, generation);

    return RET_SUCCESS;
}

RESULT Citf3Dnr::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;
    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dEnableGet(isEnable, generation);

    jResponse[NR3D_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT Citf3Dnr::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[NR3D_ENABLE_PARAMS].asBool();
    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dEnableSet(isEnable, generation);

    return RET_SUCCESS;
}

RESULT Citf3Dnr::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dReset(generation);

    return RET_SUCCESS;
}

RESULT Citf3Dnr::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

    camdev::CalibNr3d::Status status;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dStatusGet(status, generation);

    jResponse[NR3D_GAIN_PARAMS] = status.gain;
    jResponse[NR3D_INTERGRATION_TIME_PARAMS] = status.intergrationTime;

    return RET_SUCCESS;
}

RESULT Citf3Dnr::tableGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

  Json::Value jTable;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dTableGet(jTable, generation);

  jResponse[NR3D_TABLE_PARAMS] = jTable;

  return RET_SUCCESS;
}

RESULT Citf3Dnr::tableSet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

    camdev::CalibNr3d::Generation generation =
        static_cast<camdev::CalibNr3d::Generation>(jRequest[NR3D_GENERATION_PARAMS].asInt());

  Json::Value jTable = jRequest[NR3D_TABLE_PARAMS];

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->nr3dTableSet(jTable, generation);

  return RET_SUCCESS;
}

RESULT CitfWdr::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_WDR_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_ENABLE_SET : return enableSet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_RESET      : return reset(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_STATUS_GET : return statusGet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_TABLE_GET : return tableGet(jRequest, jResponse);
    case ISPCORE_MODULE_WDR_TABLE_SET : return tableSet(jRequest, jResponse);

    };

    return RET_FAILURE;
}

RESULT CitfWdr::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWdr::Generation generation =
        static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

    camdev::CalibWdr::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrConfigGet(config, generation);

    if (generation == camdev::CalibWdr::Wdr1) {
        binEncode(config.wdr1.curve, jResponse[WDR_CURVE_PARAMS]);
       // for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
       //   jResponse[WDR_CURVE_PARAMS][WDR_D_Y_PARAMS].append(config.wdr1.curve.dY[i]);
       //   jResponse[WDR_CURVE_PARAMS][WDR_Y_M_PARAMS].append(config.wdr1.curve.Ym[i]);
       // }
    } else if (generation == camdev::CalibWdr::Wdr2) {
        jResponse[WDR_STRENGTH_PARAMS] = config.wdr2.strength;
    } else if (generation == camdev::CalibWdr::Wdr3) {
        jResponse[WDR_AUTO_PARAMS] = config.wdr3.isAuto;
        jResponse[WDR_STRENGTH_PARAMS] = config.wdr3.strength;
        jResponse[WDR_GAIN_MAX_PARAMS] = config.wdr3.gainMax;
        jResponse[WDR_STRENGTH_GLOBAL_PARAMS] = config.wdr3.strengthGlobal;
    }

    return RET_SUCCESS;
}

RESULT CitfWdr::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWdr::Generation generation =
        static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

    camdev::CalibWdr::Config config;

    if (generation == camdev::CalibWdr::Wdr1) {
        binDecode(jRequest[WDR_CURVE_PARAMS], config.wdr1.curve);
        /*
        CamEngineWdrCurve_t &curve = config.wdr1.curve;
        for (int32_t i = 0; i < WDR_CURVE_ARRAY_SIZE; i++) {
            curve.dY[i] = static_cast<uint8_t>(jRequest[WDR_CURVE_PARAMS][WDR_D_Y_PARAMS][i].asUInt());
            curve.Ym[i] = static_cast<uint16_t>(jRequest[WDR_CURVE_PARAMS][WDR_Y_M_PARAMS][i].asUInt());
        }
        */
    } else if (generation == camdev::CalibWdr::Wdr2) {
        config.wdr2.strength = jRequest[WDR_STRENGTH_PARAMS].asFloat();
    } else if (generation == camdev::CalibWdr::Wdr3) {
        config.wdr3.isAuto = jRequest[WDR_AUTO_PARAMS].asBool();
        config.wdr3.strength = jRequest[WDR_STRENGTH_PARAMS].asInt();
        config.wdr3.gainMax = jRequest[WDR_GAIN_MAX_PARAMS].asInt();
        config.wdr3.strengthGlobal = jRequest[WDR_STRENGTH_GLOBAL_PARAMS].asInt();
    }

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrConfigSet(config, generation);

    return RET_SUCCESS;
}

RESULT CitfWdr::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWdr::Generation generation =
        static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrEnableGet(isEnable, generation);

    jResponse[WDR_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfWdr::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWdr::Generation generation =
        static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

    bool isEnable = jRequest[WDR_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrEnableSet(isEnable, generation);

    return RET_SUCCESS;
}

RESULT CitfWdr::reset(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWdr::Generation generation =
        static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrReset(generation);

    return RET_SUCCESS;
}

RESULT CitfWdr::statusGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  camdev::CalibWdr::Generation generation =
      static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

  camdev::CalibWdr::Status status;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrStatusGet(status, generation);

  jResponse[WDR_GAIN_PARAMS] = status.gain;
  jResponse[WDR_INTERGRATION_TIME_PARAMS] = status.intergrationTime;

  return RET_SUCCESS;
}

RESULT CitfWdr::tableGet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  camdev::CalibWdr::Generation generation =
      static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

  Json::Value jTable;

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrTableGet(jTable, generation);

  jResponse[WDR_TABLE_PARAMS] = jTable;

  return RET_SUCCESS;
}

RESULT CitfWdr::tableSet(Json::Value &jRequest, Json::Value &jResponse) {
  TRACE_CMD;

  camdev::CalibWdr::Generation generation =
      static_cast<camdev::CalibWdr::Generation>(jRequest[WDR_GENERATION_PARAMS].asInt());

  Json::Value jTable = jRequest[WDR_TABLE_PARAMS];

  jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wdrTableSet(jTable, generation);

  return RET_SUCCESS;
}

RESULT CitfSimp::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_SIMP_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_SIMP_CFG_SET : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_SIMP_ENABLE_GET : return enableGet(jRequest, jResponse);
    case ISPCORE_MODULE_SIMP_ENABLE_SET : return enableSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfSimp::enableGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = false;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->simpEnableGet(isEnable);

    jResponse[SIMP_ENABLE_PARAMS] = isEnable;

    return RET_SUCCESS;
}

RESULT CitfSimp::enableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[SIMP_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->simpEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfSimp::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibSimp::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->simpConfigGet(config);

    jResponse[SIMP_FILE_PARAMS] = config.fileName;

    jResponse[SIMP_MODE_PARAMS] = config.config.Mode;

    if (config.config.Mode == CAM_ENGINE_SIMP_MODE_OVERLAY) {
      jResponse[SIMP_X_PARAMS] = config.config.SimpModeConfig.Overlay.OffsetX;
      jResponse[SIMP_Y_PARAMS] = config.config.SimpModeConfig.Overlay.OffsetY;
    } else {
      jResponse[SIMP_Y_PARAMS] = config.config.SimpModeConfig.KeyColoring.Y;
      jResponse[SIMP_CB_PARAMS] = config.config.SimpModeConfig.KeyColoring.Y;
      jResponse[SIMP_CR_PARAMS] = config.config.SimpModeConfig.KeyColoring.Cr;
    }

    return RET_SUCCESS;
}

RESULT CitfSimp::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibSimp::Config config;

    config.fileName = jRequest[SIMP_FILE_PARAMS].asString();

    config.config.Mode = static_cast<CamEngineSimpMode_t>(jRequest[SIMP_MODE_PARAMS].asInt());

    if (config.config.Mode == CAM_ENGINE_SIMP_MODE_OVERLAY) {
        config.config.SimpModeConfig.Overlay.OffsetX = jRequest[SIMP_X_PARAMS].asUInt();
        config.config.SimpModeConfig.Overlay.OffsetY = jRequest[SIMP_Y_PARAMS].asUInt();
    } else {
        config.config.SimpModeConfig.KeyColoring.Y =
            static_cast<uint8_t>(jResponse[SIMP_Y_PARAMS].asUInt());
        config.config.SimpModeConfig.KeyColoring.Y =
            static_cast<uint8_t>(jResponse[SIMP_CB_PARAMS].asUInt());
        config.config.SimpModeConfig.KeyColoring.Cr =
            static_cast<uint8_t>(jResponse[SIMP_CR_PARAMS].asUInt());
    }

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->simpConfigSet(config);

    return RET_SUCCESS;
}

RESULT CitfWb::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_WB_CFG_GET : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_WB_CFG_SET : return configSet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfWb::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWb::Config config;

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wbConfigGet(config);

    Json::Value &jMatrix = jResponse[WB_MATRIX_PARAMS];

    for (int32_t i = 0; i < 9; i++) {
        jMatrix.append(config.ccMatrix.Coeff[i]);
    }

    Json::Value &jOffset = jResponse[WB_OFFSET_PARAMS];

    jOffset[WB_RED_PARAMS] = config.ccOffset.Red;
    jOffset[WB_GREEN_PARAMS] = config.ccOffset.Green;
    jOffset[WB_BLUE_PARAMS] = config.ccOffset.Blue;

    Json::Value &jWbGains = jResponse[WB_GAINS_PARAMS];

    jWbGains[WB_RED_PARAMS] = config.wbGains.Red;
    jWbGains[WB_GREEN_R_PARAMS] = config.wbGains.GreenR;
    jWbGains[WB_GREEN_B_PARAMS] = config.wbGains.GreenB;
    jWbGains[WB_BLUE_PARAMS] = config.wbGains.Blue;

    return RET_SUCCESS;
}

RESULT CitfWb::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    camdev::CalibWb::Config config;

    Json::Value &jMatrix = jRequest[WB_MATRIX_PARAMS];

    for (int32_t i = 0; i < 9; i++) {
        config.ccMatrix.Coeff[i] = jMatrix[i].asFloat();
    }

    Json::Value &jOffset = jRequest[WB_OFFSET_PARAMS];

    config.ccOffset.Red = static_cast<int16_t>(jOffset[WB_RED_PARAMS].asInt());
    config.ccOffset.Green = static_cast<int16_t>(jOffset[WB_GREEN_PARAMS].asInt());
    config.ccOffset.Blue = static_cast<int16_t>(jOffset[WB_BLUE_PARAMS].asInt());

    Json::Value &jWbGains = jRequest[WB_GAINS_PARAMS];

    config.wbGains.Red = jWbGains[WB_RED_PARAMS].asFloat();
    config.wbGains.GreenR = jWbGains[WB_GREEN_R_PARAMS].asFloat();
    config.wbGains.GreenB = jWbGains[WB_GREEN_B_PARAMS].asFloat();
    config.wbGains.Blue = jWbGains[WB_BLUE_PARAMS].asFloat();

    jResponse[CITF_RET] = pCitfHandle->pOperation->pEngine->wbConfigSet(config);

    return RET_SUCCESS;
}
