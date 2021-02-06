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

#ifndef _VIV_V4L2_IOCTL2_H_
#define _VIV_V4L2_IOCTL2_H_

#include <sstream>
#include <string>
#include <map>

#include "MediaEvent.h"
#include "vvctrl.h"

typedef int (*funcptr)(struct v4l2_ctrl_data *pData, EventData &data);

struct vvctrl {
    funcptr begin;
    funcptr end;
    int ctrlId;
    bool supported;
};

static int sensorGetModeInit(struct v4l2_ctrl_data *pData, EventData &data) {
    Json::Reader reader;
    std::ostringstream oss;
    oss << "{\"id\":\"sensor.g.mode\"}";
    reader.parse(oss.str(), data.jsonRequest, true);

    if (!data.jsonRequest["streamid"].isObject())
        data.jsonRequest["streamid"] = 0;
    return 0;
}

static int sensorGetResWInit(struct v4l2_ctrl_data *pData, EventData &data) {
    Json::Reader reader;
    std::ostringstream oss;
    oss << "{\"id\":\"sensor.g.resw\"}";
    reader.parse(oss.str(), data.jsonRequest, true);

    if (!data.jsonRequest["streamid"].isObject())
        data.jsonRequest["streamid"] = 0;
    return 0;
}

static int sensorGetResHInit(struct v4l2_ctrl_data *pData, EventData &data) {
    Json::Reader reader;
    std::ostringstream oss;
    oss << "{\"id\":\"sensor.g.resh\"}";
    reader.parse(oss.str(), data.jsonRequest, true);

    if (!data.jsonRequest["streamid"].isObject())
        data.jsonRequest["streamid"] = 0;
    return 0;
}

static int sensorModeRetParser(struct v4l2_ctrl_data *pData, EventData &data) {
    struct v4l2_ext_control *pCtrl;
    if (!pData)
        return -1;
    pCtrl = nextof(pData, struct v4l2_ext_control *);
    pCtrl->value = data.jsonResponse["mode"].asUInt();
    pData->ret = data.jsonResponse["result"].asInt();
    return 0;
}

static int sensorResWRetParser(struct v4l2_ctrl_data *pData, EventData &data) {
    struct v4l2_ext_control *pCtrl;
    if (!pData)
        return -1;
    pCtrl = nextof(pData, struct v4l2_ext_control *);
    pCtrl->value = data.jsonResponse["resw"].asUInt();
    pData->ret = data.jsonResponse["result"].asInt();
    return 0;
}

static int sensorResHRetParser(struct v4l2_ctrl_data *pData, EventData &data) {
    struct v4l2_ext_control *pCtrl;
    if (!pData)
        return -1;
    pCtrl = nextof(pData, struct v4l2_ext_control *);
    pCtrl->value = data.jsonResponse["resh"].asUInt();
    pData->ret = data.jsonResponse["result"].asInt();
    return 0;
}

static std::map<int, struct vvctrl> v4l2_cid_map_get = {
    { V4L2_CID_VIV_SENSOR_MODE, { sensorGetModeInit, sensorModeRetParser,
            ISPCORE_MODULE_SENSOR_MODE_GET, true }},
    { V4L2_CID_VIV_SENSOR_RES_W, { sensorGetResWInit, sensorResWRetParser,
            ISPCORE_MODULE_SENSOR_RES_W_GET, true }},
    { V4L2_CID_VIV_SENSOR_RES_H, { sensorGetResHInit, sensorResHRetParser,
            ISPCORE_MODULE_SENSOR_RES_H_GET, true }},
};

static int sensorSetModeInit(struct v4l2_ctrl_data *pData, EventData &data) {
    Json::Reader reader;
    std::ostringstream oss;
    struct v4l2_ext_control *pCtrl;
    if (!pData)
        return -1;

    pCtrl = nextof(pData, struct v4l2_ext_control *);
    oss << "{\"id\":\"sensor.s.mode\",\"mode\":" << pCtrl->value << "}";
    reader.parse(oss.str(), data.jsonRequest, true);

    if (!data.jsonRequest["streamid"].isObject())
        data.jsonRequest["streamid"] = 0;
    return 0;
}

static int defRetParser(struct v4l2_ctrl_data *pData, EventData &data) {
    pData->ret = data.jsonResponse["result"].asInt();
    return 0;
}

static std::map<int, struct vvctrl> v4l2_cid_map_set = {
    { V4L2_CID_VIV_SENSOR_MODE, { sensorSetModeInit, defRetParser,
            ISPCORE_MODULE_SENSOR_MODE_SET, true }},
};

#endif  // _VIV_V4L2_IOCTL2_H_
