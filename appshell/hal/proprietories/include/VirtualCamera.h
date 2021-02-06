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

#ifndef _CAM_SHELL_VIRTUALCAMERA_H_
#define _CAM_SHELL_VIRTUALCAMERA_H_
#include <stdlib.h>
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <thread>
#include "cam_device_api.hpp"

typedef struct CameraCaps {
    int width;
    int height;
    int format;
} CameraCaps;

static std::unordered_map<int, int> cameraFormatBpp = {
    {CAMERA_PIX_FMT_YUV422SP,  16},
    {CAMERA_PIX_FMT_YUV422I,   16},
    {CAMERA_PIX_FMT_YUV420SP,  12},
    {CAMERA_PIX_FMT_YUV444,    24},
    {CAMERA_PIX_FMT_RGB888,    24},
    {CAMERA_PIX_FMT_RGB888P,   24},
};

class VirtualCamera {
public:
    virtual ~VirtualCamera() {}
    virtual int open(int id) = 0;
    virtual void close(int id) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int connectCamera() = 0;
    virtual int disconnectCamera() = 0;
    virtual int ioctl(int cmdID, Json::Value& jsonRequest, Json::Value& jsonResponse) = 0;

    void registerBufferCallback(std::function<void(BufIdentity* buf, int port)> callback = nullptr) {
        mBufferCallback = callback;
    }

    void removeBufferCallback() {
        mBufferCallback = nullptr;
    }

    virtual void setFormat(int port, int w, int h, int f) {
        mCaps[port].width = w;
        mCaps[port].height = h;
        mCaps[port].format = f;
    }

    template<typename T>
    int post(int32_t ctrlId, const char* section, T value) {
        Json::Value jRequest, jResponse;
        if (section != NULL) {
            jRequest[section] = value;
        }
        return ioctl(ctrlId, jRequest, jResponse);
    }

    template<typename inType, typename outType>
    int post(int32_t ctrlId, const char* section, inType value, const char* retSection, outType& retVal) {
        Json::Value jRequest, jResponse;
        if (section != NULL) {
            jRequest[section] = value;
        }
        int ret = ioctl(ctrlId, jRequest, jResponse);
#if 0 //Todo: compile error
        if (!jResponse.empty() && jResponse[retSection].isObject())
            readValueFromNode<outType>(jResponse[retSection], retVal);
#endif
        return ret;
    }

    static VirtualCamera* createObject();

    void updateMetadata(struct isp_metadata* meta, int path);

protected:
    std::function<void(BufIdentity* buf, int port)> mBufferCallback = nullptr;
    std::map<int, CameraCaps> mCaps;
};

class VideoFile {
 public:
    VideoFile() {}
    ~VideoFile();
    
    int open();
    uint64_t alloc(uint64_t size);
    void free(uint64_t addr);
    void* mmap(uint64_t addr, uint64_t size);
    static VideoFile* inst();

 private:
    int fd = -1;
    static VideoFile* mInst;
};

#endif  // _CAM_SHELL_VIRTUALCAMERA_H_
