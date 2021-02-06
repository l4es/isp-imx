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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <fstream>
#include <iostream>
#include "json/json.h"
#include "log.h"
#include <MediaBuffer.h>

#define LOGTAG "CASE_GENERATOR"
using namespace std;

static std::map<std::string, int> format_map{
    { "YUV422SP",  MEDIA_PIX_FMT_YUV422SP },
    { "YUV422I",   MEDIA_PIX_FMT_YUV422I  },
    { "YUV420SP",  MEDIA_PIX_FMT_YUV420SP }
};

const std::map<std::string, int> dewarp_mode_map{
    { "LENS_CORRECTION",  0 },
    { "FISHEYE_EXPAND",  1 },
    { "SPLIT_SCREEN",  2}
};

const std::vector<int> resolutions = {
    320, 240,
    640, 480,
    800, 600,
    1280, 720,
    1600, 1200,
    1920, 1080,
    2688, 1520,
    3840, 1080,
    3840, 2160,
    4096, 1520,
    4096, 2160,
};

int main() {
    const char* szInitFile = "case/config/1001_Lens_Correction.json";
    const char* szConfigFolder = "case/config_full/";
    const char* szResourceFolder = "case/resource_full/";
    std::ifstream configStream(szInitFile);
    Json::Reader reader;
    Json::Value rootNode;

    bool parseOk = reader.parse(configStream, rootNode, false);
    if (!parseOk) {
        ALOGE("Failed to read configuration file %s", szInitFile);
        ALOGE("%s", reader.getFormatedErrorMessages().c_str());
        return 1;
    }
    ofstream list_full;
    list_full.open("case/list_full.txt");

    int index = 2001;
    for (auto& mode : dewarp_mode_map) {
        for (auto& format : format_map) {
            for (size_t i = 0; i < resolutions.size(); i+=2) {
                int w = resolutions[i];
                int h =  resolutions[i+1];
                char szImageFile[128];
                sprintf(szImageFile, "%sDWRES_%s_%dx%d.yuv", szResourceFolder, format.first.c_str(), w, h);
                rootNode["ID"] = to_string(index);
                rootNode["dewarpMode"] = mode.first;
                rootNode["input"]["format"] = format.first;
                rootNode["output"]["format"] = format.first;
                rootNode["input"]["width"] = w;
                rootNode["input"]["height"] = h;
                rootNode["input"]["file"] = szImageFile;
                rootNode["camera_matrix"][2] = w / 2;
                rootNode["camera_matrix"][5] = h / 2;
                rootNode["split"]["horizon_line"] = h / 2;
                rootNode["split"]["vertical_line_up"] = w / 2;
                rootNode["split"]["vertical_line_down"] = w / 2;
                rootNode["scale"]["factor"] = 1.0f;
                rootNode["scale"]["roix"] = 0.0f;
                rootNode["scale"]["roiy"] = 0.0f;
                rootNode["output"]["width"] = w;
                rootNode["output"]["height"] = h;

                Json::StyledStreamWriter sw;
                char szNewFileName[128];
                sprintf(szNewFileName, "%s%d_%s_%s_%dx%d.json",
                        szConfigFolder,
                        index,
                        mode.first.c_str(),
                        format.first.c_str(),
                        w, h);
                ALOGI("save to %s", szNewFileName);
                std::ofstream os;
                os.open(szNewFileName);
                sw.write(os, rootNode);
                os.close();
                list_full << szNewFileName <<endl;
                index++;
                if (mode.first == "SPLIT_SCREEN")  // split dont support scaling
                    continue;
                if (w <= 800) {  // max size limitation.
                    // test scaling
                    rootNode["ID"] = to_string(index);
                    for (int factor = 2; factor <= 4; factor++) {
                        rootNode["scale"]["factor"] = (float)factor;
                        rootNode["output"]["width"] = w*factor;
                        rootNode["output"]["height"] = h*factor;

                        sprintf(szNewFileName, "%s%d_%s_%s_%dx%d_to%dx%d.json",
                            szConfigFolder,
                            index,
                            mode.first.c_str(),
                            format.first.c_str(),
                            w, h, w*factor, h*factor);
                        ALOGI("save to %s", szNewFileName);
                        os.open(szNewFileName);
                        sw.write(os, rootNode);
                        os.close();
                        list_full << szNewFileName <<endl;
                        index++;
                    }
                } else  {
                    // test crop and scaling
                    int imax = 1;
                    if (w <= 1280) imax = 3;
                    if (w <= 1920) imax = 2;
                    rootNode["ID"] = to_string(index);
                     for (int factor = 1; factor <= imax; factor++) {
                        rootNode["scale"]["factor"] = (float)factor;
                        int rx = w/4;
                        int ry = h/4;
                        int scaledW = w*factor;
                        int scaledH = h*factor;
                        int dw = ALIGN_UP(scaledW/2, 16);
                        int dh = ALIGN_UP(scaledH/2, 8);
                        rootNode["scale"]["roix"] = (float)rx;
                        rootNode["scale"]["roiy"] = (float)ry;
                        rootNode["output"]["width"] = dw;
                        rootNode["output"]["height"] = dh;
                        rootNode["split"]["horizon_line"] = dh / 2;
                        rootNode["split"]["vertical_line_up"] = dw / 2;
                        rootNode["split"]["vertical_line_down"] = dw / 2;
                        sprintf(szNewFileName, "%s%d_%s_%s_%dx%d_to%dx%d_centercrop.json"   ,
                            szConfigFolder,
                            index,
                            mode.first.c_str(),
                            format.first.c_str(),
                            w, h, dw, dh);
                        ALOGI("save to %s", szNewFileName);
                        os.open(szNewFileName);
                        sw.write(os, rootNode);
                        os.close();
                        list_full << szNewFileName <<endl;
                        index++;
                    }
                }
            }
        }
    }
    configStream.close();
    list_full.close();
    return 0;
}
