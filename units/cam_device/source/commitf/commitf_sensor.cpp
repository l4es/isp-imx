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
#include "commitf_sensor.hpp"
#include "cam_mapcaps.hpp"
#include "cam_common.hpp"
#include <isi/isi.h>
#include <isi/isi_iss.h>
#include <vvsensor.h>

using namespace camdev;

RESULT CitfSensor::process(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {

    if (ctrlId == ISPCORE_MODULE_SENSOR_QUERY)
        return queryCaps(jRequest, jResponse);
    else if (ctrlId == ISPCORE_MODULE_SENSOR_MODE_SET)
        return setCapsMode(jRequest, jResponse);
    else if(ctrlId == ISPCORE_MODULE_SENSOR_MODE_LOCK)
        return setCapsModeLock(jRequest, jResponse);

    if (!pCitfHandle || !pCitfHandle->pOperation)
        return RET_NULL_POINTER;
    switch (ctrlId) {
    case ISPCORE_MODULE_SENSOR_CAPS                : return caps(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_CFG_GET             : return configGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_CFG_SET             : return configSet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_NAME_GET            : return nameGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REVISION_GET        : return revisionGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_OPEN                : return sensorOpen(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_IS_CONNECTED        : return isConnected(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_IS_TEST_PATTERN     : return isTestPattern(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_DRV_CHANGE          : return driverChange(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_DRV_LIST            : return driverList(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_INFO                : return info(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_RESOLUTION_LIST_GET : return resolutionListGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_GET_RESOLUTION_NAME : return isiGetResolutionName(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REG_DESCRIPTION     : return registerDescription(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REG_DUMP2FILE       : return registerDump2File(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REG_GET             : return registerGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REG_SET             : return registerSet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_REG_TABLE           : return registerTable(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_RESOLUTION_SET      : return resolutionSetSensor(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_RESOLUTION_SUP_GET  : return resolutionSupportListGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_FPS_GET             : return frameRateGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_FPS_SET             : return frameRateSet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_MODE_GET            : return modeGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_RES_W_GET           : return resWGet(jRequest, jResponse);
    case ISPCORE_MODULE_SENSOR_RES_H_GET           : return resHGet(jRequest, jResponse);
    };

    return RET_FAILURE;
}

RESULT CitfSensor::caps(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    IsiSensorCaps_t caps;
    REFSET(caps, 0);

    jResponse[CITF_RET] = sensor().capsGet(caps);

    binEncode(caps, jResponse[SENSOR_CAPS_PARAMS_BASE64]);
    return RET_SUCCESS;
}

RESULT CitfSensor::configGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    IsiSensorConfig_t sensorConfig;
    REFSET(sensorConfig, 0);

    jResponse[CITF_RET] = sensor().configGet(sensorConfig);

    binEncode(sensorConfig, jResponse[SENSOR_CAPS_PARAMS_BASE64]);
    return RET_SUCCESS;
}

RESULT CitfSensor::configSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    IsiSensorConfig_t sensorConfig;
    REFSET(sensorConfig, 0);

    binDecode(jRequest[SENSOR_CONFIG_PARAMS_BASE64], sensorConfig);

    jResponse[CITF_RET] = sensor().configSet(sensorConfig);

    return RET_SUCCESS;
}

RESULT CitfSensor::nameGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    std::string name;
    sensor().checkValid();

    jResponse[CITF_RET] = sensor().nameGet(name);
    jResponse[SENSOR_NAME_GET] = name.c_str();

    return RET_SUCCESS;
}

RESULT CitfSensor::revisionGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t revId = jRequest[SENSOR_SENSOR_ID_PARAMS].asUInt();

    jResponse[CITF_RET] = sensor().revisionGet(revId);
    jResponse[SENSOR_SENSOR_ID_PARAMS] = revId;

    return RET_SUCCESS;
}

RESULT CitfSensor::sensorOpen(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    return sensor().open();
}

RESULT CitfSensor::isConnected(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();
    bool isConnected = sensor().isConnected();

    jResponse[CITF_RET] = RET_SUCCESS;
    jResponse[SENSOR_CONNECTION_PARAMS] = isConnected;

    return RET_SUCCESS;
}

RESULT CitfSensor::isTestPattern(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();
    bool isTestPattern = sensor().isTestPattern();

    jResponse[CITF_RET] = RET_SUCCESS;
    jResponse[SENSOR_TEST_PATTERN_PARAMS] = isTestPattern;

    return RET_SUCCESS;
}



RESULT CitfSensor::driverChange(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    CamStatus::State deviceStateBackup = pCitfHandle->pOperation->state;

    if (deviceStateBackup >= CamStatus::Running) {
        pCitfHandle->pOperation->previewStop();
    }

    if (deviceStateBackup >= CamStatus::Idle) {
        pCitfHandle->pOperation->cameraDisconnect();
    }

    jResponse[CITF_RET] = sensor().driverChange(jRequest[SENSOR_DRIVER_FILE_PARAMS].asString(),  jRequest[SENSOR_CALIB_FILE_PARAMS].asString());

    if (jResponse[CITF_RET].asInt() != RET_SUCCESS) {
        return RET_SUCCESS;
    }

    pCitfHandle->pCalibration->module<camdev::CalibInputs>().input().config.type = camdev::CalibInput::Sensor;

    // pCitfHandle->pOperation->cameraConnect(true);

    if (deviceStateBackup >= CamStatus::Running) {
        pCitfHandle->pOperation->previewStart();
    }

    return RET_SUCCESS;
}

RESULT CitfSensor::driverList(Json::Value &jRequest,  Json::Value &jResponse) {
    TRACE_CMD;

    return RET_SUCCESS;
}

RESULT CitfSensor::info(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    jResponse[CITF_RET] = RET_SUCCESS;

    std::list<camdev::SensorOps::Resolution> resolutionList;

    int32_t ret = sensor().resolutionDescriptionListGet(resolutionList);
    if (ret == RET_SUCCESS) {
        Json::Value jResolutionList = Json::Value(Json::arrayValue);
        for (const camdev::SensorOps::Resolution &resolution : resolutionList) {
            Json::Value jResolution;

            jResolution[SENSOR_VALUE_PARAMS] = resolution.value;
            jResolution[SENSOR_DESCRIPTION_PARAMS] = resolution.description;

            jResolutionList.append(jResolution);
        }

        jResponse[SENSOR_RESOLUTION_LIST_PARAMS] = jResolutionList;
    }

    jResponse[SENSOR_SENSOR_DRIVER_PARAMS] = sensor().calibrationSensor.config.driverFileName;
    jResponse[SENSOR_CALIBRATION_DATA_PARAMS] = sensor().calibrationSensor.config.calibFileName;
    jResponse[SENSOR_SENSOR_NAME_PARAMS] = sensor().pSensor->pszName;
    jResponse[SENSOR_SENSOR_STATE_PARAMS] = sensor().state;


    uint32_t revId = 0;

    sensor().revisionGet(revId);
    jResponse[SENSOR_SENSOR_ID_PARAMS] = revId;

    jResponse[SENSOR_CONNECTION_PARAMS] = sensor().isConnected();

    jResponse[SENSOR_TEST_PATTERN_PARAMS] = sensor().isTestPattern();

    jResponse[SENSOR_BAYER_PATTERN_PARAMS] =
            isiCapDescription<CamerIcIspBayerPattern_t>(sensor().config.BPat);
    jResponse[SENSOR_BUS_WIDTH_PARAMS] = isiCapDescription<CamerIcIspInputSelection_t>(
            sensor().config.BusWidth);
    jResponse[SENSOR_MIPI_MODE_PARAMS] =
            isiCapDescription<MipiDataType_t>(sensor().config.MipiMode);

    return RET_SUCCESS;
}


RESULT CitfSensor::resolutionListGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    sensor().checkValid();

    std::list<camdev::SensorOps::Resolution> resolutionList;

    int32_t ret = sensor().resolutionDescriptionListGet(resolutionList);
    if (ret == RET_SUCCESS) {
        Json::Value jResolutionList = Json::Value(Json::arrayValue);
        for (const camdev::SensorOps::Resolution &resolution : resolutionList) {
            Json::Value jResolution;

            jResolution[SENSOR_VALUE_PARAMS] = resolution.value;
            jResolution[SENSOR_DESCRIPTION_PARAMS] = resolution.description;

            jResolutionList.append(jResolution);
        }

        jResponse[SENSOR_RESOLUTION_LIST_PARAMS] = jResolutionList;
    }

    return ret;
}

RESULT CitfSensor::isiGetResolutionName(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    return RET_SUCCESS;
}

RESULT CitfSensor::registerDescription(Json::Value &jRequest,   Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t address = jRequest[SENSOR_ADDRESS_PARAMS].asUInt();

    IsiRegDescription_t regDescription;
    REFSET(regDescription, 0);

    int32_t ret = sensor().registerDescriptionGet(address, regDescription);
    jResponse[CITF_RET] = ret;

    if (ret != RET_SUCCESS) {
        return RET_SUCCESS;
    }

    Json::Value jDescription;

    jDescription[SENSOR_ADDRESS_PARAMS] = regDescription.Addr;
    jDescription[SENSOR_DEFAULT_VALUE_PARAMS] = regDescription.DefaultValue;
    jDescription[SENSOR_NAME_PARAMS] = regDescription.pName;
    jDescription[SENSOR_FLAGS_PARAMS] = regDescription.Flags;

    jResponse[SENSOR_DESCRIPTION_PARAMS] = jDescription;

    return RET_SUCCESS;
}

RESULT CitfSensor::registerDump2File(Json::Value &jRequest,  Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    std::string filename = jRequest[SENSOR_FILENAME_PARAMS].asString();
    jResponse[CITF_RET] = sensor().registerDump2File(filename);

    return RET_SUCCESS;
}

RESULT CitfSensor::registerGet(Json::Value &jRequest,  Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t address = jRequest[SENSOR_ADDRESS_PARAMS].asUInt();
    uint32_t value = 0;

    jResponse[CITF_RET] = sensor().registerRead(address, value);

    jResponse[SENSOR_VALUE_PARAMS] = value;

    return RET_SUCCESS;
}

RESULT CitfSensor::registerSet(Json::Value &jRequest,  Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t address = jRequest[SENSOR_ADDRESS_PARAMS].asUInt();
    uint32_t value = jRequest[SENSOR_VALUE_PARAMS].asUInt();

    jResponse[CITF_RET] = sensor().registerWrite(address, value);

    return RET_SUCCESS;
}

RESULT CitfSensor::registerTable(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();
    const IsiRegDescription_t *pRegDescription = sensor().pSensor->pRegisterTable;
    jResponse[CITF_RET] = RET_SUCCESS;
    Json::Value jRegisters = Json::Value(Json::arrayValue);

    while (pRegDescription->Flags) {
        Json::Value jRegister;

        jRegister[SENSOR_ADDRESS_PARAMS] = pRegDescription->Addr;
        jRegister[SENSOR_DEFAULT_VALUE_PARAMS] = pRegDescription->DefaultValue;
        jRegister[SENSOR_NAME_PARAMS] = pRegDescription->pName;
        jRegister[SENSOR_FLAGS_PARAMS] = pRegDescription->Flags;

        jRegisters.append(jRegister);

        pRegDescription++;
    }

    jResponse[SENSOR_REGISTERS_PARAMS] = jRegisters;
    return RET_SUCCESS;
}

RESULT CitfSensor::resolutionSetSensor(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;
    sensor().checkValid();
    CamStatus::State oldState = pCitfHandle->pOperation->state;

    if (oldState == CamStatus::Running) {
        pCitfHandle->pOperation->streamingStop();
    }
    uint16_t width = jRequest[DEVICE_WIDTH_PARAMS].asUInt();
    uint16_t height = jRequest[DEVICE_HEIGHT_PARAMS].asUInt();
    jResponse[CITF_RET] = pCitfHandle->pOperation->resolutionSet(width,height);

    if (oldState == CamStatus::Running) {
        pCitfHandle->pOperation->streamingStart();
    }

    return RET_SUCCESS;
}

RESULT CitfSensor::testPatternEnableSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    bool isEnable = jRequest[SENSOR_ENABLE_PARAMS].asBool();

    jResponse[CITF_RET] = sensor().checkValid().testPatternEnableSet(isEnable);

    return RET_SUCCESS;
}

RESULT CitfSensor::resolutionSupportListGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    int32_t ret = RET_SUCCESS;

    sensor().checkValid();

    std::list<camdev::SensorOps::Resolution> resolutionList;

    ret = sensor().resolutionSupportListGet(resolutionList);
    if (ret == RET_SUCCESS) {
      Json::Value jResolutionList = Json::Value(Json::arrayValue);
      for (const camdev::SensorOps::Resolution &resolution : resolutionList) {
          Json::Value jResolution;

          jResolution[SENSOR_VALUE_PARAMS] = resolution.value;
          jResolution[SENSOR_DESCRIPTION_PARAMS] = resolution.description;

          jResolutionList.append(jResolution);
      }

      jResponse[SENSOR_RESOLUTION_LIST_PARAMS] = jResolutionList;
    }

    jResponse[CITF_RET] = ret;

    return RET_SUCCESS;
}

RESULT CitfSensor::frameRateGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t fps = 0;

    jResponse[CITF_RET] = sensor().frameRateGet(fps);
    jResponse[SENSOR_FPS_PARAMS] = fps;

    return RET_SUCCESS;
}

RESULT CitfSensor::frameRateSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t fps = jRequest[SENSOR_FPS_PARAMS].asUInt();

    jResponse[CITF_RET] = sensor().frameRateSet(fps);

    return RET_SUCCESS;
}

RESULT CitfSensor::modeGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t mode = 0;

    jResponse[CITF_RET] = sensor().modeGet(mode);
    jResponse[SENSOR_MODE] = mode;

    return RET_SUCCESS;
}

RESULT CitfSensor::modeSet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    uint32_t mode = jRequest[SENSOR_MODE].asUInt();

    jResponse[CITF_RET] = sensor().modeSet(mode);

    return RET_SUCCESS;
}

RESULT CitfSensor::resWGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    int32_t ret;
    uint16_t width,height;
    ret = sensor().resolutionGet(&width,&height);
    if (!ret) {
        jResponse[SENSOR_RES_W] = width;
    } else {
        jResponse[SENSOR_RES_W] = 0;
    }
    jResponse[CITF_RET] = ret;

    return RET_SUCCESS;
}

RESULT CitfSensor::resHGet(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE_CMD;

    sensor().checkValid();

    int32_t ret;
    uint16_t width,height;
    ret = sensor().resolutionGet(&width,&height);
    if (!ret) {
        jResponse[SENSOR_RES_H] = height;
    } else {
        jResponse[SENSOR_RES_H] = 0;
    }
    jResponse[CITF_RET] = ret;

    return RET_SUCCESS;
}

RESULT CitfSensor::queryCaps(Json::Value &jRequest, Json::Value &jResponse) {
    struct vvcam_mode_info_array array;
    uint32_t curMode, defMode;
    SensorOps::queryCaps(pCitfHandle->pHalHolder, &array, &curMode, &defMode);
    jResponse["current"] = curMode;
    jResponse["default"] = defMode;
    for (uint32_t i = 0; i < array.count; i++) {
        jResponse["caps"][i]["index"]             = array.modes[i].index;
        jResponse["caps"][i]["width"]             = array.modes[i].width;
        jResponse["caps"][i]["height"]            = array.modes[i].height;
        jResponse["caps"][i]["fps"]               = array.modes[i].fps;
        jResponse["caps"][i]["hdr_mode"]          = array.modes[i].hdr_mode;
        jResponse["caps"][i]["bit_width"]         = array.modes[i].bit_width;
        jResponse["caps"][i]["bayer_pattern"]     = array.modes[i].bayer_pattern;
        jResponse["caps"][i]["stitching_mode"]    = array.modes[i].stitching_mode;
    }
    return RET_SUCCESS;
}

RESULT CitfSensor::setCapsMode(Json::Value &jRequest, Json::Value &jResponse)
{
    RESULT result = RET_SUCCESS;
    uint32_t mode = jRequest["mode"].asUInt();
    std::string CalibXmlName = jRequest["CalibXmlName"].asString();

    if (CalibXmlName.empty() || strstr(CalibXmlName.c_str(),".xml") == NULL)
    {
        result = SensorOps::setCapsMode(pCitfHandle->pHalHolder,mode,NULL);
    }else
    {
        result = SensorOps::setCapsMode(pCitfHandle->pHalHolder,mode,CalibXmlName.c_str());
    }
    
    
    jResponse["result"] = result;
    return result;
}

RESULT CitfSensor::setCapsModeLock(Json::Value &jRequest, Json::Value &jResponse)
{
    RESULT result;
    result = SensorOps::setCapsModeLock(pCitfHandle->pHalHolder);
    return result;
}
