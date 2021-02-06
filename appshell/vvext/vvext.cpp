/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <memory.h>
#include "math.h"

#include <inttypes.h>

#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <functional>

#include <EAutoLock.h>
#include <cam_device_api.hpp>
#include <json_helper.h>

#include "viv_video_kevent.h"
#include "ioctl_cmds.h"

#include "log.h"

#define LOGTAG "VVEXT:"

ESignal sig;
int fd;
int streamid = 0;
std::string strError;

class Splitter {
public:
    static std::vector<std::string> exec(const std::string &str, const std::string &pattern);
    static void exec(const std::string &str, const std::string &pattern, std::vector<uint32_t>& ret);
    static void exec(const std::string &str, const std::string &pattern, std::vector<int32_t>& ret);
    static void exec(const std::string &str, const std::string &pattern, std::vector<int64_t>& ret);
    static void exec(const std::string &str, const std::string &pattern, std::vector<float>& ret);
};

std::vector<std::string> Splitter::exec(const std::string &str, const std::string &pattern) {
    std::vector<std::string> resultVec;
    char* tmpStr = strtok((char*)str.c_str(), pattern.c_str());

    while (tmpStr != NULL) {
        resultVec.push_back(std::string(tmpStr));
        tmpStr = strtok(NULL, pattern.c_str());
    }

    return resultVec;
}

void Splitter::exec(const std::string &str,const std::string &pattern, std::vector<int32_t>& ret) {
    std::vector<std::string> stringArray = exec(str, pattern);
    ret.clear();
    for (const auto &elem : stringArray) {
        // autodetect 0x hex notation
        ret.push_back(stoi(elem, 0, 0));
    }
}

void Splitter::exec(const std::string &str,const std::string &pattern, std::vector<uint32_t>& ret) {
    std::vector<std::string> stringArray = exec(str, pattern);
    ret.clear();
    for (const auto &elem : stringArray) {
        // autodetect 0x hex notation
        ret.push_back(stoul(elem, 0, 0));
    }
}


void Splitter::exec(const std::string &str, const std::string &pattern, std::vector<int64_t>& ret) {
    std::vector<std::string> stringArray = exec(str, pattern);
    ret.clear();
    for (const auto &elem : stringArray) {
        // autodetect 0x hex notation
        ret.push_back(stoll(elem, 0, 0));
    }
}

void Splitter::exec(const std::string &str,const std::string &pattern, std::vector<float>& ret) {
    std::vector<std::string> stringArray = exec(str, pattern);;
    ret.clear();
    for (const auto &elem : stringArray) {
        ret.push_back(stof(elem));
    }
}
#define VIV_CUSTOM_CID_BASE (V4L2_CID_USER_BASE | 0xf000)
#define V4L2_CID_VIV_EXTCTRL (VIV_CUSTOM_CID_BASE + 1)

int viv_private_ioctl(const char *cmd, Json::Value& jsonRequest, Json::Value& jsonResponse) {
    if (!cmd) {
        ALOGE("cmd should not be null!");
        return -1;
    }
    jsonRequest["id"] = cmd;
    jsonRequest["streamid"] = streamid;

    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.string = new char[VIV_JSON_BUFFER_SIZE];
    ec.id = V4L2_CID_VIV_EXTCTRL;
    ec.size = 0;
    ecs.controls = &ec;
    ecs.count = 1;

    ::ioctl(fd, VIDIOC_G_EXT_CTRLS, &ecs);

    strcpy(ec.string, jsonRequest.toStyledString().c_str());

    int ret = ::ioctl(fd, VIDIOC_S_EXT_CTRLS, &ecs);
    if (ret != 0)
        return ret;

    ::ioctl(fd, VIDIOC_G_EXT_CTRLS, &ecs);

    Json::Reader reader;
    reader.parse(ec.string, jsonResponse, true);
    delete ec.string;
    ec.string = NULL;
    return jsonResponse["MC_RET"].asInt();
}

/*************************BASE**************************/
struct IBasePage {
    IBasePage(IBasePage* page, const char* szName);
    virtual ~IBasePage();

    void insert(IBasePage* subPage);
    virtual void dump();
    virtual IBasePage* search(const char* szName);
    virtual void process(std::string& str);
    virtual void onSelected() {}
    virtual const char* getClassName() { return mName.c_str(); }
    void navigateBack();

    std::string mName;
    std::vector<IBasePage*> pages;
    IBasePage* parent = NULL;
    bool isPage = true;
};

IBasePage* pCurrentPage;


IBasePage::IBasePage(IBasePage* page, const char* szName) { 
    parent = page;
    mName = szName;
}

IBasePage::~IBasePage() { 
    for (auto item : pages)
        delete item;
}


void IBasePage::insert(IBasePage* subPage) {
    pages.push_back(subPage);
}

void IBasePage::dump() {
    printf("%s: select and press enter\n", getClassName());
    if (pCurrentPage == this) {
        for (size_t i = 0; i < pages.size(); i++)
            printf("  %ld. %s\n", i, pages[i]->getClassName());
        printf("\n");
        if (parent)
            ALOGI("press enter to navigate back");
    } else {
        for (size_t i = 0; i < pages.size(); i++)
            pages[i]->dump();
    }
}

IBasePage* IBasePage::search(const char* szName) {
    if (strcmp(getClassName(), szName) == 0)
        return this;
    else {
        for (size_t i = 0; i < pages.size(); i++) {
            IBasePage* pSearchedPage = pages[i]->search(szName);
            if (pSearchedPage)
                return pSearchedPage;
        }
    }
    return NULL;
}

void IBasePage::navigateBack() {
    if (parent) {
        pCurrentPage = parent;
    }
}

void IBasePage::process(std::string& str) {
    if (pCurrentPage == this) {
        int id = atoi(str.c_str());
        if (id >= 0 && id < (int)pages.size() ) {
            if (pages[id]->isPage)
                pCurrentPage = pages[id];
            else
                pages[id]->onSelected();
        } else {
            strError = "input error.";
        }
    }
}

struct BasePage : IBasePage {
    using IBasePage::IBasePage;
};

struct BaseOption : IBasePage {
    using IBasePage::IBasePage;
    BaseOption(IBasePage* page, const char* szName) : IBasePage(page, szName) {
        isPage = false;
    }
    virtual void onSelected() {}
    bool enable = true;
};


/*************************AWB**************************/
struct OptionAwbEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "awb enabled" : "awb disabled";
        Json::Value jRequest, jResponse;
        jRequest[AWB_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_AWB_S_EN, jRequest, jResponse);
    }
};

struct OptionAwbReset : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        jRequest[AWB_RESET_PARAMS] = enable;
        viv_private_ioctl(IF_AWB_RESET, jRequest, jResponse);
    }
};

struct PageAwbAutoCtrl : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input params : [mode, profile-index, isDamping](1-manual/2-auto, index:0~4, 0/1)\n:");
    }
    void process(std::string& str) {
        std::vector<float> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 3) {
            strError = ("input error [mode, profile-index, isDamping].");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[AWB_MODE_PARAMS] = params[0];
            jRequest[AWB_INDEX_PARAMS] = params[1];
            jRequest[AWB_DAMPING_PARAMS] = params[2];
            viv_private_ioctl(IF_AWB_S_CFG, jRequest, jResponse);
        }
    }
};

struct OptionAwbGainR : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        jRequest[WB_RED_PARAMS] = 3.0f;
        jRequest[WB_GREEN_R_PARAMS] = 1.0f;
        jRequest[WB_GREEN_B_PARAMS] = 1.0f;
        jRequest[WB_BLUE_PARAMS] = 1.0f;
        viv_private_ioctl(IF_AWB_S_GAIN, jRequest, jResponse);
    }
};

struct OptionAwbGainB : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        jRequest[WB_RED_PARAMS] = 1.0f;
        jRequest[WB_GREEN_R_PARAMS] = 1.0f;
        jRequest[WB_GREEN_B_PARAMS] = 1.0f;
        jRequest[WB_BLUE_PARAMS] = 3.0f;
        viv_private_ioctl(IF_AWB_S_GAIN, jRequest, jResponse);
    }
};

struct PageAwbGain : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input params : [r, gr, gb, b](0.2 - 3.9)\n:");
    }
    void process(std::string& str) {
        std::vector<float> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 4) {
            strError = ("input error [r, gb, gr, b].");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[WB_RED_PARAMS] = std::min(3.9f, std::max(0.2f, params[0]));
            jRequest[WB_GREEN_R_PARAMS] = std::min(3.9f, std::max(0.2f, params[1]));
            jRequest[WB_GREEN_B_PARAMS] = std::min(3.9f, std::max(0.2f, params[2]));
            jRequest[WB_BLUE_PARAMS] =  std::min(3.9f, std::max(0.2f, params[3]));
            viv_private_ioctl(IF_AWB_S_GAIN, jRequest, jResponse);
        }
    }
};

struct PageAwb : BasePage {
    using BasePage::BasePage;
    PageAwb(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionAwbEnable(this, "AWB On/Off"));
        insert(new OptionAwbReset(this, "AWB Reset"));
        insert(new PageAwbAutoCtrl(this, "AWB Auto Ctrl item"));
        insert(new OptionAwbGainR(this, "GAIN RED to 3") );
        insert(new OptionAwbGainB(this, "GAIN BLUE to 3"));
        insert(new PageAwbGain(this, "GAIN INPUT"));
    }
};

struct PageFps : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input new fps: \n:");
    }
    void process(std::string &str) {
        int i = atoi(str.c_str());
        if (i >= 1 ) {
            Json::Value jRequest, jResponse;
            jRequest[DEVICE_FPS_PARAMS] = i;
            viv_private_ioctl(IF_S_FPS, jRequest, jResponse);
        } else {
            strError = ("input error fps.");
        }
    }
};

struct PageAecSetPoint : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input new setpoint: (1 float number)\n:");
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_AE_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[AE_SET_POINT_PARAMS] = atof(str.c_str());
        viv_private_ioctl(IF_AE_S_CFG, jRequest, jResponse);
    }
};

struct PageAecDampOver : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input new dampOver: (1 float number)\n:");
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_AE_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[AE_DAMP_OVER_PARAMS] = atof(str.c_str());
        viv_private_ioctl(IF_AE_S_CFG, jRequest, jResponse);
    }
};

struct PageAecDampUnder : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input new dampUnder: (1 float number)\n:");
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_AE_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[AE_DAMP_UNDER_PARAMS] = atof(str.c_str());
        viv_private_ioctl(IF_AE_S_CFG, jRequest, jResponse);
    }
};

struct PageAecTolerance : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input new clmTolerance: (1 float number)\n:");
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_AE_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[AE_CLM_TOLERANCE_PARAMS] = atof(str.c_str());
        viv_private_ioctl(IF_AE_S_CFG, jRequest, jResponse);
    }
};

struct OptionAecEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "aec enabled" : "aec disabled";
        Json::Value jRequest, jResponse;
        jRequest[AE_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_AE_S_EN, jRequest, jResponse);
    }
};

struct PageAecGain : BasePage {
    using BasePage::BasePage;
    void dump() {
        // fetch current value
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_EC_G_CFG, jRequest, jResponse);
        double currentGain = jResponse[EC_GAIN_PARAMS].asDouble();
        double minGain = jResponse[EC_GAIN_MIN_PARAMS].asDouble();
        double maxGain = jResponse[EC_GAIN_MAX_PARAMS].asDouble();
        printf("please input new gain value: (1 float number %f~%f)\n", minGain, maxGain);
        printf("Current Gain: %f\n:", currentGain);
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_EC_G_CFG, jRequest, jResponse);
        double currentGain = jResponse[EC_GAIN_PARAMS].asDouble();
        double currentExposure = jResponse[EC_TIME_PARAMS].asDouble();
        double minGain = jResponse[EC_GAIN_MIN_PARAMS].asDouble();
        double maxGain = jResponse[EC_GAIN_MAX_PARAMS].asDouble();

        jRequest[EC_GAIN_PARAMS] = str.empty()?currentGain:std::stod(str);
        if(jRequest[EC_GAIN_PARAMS].asDouble() < minGain)
        {
            jRequest[EC_GAIN_PARAMS] = minGain;
        }
        else if(jRequest[EC_GAIN_PARAMS].asDouble() > maxGain)
        {
            jRequest[EC_GAIN_PARAMS] = maxGain;
        }
        jRequest[EC_TIME_PARAMS] = currentExposure;
        viv_private_ioctl(IF_EC_S_CFG, jRequest, jResponse);
    }
};

struct PageAecExposureTime : BasePage {
    using BasePage::BasePage;
    void dump() {
        // fetch current value
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_EC_G_CFG, jRequest, jResponse);
        double currentExposure = jResponse[EC_TIME_PARAMS].asDouble();
        printf("please input new exposure time value: (1 float number) %f\n:",currentExposure);
    }
    void process(std::string &str) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_EC_G_CFG, jRequest, jResponse);
        double currentGain = jResponse[EC_GAIN_PARAMS].asDouble();
        double currentExposure = jResponse[EC_TIME_PARAMS].asDouble();
        jRequest[EC_GAIN_PARAMS] = currentGain;
        jRequest[EC_TIME_PARAMS] = str.empty()?currentExposure:std::stod(str);
        viv_private_ioctl(IF_EC_S_CFG, jRequest, jResponse);
    }
};

struct OptionAecReset : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        jRequest[AE_RESET_PARAMS] = enable;
        viv_private_ioctl(IF_AE_RESET, jRequest, jResponse);
    }
};

struct PageAec : BasePage {
    using BasePage::BasePage;
    PageAec(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionAecEnable(this, "AEC On/Off"));
        insert(new OptionAecReset(this, "AEC Reset"));
        insert(new PageAecSetPoint(this, "AEC SetPoint"));
        insert(new PageAecDampOver(this, "AEC DampOver"));
        insert(new PageAecDampUnder(this, "AEC DampUnder"));
        insert(new PageAecTolerance(this, "AEC Tolerance"));
        insert(new PageAecGain(this, "AEC Gain"));
        insert(new PageAecExposureTime(this, "AEC ExposureTime"));
    }
};

/*************************DWE**************************/
enum {
    DEWARP_MODEL_LENS_DISTORTION_CORRECTION = 1 << 0,
    DEWARP_MODEL_FISHEYE_EXPAND             = 1 << 1,
    DEWARP_MODEL_SPLIT_SCREEN               = 1 << 2,
    DEWARP_MODEL_FISHEYE_DEWARP             = 1 << 3,
};

typedef struct DewarpParams {
    int mode;
    bool hflip, vflip, bypass;
    double mat[17];
} DewarpParams;

DewarpParams dweParams;

void parseDewarpParams(Json::Value& node) {
    JH_READ_VAL(dweParams.mode, node, "mode");
    JH_READ_VAL(dweParams.hflip, node, "hflip");
    JH_READ_VAL(dweParams.vflip, node, "vflip");
    JH_READ_VAL(dweParams.bypass, node, "bypass");
    JH_READ_ARRAY(node, "mat", dweParams.mat);
}

void setDewarpParams() {
    Json::Value jRequest, jResponse;
    jRequest["dwe"]["mode"] = dweParams.mode;
    jRequest["dwe"]["hflip"] = dweParams.hflip;
    jRequest["dwe"]["vflip"] = dweParams.vflip;
    jRequest["dwe"]["bypass"] = dweParams.bypass;
    for (int i = 0; i < 17; i++) {
        jRequest["dwe"]["mat"][i] = dweParams.mat[i];
    }
    viv_private_ioctl(IF_DWE_S_PARAMS, jRequest, jResponse);
}


struct OptionDewarpMode0 : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        dweParams.mode = DEWARP_MODEL_LENS_DISTORTION_CORRECTION;
        setDewarpParams();
    }
};

struct OptionDewarpMode1 : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        dweParams.mode = DEWARP_MODEL_FISHEYE_EXPAND;
        setDewarpParams();
    }
};

struct OptionDewarpMode2 : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        dweParams.mode = DEWARP_MODEL_SPLIT_SCREEN;
        setDewarpParams();
    }
};

struct OptionDewarpMode3 : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        dweParams.mode = DEWARP_MODEL_FISHEYE_DEWARP;
        setDewarpParams();
    }
};

struct OptionDewarpHFlip : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        dweParams.hflip = !dweParams.hflip;
        setDewarpParams();
    }
};

struct OptionDewarpVFlip : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        dweParams.vflip = !dweParams.vflip;
        setDewarpParams();
    }
};

struct OptionDewarpBypass : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        dweParams.bypass = !dweParams.bypass;
        setDewarpParams();
    }
};

struct PageDewarpMap : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input 17 double number: (9 matrix + 8 coefficient)\n:");
    }
    void process(std::string &str) {
        std::vector<float> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 17) {
            strError = "input error number";
        } else {
            for (int i = 0; i < 17; i++) {
                dweParams.mat[i] = params[i];
            }
            setDewarpParams();
        }
    }
};

struct PageDwe : BasePage {
    PageDwe(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionDewarpMode0(this, "DEWARP_MODEL_LENS_DISTORTION_CORRECTION"));
        insert(new OptionDewarpMode1(this, "DEWARP_MODEL_FISHEYE_EXPAND"));
        // insert(new OptionDewarpMode2(this, "DEWARP_MODEL_SPLIT_SCREEN")); BUG here
        insert(new OptionDewarpMode3(this, "DEWARP_MODEL_FISHEYE_DEWARP"));
        insert(new OptionDewarpHFlip(this, "DEWARP HFLIP ON/OFF"));
        insert(new OptionDewarpVFlip(this, "DEWARP VFLIP ON/OFF"));
        insert(new OptionDewarpBypass(this, "DEWARP BYPASS ON/OFF"));
        insert(new PageDewarpMap(this, "DEWARP MATRIX"));
    }
};

/*************************CAPTRUE**************************/
struct OptionCapture : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_CAPTURE, jRequest, jResponse);
    }
};

struct OptionPreload : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_SENSOR_LIB_PRELOAD, jRequest, jResponse);
    }
};

struct PageRegSet : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input params : addr, value\n:");
    }
    void process(std::string& str) {
        std::vector<uint32_t> params;
        Splitter::exec(str, ",", params);
        if (params.size() != 2) {
            strError = ("input error addr, value.");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[SENSOR_ADDRESS_PARAMS] = params[0];
            jRequest[SENSOR_VALUE_PARAMS] = params[1];

            viv_private_ioctl(IF_SENSOR_S_REG, jRequest, jResponse);
        }
    }
};

struct PageRegGet : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input params : addr\n:");
    }
    void process(std::string& str) {
        std::vector<uint32_t> params;
        Splitter::exec(str, ",", params);
        if (params.size() != 1) {
            strError = ("input error addr");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[SENSOR_ADDRESS_PARAMS] = params[0];
            viv_private_ioctl(IF_SENSOR_G_REG, jRequest, jResponse);
            jRequest[SENSOR_VALUE_PARAMS] = params[1];
            uint32_t val = jResponse[SENSOR_VALUE_PARAMS].asUInt();
            printf("Register 0x%08x:0x%08x\n", params[0], val );

        }
    }
};



/*************************BLS**************************/
// always open bls, do not support on/off at video streaming.
struct PageBls : BasePage {
    using BasePage::BasePage;
    void dump() {
        Json::Value jRequest, jResponse;
        int r, gr, gb, b;
        viv_private_ioctl(IF_BLS_G_CFG, jRequest, jResponse);
        r  = jResponse[BLS_RED_PARAMS].asInt();
        gr = jResponse[BLS_GREEN_R_PARAMS].asInt();
        gb = jResponse[BLS_GREEN_B_PARAMS].asInt();
        b  = jResponse[BLS_BLUE_PARAMS].asInt();
        printf("bls params: %d, %d, %d, %d\n", r, gr, gb, b);
        printf("please input params : [r, gr, gb, b]\n:");
    }
    void process(std::string& str) {
        std::vector<int> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 4) {
            strError = ("input error [r, gb, gr, b].");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[BLS_RED_PARAMS] = std::max(-4095, std::min(4095, params[0]));
            jRequest[BLS_GREEN_R_PARAMS] = std::max(-4095, std::min(4095, params[1]));
            jRequest[BLS_GREEN_B_PARAMS] = std::max(-4095, std::min(4095, params[2]));
            jRequest[BLS_BLUE_PARAMS] = std::max(-4095, std::min(4095, params[3]));
            viv_private_ioctl(IF_BLS_S_CFG, jRequest, jResponse);
        }
    }
};

/*************************LSC**************************/
struct OptionLscEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "lsc enabled" : "lsc disabled";
        Json::Value jRequest, jResponse;
        jRequest[LSC_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_LSC_S_EN, jRequest, jResponse);
    }
};

struct PageLsc : BasePage {
    using BasePage::BasePage;
    PageLsc(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionLscEnable(this, "LSC ON/OFF")); // (dwe bypass is better)
    }
};

/*************************DEMOSAIC**************************/
struct OptionDemosaicMode : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "demosaic normal" : "demosaic bypass";
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_DEMOSAIC_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[DEMOSAIC_MODE_PARAMS] = enable ? 1 : 2;
        viv_private_ioctl(IF_DEMOSAIC_S_CFG, jRequest, jResponse);
    }
};

struct PageDemosaicThreshold : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input threshold : [0 to 255]\n:");
    }
    void process(std::string& str) {
        int threshold = atoi(str.c_str());
        if (threshold < 0 || threshold > 255) {
            strError = ("input error.");
        } else {
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_DEMOSAIC_G_CFG, jRequest, jResponse);
            jRequest = jResponse;
            jRequest[DEMOSAIC_THRESHOLD_PARAMS] = threshold;
            viv_private_ioctl(IF_DEMOSAIC_S_CFG, jRequest, jResponse);
        }
    }
};

struct PageDemosaic : BasePage {
    using BasePage::BasePage;
    PageDemosaic(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionDemosaicMode(this, "DEMOSAIC SWITCH MODE"));
        insert(new PageDemosaicThreshold(this, "DEMOSAIC THRESHOLD"));
    }
};

/*************************GAMMA**************************/
struct OptionGammaEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "gamma enabled" : "gamma disabled";
        Json::Value jRequest, jResponse;
        jRequest[GC_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_GC_S_EN, jRequest, jResponse);
    }
};

struct OptionGammaScaling : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "gamma mode LOG" : "gamma mode EQU";
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_GC_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        jRequest[GC_MODE_PARAMS] = enable ? 1 : 2;
        viv_private_ioctl(IF_GC_S_CFG, jRequest, jResponse);
    }
};

struct PageGammaCurve : BasePage {
    using BasePage::BasePage;
    void dump() {
        printf("please input gamma : [0.1 to 4.0]\n:");
    }
    void process(std::string& str) {
        float fGamma = atof(str.c_str());
        if (fGamma < 0.1f || fGamma > 4.0f) {
            strError = ("input error.");
        } else {
            uint16_t curve[17] = {0};
            uint16_t gamma_x_equ[16] = {256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256};
            uint16_t gamma_x_log[16] = {64, 64, 64, 64, 128, 128, 128, 128, 256, 256, 256, 256, 512, 512, 512, 512};
            uint16_t *pTable;
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_GC_G_CFG, jRequest, jResponse);
            int mode = 0;
            JH_READ_VAL(mode, jResponse, GC_MODE_PARAMS);
            jRequest = jResponse;
            float dinvgamma = 1.0f/fGamma;
            float sumx = 0;
            pTable = mode == 1 ? gamma_x_log : gamma_x_equ;
            for(int i = 0; i < 16; i++) {
                sumx += pTable[i];
                curve[i+1]= std::min(1023.0f, std::max(0.f, pow(((float)sumx)/4096.0f, dinvgamma) * 1024));
            }
            addArray(curve, jRequest, GC_CURVE_PARAMS);
            viv_private_ioctl(IF_GC_S_CURVE, jRequest, jResponse);
        }
    }
};

struct PageGamma : BasePage {
    using BasePage::BasePage;
    PageGamma(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionGammaEnable(this, "GAMMA ON/OFF"));
        insert(new OptionGammaScaling(this, "SWITCH GAMMA SCALING MODE"));
        insert(new PageGammaCurve(this, "GAMMA CURVE"));
    }
};


/*************************FILTER**************************/
struct OptionFilterEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "filter enabled" : "filter disabled";
        Json::Value jRequest, jResponse;
        jRequest[FILTER_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_FILTER_S_EN, jRequest, jResponse);
    }
};

struct PageFilterThreshold : BasePage {
    using BasePage::BasePage;
    void dump() override {
        uint8_t dl, sl;
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_FILTER_G_CFG, jRequest, jResponse);
        dl = jResponse[FILTER_DENOISE_PARAMS].asUInt();
        sl = jResponse[FILTER_SHARPEN_PARAMS].asUInt();

        printf("filter level(denoise, sharpen): %d, %d\n", dl, sl);
        printf("please input new two levels : (1-10)\n:");
    }

    virtual void process(std::string &str) override {
        std::vector<int> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 2) {
            strError = "Input Error.";
        } else {
            if ( (params[0] > 10 || params[0] < 1 ) ||
                 (params[1] > 10 || params[1] < 1 ))  {
                 strError = "Input Error.";
            } else {
                Json::Value jRequest, jResponse;
                jRequest[FILTER_DENOISE_PARAMS] = params[0];
                jRequest[FILTER_SHARPEN_PARAMS] = params[1];
                viv_private_ioctl(IF_FILTER_S_CFG, jRequest, jResponse);
            }
        }
    }
};

struct PageFilter : BasePage {
    using BasePage::BasePage;
    PageFilter(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionFilterEnable(this, "FILTER ON/OFF"));
        insert(new PageFilterThreshold(this, "Adjust denoise/sharpen level"));
    }
};

/*************************CAC**************************/
struct OptionCacEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "cac enabled" : "cac disabled";
        Json::Value jRequest, jResponse;
        jRequest[CAC_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_CAC_S_EN, jRequest, jResponse);
    }
};

struct PageCac : BasePage {
    using BasePage::BasePage;
    PageCac(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionCacEnable(this, "CAC ON/OFF"));
    }
};

/*************************CNR**************************/
struct OptionCnrEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "cnr enabled" : "cnr disabled";
        Json::Value jRequest, jResponse;
        jRequest[CNR_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_CNR_S_EN, jRequest, jResponse);
    }
};

struct PageCnrThreshold : BasePage {
    using BasePage::BasePage;
    void dump() override {
        uint32_t tc1, tc2;
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_CNR_G_CFG, jRequest, jResponse);
        tc1 = jResponse[CNR_TC1_PARAMS].asUInt();
        tc2 = jResponse[CNR_TC2_PARAMS].asUInt();

        printf("cnr params(thresh1, thresh2): %d, %d\n", tc1, tc2);
        printf("please input params : thresh1, thresh2\n:");
    }

    virtual void process(std::string& str) override {
        std::vector<int> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 2) {
            strError = ("input error.");
        } else {
            Json::Value jRequest, jResponse;
            jRequest[CNR_TC1_PARAMS] = params[0];
            jRequest[CNR_TC2_PARAMS] = params[1];
            viv_private_ioctl(IF_CNR_S_CFG, jRequest, jResponse);
        }
    }
};

struct PageCnr : BasePage {
    using BasePage::BasePage;
    PageCnr(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionCnrEnable(this, "CNR ON/OFF"));
        insert(new PageCnrThreshold(this, "Adjust CNR threshold"));
    }
};

struct OptionCprocEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "cproc enabled" : "cproc disabled";
        Json::Value jRequest, jResponse;
        jRequest[CPROC_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_CPROC_S_EN, jRequest, jResponse);
    }
};

struct OptionCprocSwitchLumain : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        enable = !enable;
        strError = enable ? "lumain bt.601" : "lumain full range";
        jRequest[CPROC_LUMA_IN_PARAMS] = enable ? 1 : 2;
        viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
    }
};

struct OptionCprocSwitchLumaout : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        enable = !enable;
        strError = enable ? "lumaout bt.601" : "lumaout full range";
        jRequest[CPROC_LUMA_OUT_PARAMS] = enable ? 1 : 2;
        viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);

    }
};

struct OptionCprocSwitchChromaout : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
        jRequest = jResponse;
        enable = !enable;
        strError = enable ? "chromaout bt.601" : "chromaout full range";
        jRequest[CPROC_CHROMA_OUT_PARAMS] = enable ? 1 : 2;
        viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
    }
};

struct PageCprocAdjustBrightness : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input new brightness gain: (-127 to 127)\n:");
    }
    void process(std::string &str) {
        int brightness = atoi(str.c_str());
        if (brightness > -128 && brightness < 128) {
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
            jRequest = jResponse;
            jRequest[CPROC_BRIGHTNESS_PARAMS] = brightness;
            viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
        } else {
            strError = "input error.";
        }
    }
};

struct PageCprocAdjustContrast : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input new contrast: (0 to 1.99)\n:");
    }
    void process(std::string &str) {
        float contrast = atof(str.c_str());
        if (contrast >= 0.0f && contrast < 1.99f) {
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
            jRequest = jResponse;
            jRequest[CPROC_CONTRAST_PARAMS] = contrast;
            viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
        } else {
            strError = "input error.";
        }
    }
};

struct PageCprocAdjustSaturation : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input new saturation: (0 to 1.99)\n:");
    }
    void process(std::string &str) {
        float saturation = atof(str.c_str());
        if (saturation >= 0.0f && saturation < 1.99f) {
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
            jRequest = jResponse;
            jRequest[CPROC_SATURATION_PARAMS] = saturation;
            viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
        } else {
            strError = "input error.";
        }
    }
};

struct PageCprocAdjustHue : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input new brightness gain: (-127 to 127)\n:");
    }
    void process(std::string &str) {
        float hue = atof(str.c_str());
        if (hue > -128 && hue < 128) {
            Json::Value jRequest, jResponse;
            viv_private_ioctl(IF_CPROC_G_CFG, jRequest, jResponse);
            jRequest = jResponse;
            jRequest[CPROC_HUE_PARAMS] = hue;
            viv_private_ioctl(IF_CPROC_S_CFG, jRequest, jResponse);
        } else {
            strError = "input error.";
        }
    }
};

struct PageCproc : BasePage {
    using BasePage::BasePage;
    PageCproc(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionCprocEnable(this, "CPROC ON/OFF"));
        insert(new PageCprocAdjustBrightness(this, "Adjust brightness"));
        insert(new PageCprocAdjustContrast(this, "Adjust contrast"));
        insert(new PageCprocAdjustSaturation(this, "Adjust saturation"));
        insert(new PageCprocAdjustHue(this, "Adjust HUE"));
    }
};

/*************************DPCC**************************/
struct OptionDpccEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "dpcc enabled" : "dpcc disabled";
        Json::Value jRequest, jResponse;
        jRequest[DPCC_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_DPCC_S_EN, jRequest, jResponse);
    }
};

struct PageDpcc : BasePage {
    using BasePage::BasePage;
    PageDpcc(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionDpccEnable(this, "DPCC ON/OFF"));
    }
};

/*************************DPF**************************/
struct OptionDpfEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "dpf enabled" : "dpf disabled";
        Json::Value jRequest, jResponse;
        jRequest[DPF_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_DPF_S_EN, jRequest, jResponse);
    }
};

struct PageDpf : BasePage {
    using BasePage::BasePage;
    PageDpf(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionDpfEnable(this, "DPF ON/OFF"));
    }
};

/*************************EE**************************/


/*************************DNR2**************************/


/*************************DNR3**************************/


/*************************WDR3**************************/
struct OptionWdr3Enable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "wdr3 enabled" : "wdr3 disabled";
        Json::Value jRequest, jResponse;
        jRequest[WDR_ENABLE_PARAMS] = enable;
        jRequest[WDR_GENERATION_PARAMS] = 2; // wdr3
        viv_private_ioctl(IF_WDR_S_EN, jRequest, jResponse);
    }
};

struct PageWdr3Strength : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input wdr3 strength : [0-128] (strength, maxGain, globalStrength)\n:");
    }

    virtual void process(std::string &str) override {
        std::vector<int> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 3) {
            strError = "Input Error.";
        } else {
            for (int i = 0; i < 3; i++) {
                if (params[i] < 0 || params[i] > 128) {
                    strError = "Input Error.";
                    return;
                }
            }
            Json::Value jRequest, jResponse;
            jRequest[WDR_ENABLE_PARAMS] = true;
            jRequest[WDR_GENERATION_PARAMS] = 2; // wdr3
            jRequest[WDR_STRENGTH_PARAMS] = params[0];
            jRequest[WDR_GAIN_MAX_PARAMS] = params[1];
            jRequest[WDR_STRENGTH_GLOBAL_PARAMS] = params[2];
            viv_private_ioctl(IF_WDR_S_CFG, jRequest, jResponse);
        }
    }
};

struct PageWdr3 : BasePage {
    using BasePage::BasePage;
    PageWdr3(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionWdr3Enable(this, "WDR3 ON/OFF"));
        insert(new PageWdr3Strength(this, "WDR3 STRENGTH INPUT"));
    }
};

/*************************HDR**************************/
struct OptionHdrEnable : BaseOption {
    using BaseOption::BaseOption;
    virtual void onSelected() {
        enable = !enable;
        strError = enable ? "hdr enabled" : "hdr disabled";
        Json::Value jRequest, jResponse;
        jRequest[HDR_ENABLE_PARAMS] = enable;
        viv_private_ioctl(IF_HDR_S_EN, jRequest, jResponse);
    }
};

struct PageHdrRatio : BasePage {
    using BasePage::BasePage;
    void dump() override {
        printf("please input extension bit[0-4], hdr ratio[0.0-64.0](extBit, ratio)\n:");
    }

    virtual void process(std::string &str) override {
        std::vector<int> params;
        Splitter::exec(str, ",", params);
        if (params.size() < 2) {
            strError = "Input Error.";
        } else {
                if (params[0] < 0 || params[0] > 4) {
                    strError = "Input Error.";
                    return;
                }
                if (params[1] < 1 || params[1] > 64) {
                    strError = "Input Error.";
                    return;
                }
            Json::Value jRequest, jResponse;
            jRequest[HDR_EXTENSION_BIT_PARAMS] = params[0];
            jRequest[HDR_EXPOSURE_RATIO_PARAMS] = params[1];
            viv_private_ioctl(IF_HDR_S_CFG, jRequest, jResponse);
        }
    }
};

struct PageHdr : BasePage {
    using BasePage::BasePage;
    PageHdr(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionHdrEnable(this, "HDR ON/OFF"));
        insert(new PageHdrRatio(this, "HDR RATIO INPUT"));
    }
};

/*************************MAIN PAGE**************************/
// holder when one page is not enough.
struct PageMain3 : BasePage {
    PageMain3(IBasePage* page, const char* szName) : BasePage(page, szName) {
      insert(new PageRegGet(this,"REGGET"));
      insert(new PageRegSet(this,"REGSET"));
      insert(new OptionPreload(this,"PRELOAD"));
    }
};

struct PageMain2 : BasePage {
    PageMain2(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new PageDemosaic(this, "DEMOSAIC"));
        insert(new PageFilter(this, "FILTER"));
        insert(new PageCac(this, "CAC"));
        insert(new PageCnr(this, "CNR"));
        insert(new PageDpcc(this, "DPCC"));
        insert(new PageDpf(this, "DPF"));
        insert(new PageWdr3(this, "WDR3"));
        insert(new PageHdr(this, "HDR"));
        insert(new PageMain3(this, "NEXT->"));
    }
};

struct PageMain : BasePage {
    PageMain(IBasePage* page, const char* szName) : BasePage(page, szName) {
        insert(new OptionCapture(this, "CAPTURE"));
        insert(new PageDwe(this, "DWE"));
        insert(new PageFps(this, "FPS"));
        insert(new PageAec(this, "AEC"));
        insert(new PageAwb(this, "AWB"));
        insert(new PageBls(this, "BLS"));
        insert(new PageLsc(this, "LSC"));
        insert(new PageCproc(this, "CPROC"));
        insert(new PageGamma(this, "GAMMA"));
        insert(new PageMain2(this, "NEXT->"));
    }
};

/*************************MAIN LOOP**************************/
void loop() {
    while (1) {
        Json::Value jRequest, jResponse;
        viv_private_ioctl(IF_DWE_G_PARAMS, jRequest, jResponse);
        parseDewarpParams(jResponse["dwe"]);
        if ( system("clear\n") < 0 ) {
		ALOGE("can't clear screen");
		break;
	}
#if 0
        printf("dewarp:  mode %d, hflip %d, vflip %d, bypass %d\n", dweParams.mode, dweParams.hflip, dweParams.vflip, dweParams.bypass);
        printf("   camera_matrix :  ");
        for (int i = 0; i < 17; i++) {
            printf("%.3f, ", dweParams.mat[i]);
        }
        printf("\n");
#endif
        printf("\n");
        pCurrentPage->dump();
        if (!strError.empty())
            ALOGW("%s", strError.c_str());
        strError.clear();
        std::string inputString;
        std::getline(std::cin, inputString);
        if (inputString.empty()) {
            pCurrentPage->navigateBack();
            continue;
        }
        pCurrentPage->process(inputString);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        ALOGE("usage: ./vvext videoid streamid");
        ALOGE("OR");
        ALOGE("usage: ./vvext videoid streamid 'windowname' 'param string' ");
        return 1;
    }
    int videoid = atoi(argv[1]);
    streamid = atoi(argv[2]);

    char szFile[256] = {0};
    sprintf(szFile, "/dev/video%d", videoid);
    fd = ::open(szFile, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        ALOGE("can't open video file %s", szFile);
        return 1;
    }
    v4l2_capability caps;
    int result = ::ioctl(fd, VIDIOC_QUERYCAP, &caps);
    if (result  < 0) {
        ALOGE("failed to get device caps for %s (%d = %s)", szFile, errno, strerror(errno));
        return 1;
    }

    ALOGI("Open Device: %s (fd=%d)", szFile, fd);
    ALOGI("  Driver: %s", caps.driver);

    if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
        ALOGI("found viv video dev %s", szFile);
        int streamid = -1;
        ::ioctl(fd, VIV_VIDIOC_S_STREAMID, &streamid);
    }

    PageMain pageMain(NULL, "MAIN PAGE");

    if (argc == 5) {
        /* command mode  */
        ALOGI("get command %s %s", argv[3], argv[4]);
        std::string strParams = argv[4];
        pCurrentPage = pageMain.search(argv[3]);
        if (!pCurrentPage) {
            ALOGE("failed to search window: %s", argv[3]);
            goto end;
        }

        if (pCurrentPage->isPage) {
            pCurrentPage->process(strParams);
        } else {
            BaseOption* pOption = (BaseOption*)pCurrentPage;
            pOption->enable = ! atoi(strParams.c_str());
            pOption->onSelected();
        }
    } else{
        /* Window mode */
        pCurrentPage = (IBasePage*)&pageMain;
        std::thread dt(loop);
        sig.wait();
    }

end:
    close(fd);
    return 0;
}
