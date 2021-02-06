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
#include "cam_sensor.hpp"
#include "cam_common.hpp"
#include "calib_calibration.hpp"
#include "calib_features.hpp"
#include "ispcore_holder.hpp"
#include "exception.hpp"
#include "macros.hpp"
#include <dlfcn.h>
#include <isi/isi_iss.h>
#include <vector>
#include <vvsensor.h>

using namespace camdev;

SensorOps ::SensorOps (camdev::CalibSensor &calibrationSensor, HalHolder *pHalHolder_input) : calibrationSensor (calibrationSensor) {
    TRACE_IN;

    pHalHolder = pHalHolder_input;
    if (!pHalHolder) {
    //pHalHolder = new HalHolder();
        TRACE(CITF_INF, "%s Error: pah hot initialized\n", __PRETTY_FUNCTION__);
    }
    // configure sample edge on fpga
    //HalWriteSysReg(pHalHolder->hHal, 0x0004,
    //                             0x601); // TODO: this crude write does probably overwrite
    // vital HW control signal settings

    // reference marvin software HAL
    //DCT_ASSERT(!HalAddRef(pHalHolder->hHal));

    state = Init;

    TRACE_OUT;
}

SensorOps::~SensorOps() {
    TRACE_IN;

    // dereference marvin software HAL
    //DCT_ASSERT(!HalDelRef(pHalHolder->hHal));

    DCT_ASSERT(!close());

    calibDb.uninstall();

    if (pLib) {
        DCT_ASSERT(!dlclose(pLib));
    }

    pHalHolder = nullptr;
    TRACE_OUT;
}

int32_t SensorOps::capsGet(IsiSensorCaps_t &sensorCaps) {

    int32_t ret = IsiGetCapsIss(hSensor, &sensorCaps);
    REPORT(ret);

    return RET_SUCCESS;
}

SensorOps &SensorOps::checkValid() {
    if (state <= Init) {
        throw exc::LogicError(RET_WRONG_STATE, "Load sensor driver firstly");
    }

    return *this;
}

int32_t SensorOps::configGet(IsiSensorConfig_t &sensorConfig) {
    sensorConfig = config;

    return RET_SUCCESS;
}

int32_t SensorOps::configSet(IsiSensorConfig_t &sensorConfig) {
    int32_t ret = IsiSetupSensorIss(hSensor, &sensorConfig);
    REPORT(ret);

    config = sensorConfig;

    return RET_SUCCESS;
}

int32_t SensorOps::close() {
    if (hSensor) {
        int32_t ret = IsiReleaseSensorIss(hSensor);
        REPORT(ret);

        /*Reset current mode to default*/
        uint32_t mode;
        DCT_ASSERT(!HalGetSensorDefaultMode(pHalHolder->hHal, &mode));
        DCT_ASSERT(!HalSetSensorMode(pHalHolder->hHal, mode));
    }

    return RET_SUCCESS;
}

int32_t SensorOps::driverChange(std::string driverFileName, std::string calibFileName) {
    if (!fs::isExists(driverFileName)) {
        throw exc::LogicError(
                RET_INVALID_PARM,
                "Select sensor driver file and calibration file firstly");
    }

    close();

    calibDb.uninstall();

    if (pLib) {
        DCT_ASSERT(!dlclose(pLib));
    }

    calibrationSensor.config.driverFileName = driverFileName;

    DCT_ASSERT(pLib = dlopen(calibrationSensor.config.driverFileName.c_str(), RTLD_LAZY));

    DCT_ASSERT(pCamDrvConfig = (IsiCamDrvConfig_t *)dlsym(pLib, "IsiCamDrvConfig"));

    DCT_ASSERT(!pCamDrvConfig->pfIsiGetSensorIss(&pCamDrvConfig->IsiSensor));

    pSensor = &pCamDrvConfig->IsiSensor;
    pRegisterTable = pSensor->pRegisterTable;

    if (calibFileName.empty()) {
        calibrationSensor.config.calibFileName = driverFileName;
        calibrationSensor.config.calibFileName.replace(
                calibrationSensor.config.calibFileName.end() - 4,
                calibrationSensor.config.calibFileName.end(), ".xml");
    } else {
        calibrationSensor.config.calibFileName = calibFileName;
    }

    DCT_ASSERT(!calibDb.install(calibrationSensor.config.calibFileName));

    //int32_t ret = open(driverFileName);
    //REPORT(ret);

    int ret = reset();
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::ecConfigGet(camdev::CalibEc::Config &config) {
    int32_t ret = IsiGetGainIss(hSensor, &calibrationSensor.config.ec.config.gain);
    REPORT(ret);

    ret = IsiGetIntegrationTimeIss(hSensor, &calibrationSensor.config.ec.config.integrationTime);
    REPORT(ret);

    ret = IsiGetGainLimitsIss(hSensor, &calibrationSensor.config.ec.config.minGain, &calibrationSensor.config.ec.config.maxGain);
    REPORT(ret);

    config = calibrationSensor.config.ec.config;

    return RET_SUCCESS;
}

int32_t SensorOps::ecConfigSet(camdev::CalibEc::Config config) {
#if defined CTRL_HDR
  // camdev::CalibHdr &hdr = pCalibration->module<camdev::CalibHdr>();

  // float hdrRatio = static_cast<float>(hdr.config.exposureRatio);

  // float ratios[2] = {hdrRatio, hdrRatio};
    float ratios[2] = {1.2, 3.4};

    int32_t ret =
            IsiSetGainIss(hSensor, config.gain, &config.gain, ratios);
    REPORT(ret);

    ret = IsiSetIntegrationTimeIss(hSensor, config.integrationTime, &config.integrationTime, ratios);
    REPORT(ret);
#else
    int32_t ret = IsiSetGainIss(hSensor, config.gain, &config.gain);
    REPORT(ret);

    ret = IsiSetIntegrationTimeIss(hSensor, config.integrationTime, &config.integrationTime);
    REPORT(ret);
#endif

    calibrationSensor.config.ec.config = config;

    return RET_SUCCESS;
}

int32_t SensorOps::ecStatusGet(camdev::CalibEc::Status &status) {
    int32_t ret =
            IsiGetGainLimitsIss(hSensor, &status.gain.min, &status.gain.max);
    REPORT(ret);

    ret = IsiGetGainIncrementIss(hSensor, &status.gain.step);
    REPORT(ret);

    ret = IsiGetIntegrationTimeLimitsIss(hSensor, &status.integrationTime.min, &status.integrationTime.max);
    REPORT(ret);

    ret =
            IsiGetIntegrationTimeIncrementIss(hSensor, &status.integrationTime.step);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::focusGet(uint &focus) {
    int32_t ret = IsiMdiFocusGet(hSensor, &focus);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::focusSet(uint focus) {
    int32_t ret = IsiMdiFocusSet(hSensor, focus);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t
SensorOps::illuminationProfilesGet(std::vector<CamIlluProfile_t *> &profiles) {
    profiles.clear();

    CamCalibDbHandle_t calibDbHandle = calibDb.GetCalibDbHandle();
    int32_t count = 0;

    int32_t ret = CamCalibDbGetNoOfIlluminations(calibDbHandle, &count);
    REPORT(ret);

    for (int32_t i = 0; i < count; ++i) {
        CamIlluProfile_t *pIlluProfile = NULL;

        ret = CamCalibDbGetIlluminationByIdx(calibDbHandle, i, &pIlluProfile);
        DCT_ASSERT(ret == RET_SUCCESS);

        profiles.push_back(pIlluProfile);
    }

    return RET_SUCCESS;
}

bool SensorOps::isConnected() {
    int32_t ret = IsiCheckSensorConnectionIss(hSensor);

    return ret == RET_SUCCESS ? BOOL_TRUE : BOOL_FALSE;
}

bool SensorOps::isTestPattern() { return calibrationSensor.config.isTestPattern; }

int32_t SensorOps::nameGet(std::string &name) {
    if (!pCamDrvConfig) {
        return RET_NOTAVAILABLE;
    }

    name = pCamDrvConfig->IsiSensor.pszName;

    return RET_SUCCESS;
}

int32_t SensorOps::registerDescriptionGet(uint addr, IsiRegDescription_t &description) {
    const IsiRegDescription_t *ptReg = pRegisterTable;

    while (ptReg->Flags != eTableEnd) {
        if (ptReg->Addr == addr) {
            description = *ptReg;

            return RET_SUCCESS;
        }

        ++ptReg;
    }

    REPORT(RET_FAILURE);

    return RET_SUCCESS;
}

int32_t SensorOps::revisionGet(uint &revId) {
    int32_t ret = IsiGetSensorRevisionIss(hSensor, &revId);
    REPORT(ret);

    return RET_SUCCESS;
}


#ifdef SUBDEV_V4L2
#include <linux/videodev2.h>
#include <linux/media.h>
#endif

int32_t SensorOps::open() {
    HalGetSensorDrvName(pHalHolder->hHal, szDriverName, 256);
    HalGetSensorCalibXmlName(pHalHolder->hHal, szCalibFile, 256);

    driverChange(szDriverName, szCalibFile);
    int32_t ret = RET_SUCCESS;

    IsiSensorInstanceConfig_t sensorInstanceConfig;
    REFSET(sensorInstanceConfig, 0);

    // get csi config
    vvcam_mode_info_array_t array;
    DCT_ASSERT(!pCamDrvConfig->pIsiQuerySensorSupportIss(pHalHolder->hHal, &array));
    DCT_ASSERT(!HalGetSensorCurrMode(pHalHolder->hHal, &(sensorInstanceConfig.SensorModeIndex)));

    sensorInstanceConfig.HalHandle = pHalHolder->hHal;
    sensorInstanceConfig.pSensor = &pCamDrvConfig->IsiSensor;
    ret = IsiCreateSensorIss(&sensorInstanceConfig);
    REPORT(ret);

    hSensor = sensorInstanceConfig.hSensor;
    state = Idle;

    ret = IsiInitSensorIss(hSensor);
    REPORT(ret);

    struct vvcam_mode_info sensor_mode;
    ret = IsiGetSensorModeIss(hSensor,&sensor_mode);
    REPORT(ret);

    SensorHdrMode = sensor_mode.hdr_mode;
    SensorHdrStichMode = sensor_mode.stitching_mode;

    IsiGetCapsIss(hSensor,&config);

    if (config.BPat ==        BAYER_BGGR && config.BusWidth == 12) { csiFormat = V4L2_PIX_FMT_SBGGR12;
    } else if (config.BPat == BAYER_GBRG && config.BusWidth == 12) { csiFormat = V4L2_PIX_FMT_SGBRG12;
    } else if (config.BPat == BAYER_GRBG && config.BusWidth == 12) { csiFormat = V4L2_PIX_FMT_SGRBG12;
    } else if (config.BPat == BAYER_RGGB && config.BusWidth == 12) { csiFormat = V4L2_PIX_FMT_SRGGB12;

    } else if (config.BPat == BAYER_BGGR && config.BusWidth == 10) { csiFormat = V4L2_PIX_FMT_SBGGR10;
    } else if (config.BPat == BAYER_GBRG && config.BusWidth == 10) { csiFormat = V4L2_PIX_FMT_SGBRG10;
    } else if (config.BPat == BAYER_GRBG && config.BusWidth == 10) { csiFormat = V4L2_PIX_FMT_SGRBG10;
    } else if (config.BPat == BAYER_RGGB && config.BusWidth == 10) { csiFormat = V4L2_PIX_FMT_SRGGB10;
    }

    if (sensor_mode.data_compress.enable)
    {
        sensor_expand_curve_t sensor_expand_curve;

        sensor_expand_curve.x_bit = sensor_mode.data_compress.y_bit;
        sensor_expand_curve.y_bit = sensor_mode.data_compress.x_bit;
        IsiSensorGetExpandCurve(hSensor,&sensor_expand_curve);
        expand_curve.enable = 1;
        expand_curve.in_bit = sensor_expand_curve.x_bit;
        expand_curve.out_bit = sensor_expand_curve.y_bit;
        memcpy(expand_curve.px, sensor_expand_curve.expand_px,sizeof(sensor_expand_curve.expand_px));
        memcpy(expand_curve.x_data, sensor_expand_curve.expand_x_data,sizeof(sensor_expand_curve.expand_x_data));
        memcpy(expand_curve.y_data, sensor_expand_curve.expand_y_data,sizeof(sensor_expand_curve.expand_y_data));

    }else{
        expand_curve.enable = 0;
    }

    ret = IsiSetCsiConfig(hSensor,0);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::registerDump2File(std::string &filename) {
    int32_t ret = IsiDumpAllRegisters(hSensor, (const uint8_t *)filename.c_str());
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::registerRead(uint addr, uint &value) {
    int32_t ret = IsiReadRegister(hSensor, addr, &value);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::registerWrite(uint addr, uint value) {
    int32_t ret = IsiWriteRegister(hSensor, addr, value);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::reset() {
    return RET_SUCCESS;
}

int32_t SensorOps::resolutionDescriptionListGet(std::list<Resolution> &list) {
    int32_t ret = RET_SUCCESS;

    vvcam_mode_info_array_t SensorInfo;
    ret = IsiQuerySensorIss(hSensor,&SensorInfo);
    REPORT(ret);

    for (uint32_t i = 0; i < SensorInfo.count; i++)
    {
        Resolution resolution;
        char StringResolution[128];

        sprintf(StringResolution,"%dX%d",SensorInfo.modes[i].width,SensorInfo.modes[i].height);
        resolution.value = SensorInfo.modes[i].index;
        resolution.description = StringResolution;
        list.push_back(resolution);
    }

    return RET_SUCCESS;
}

int32_t SensorOps::resolutionSupportListGet(std::list<Resolution> &list) {
    int32_t ret = RET_SUCCESS;
    
    vvcam_mode_info_array_t SensorInfo;
    ret = IsiQuerySensorIss(hSensor,&SensorInfo);
    REPORT(ret);

    for (uint32_t i = 0; i < SensorInfo.count; i++)
    {
        Resolution resolution;
        char StringResolution[128];

        sprintf(StringResolution,"%dX%d",SensorInfo.modes[i].width,SensorInfo.modes[i].height);
        resolution.value = SensorInfo.modes[i].index;
        resolution.description = StringResolution;
        list.push_back(resolution);
    }

    return RET_SUCCESS;
}

int32_t SensorOps::frameRateGet(uint32_t &fps) {
    int32_t ret = RET_SUCCESS;

    ret = IsiGetSensorFpsIss(hSensor, &fps);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::frameRateSet(uint32_t fps) {
    int32_t ret = RET_SUCCESS;

    ret = IsiSetSensorFpsIss(hSensor, fps);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::modeSet(uint32_t modeIndex) {
    int32_t ret;

    ret = HalSetSensorMode(pHalHolder->hHal, modeIndex);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::modeGet(uint32_t &modeIndex) {
    int32_t ret = RET_SUCCESS;

    ret = HalGetSensorCurrMode(pHalHolder->hHal, &modeIndex);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::resolutionSet(uint16_t width, uint16_t height) {
    int32_t ret = IsiChangeSensorResolutionIss(hSensor, width, height);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::resolutionGet(uint16_t *pwidth, uint16_t *pheight) {
    int32_t ret = IsiGetResolutionIss(hSensor, pwidth, pheight);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::setup() {
    int32_t ret = IsiSetupSensorIss(hSensor, &config);
    REPORT(ret);

    bool_t isTestPattern = calibrationSensor.config.isTestPattern ? BOOL_TRUE : BOOL_FALSE;

    ret = IsiActivateTestPattern(hSensor, isTestPattern);
    REPORT(ret);

    return RET_SUCCESS;
}

int32_t SensorOps::streamEnableSet(bool isEnable) {
    int32_t ret = IsiSensorSetStreamingIss(hSensor, isEnable ? BOOL_TRUE : BOOL_FALSE);
    REPORT(ret);

    state = isEnable ? Running : Idle;

    return RET_SUCCESS;
}

int32_t SensorOps::testPatternEnableSet(bool isEnable) {
    int32_t ret = IsiActivateTestPattern(hSensor, isEnable ? BOOL_TRUE : BOOL_FALSE);
    REPORT(ret);

    calibrationSensor.config.isTestPattern = isEnable;

    return RET_SUCCESS;
}

int32_t SensorOps::bayerPatternSet(uint8_t pattern) {
    int32_t ret = IsiSetBayerPattern(hSensor, pattern);
    REPORT_WORET(ret);

    return RET_SUCCESS;
}

//NOTE: we must use static function which using before pOperation->sensor initialized.
int32_t SensorOps::queryCaps(HalHolder *pHal, vvcam_mode_info_array_t *pModeArray,
                uint32_t *pCurMode, uint32_t *pDefMode) {
    void *pLib = nullptr;
    char szDriverName[256];
    HalGetSensorDrvName(pHal->hHal, szDriverName, 256);
    HalGetSensorCurrMode(pHal->hHal, pCurMode);
    HalGetSensorDefaultMode(pHal->hHal, pDefMode);
    printf("loading [%s]...\n", szDriverName);
    DCT_ASSERT(pLib = dlopen(szDriverName, RTLD_LAZY));
    IsiCamDrvConfig_t * pCamDrvConfig;
    DCT_ASSERT(pCamDrvConfig = (IsiCamDrvConfig_t *)dlsym(pLib, "IsiCamDrvConfig"));
    DCT_ASSERT(!pCamDrvConfig->pIsiQuerySensorSupportIss(pHal->hHal, pModeArray));
    dlclose(pLib);
    return 0;
}

int32_t SensorOps::setCapsMode(HalHolder *pHal, uint32_t mode, const char* CalibXmlName)
{
    RESULT result;
    result = HalSetSensorMode(pHal->hHal, mode);
    if (CalibXmlName != NULL)
    {
        HalSetSensorCalibXmlName(pHal->hHal, CalibXmlName);
    }
    return result;
}

int32_t SensorOps::setCapsModeLock(HalHolder *pHal)
{
    RESULT result;
    result = HaSensorModeLock(pHal->hHal);
    return result;
}
