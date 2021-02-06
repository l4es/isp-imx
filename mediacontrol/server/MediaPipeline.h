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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_MEDIAPIPELINE_H_
#define DEVELOPER_MEDIACONTROL_SERVER_MEDIAPIPELINE_H_
#include <map>
#include <string>
#include <fstream>

#include "IMediaModule.h"
#include "MediaEvent.h"
#include "json/json.h"

#define MEDIA_SERVER_VERSION "4.2.1"

typedef struct StreamNode {
    IMediaModule* prevModule;
    IMediaModule* pModule;
    int prePort;
    int nPort;
} StreamNode;

const int MAX_SUPPORTED_CAPS = 8;
typedef struct CapsArray {
    int caps_num = 0;
    MediaCap cap[MAX_SUPPORTED_CAPS];
} CapsArray;

class MediaPipeline : public EventListener {
 public:
    MediaPipeline();
    ~MediaPipeline();
    bool load(const char* szFileName);  // media link file name.   case/config/xxx.json
    bool handleEvent(EventData& data) override;

    inline int getStreamNumber() { return mapStreams.size(); }
    void reigsterBufferCallBack(std::function<void(MediaBuffer*)> callback = nullptr);
    void removeBufferCallBack();

    bool start(int id);
    bool stop(int id);
    bool setFormat(int id, MediaCap& res);
    bool open(int id);
    bool close(int id);
    bool setStringParam(int id, const char* key, int val);
    bool queueBuffer(int id, uint64_t addr);
    bool setMode(int id, int msgType, void* param);
    int jsonRequest(int streamid, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse);
    const CapsArray* queryAllStreamCaps() { return &mCapsArray[0]; }

 private:
    bool parseMediaControlConfig(const char* pFileName);
    void moduleConfig(const Json::Value& node, const char* pModuleName);
    void resetAllStreamsFormat();
    bool findNearestFormat(int id, MediaCap& res, MediaCap& lres);
    std::map<std::string, IMediaModule*> mapModules;
    std::map<int, std::vector<StreamNode>> mapStreams;    // streamid 0,  isp 0 -> dwe 0
                                                          // streamid 1, isp 0 -> vse 0
    std::map<int, std::vector<MediaCap>> mStreamsCapability;
    std::vector<CapsArray> mCapsArray;
};

#endif  // DEVELOPER_MEDIACONTROL_SERVER_MEDIAPIPELINE_H_
