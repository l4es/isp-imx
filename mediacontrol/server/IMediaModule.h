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

#ifndef DEVELOPER_MEDIACONTROL_SERVER_IMEDIAMODULE_H_
#define DEVELOPER_MEDIACONTROL_SERVER_IMEDIAMODULE_H_

#include <map>
#include <vector>
#include <string>
#include <deque>
#include <functional>

#include "MediaBufferQueue.h"
#include "BufferManager.h"

#include "json/json.h"
#include "viv_v4l2_ioctl.h"

const int PadTypeSink = 0;
const int PadTypeSrc = 1;

enum {
    PAD_TYPE_IMAGE = 0,
    PAD_TYPE_INT,
    PAD_TYPE_FLOAT,
    PAD_TYPE_STRING,
    PAD_TYPE_UNKNOWN,
};

typedef struct MediaPad {
    int type = 0;
    int state = 0;
    MediaBuffer* image = NULL;
    int nVal = 0;
    float fVal = 0;
    std::string strVal;
    MediaPad() { state = false; }
    MediaPad(int nType) {
        type = nType;
    }
} MediaPad;


class IMediaModule;

typedef struct MediaRemotePad {
    uint32_t nSinkIndex = 0;
    IMediaModule* pNext = NULL;
    MediaRemotePad(uint32_t n, IMediaModule* p) {
        nSinkIndex = n;
        pNext = p;
    }
} MediaRemotePad;

class IMediaModule {
 public:
    virtual ~IMediaModule() {}
    virtual const char* getClassName() = 0;
    virtual void setParam(const char* szKey, float fVal) {
        mParamerter[szKey] = fVal;
    }
    // virtual abstract interface
 protected:
    virtual bool run(uint32_t nSinkIndex) = 0;
    virtual void trigerNext(uint32_t nSrcIndex);
    virtual void triger(MediaBuffer* image, uint32_t nSinkIndex = 0);
    virtual void triger(int nVal, uint32_t nSinkIndex = 0);
    virtual void triger(float fVal, uint32_t nSinkIndex = 0);
    virtual void triger(const char* szVal, uint32_t nSinkIndex = 0);
    virtual void triger(uint32_t nSinkIndex);
    virtual void setSinkState(uint32_t nIndex,  bool bState);
    virtual bool getSinkState(uint32_t nIndex);
    virtual int queryType(int nSinkSrc, uint32_t nIndex);

 public:
    virtual bool connect   (IMediaModule* pNext, uint32_t nSinkIndex, uint32_t nSrcIndex);
    virtual void disconnect(IMediaModule* pNext, uint32_t nSinkIndex, uint32_t nSrcIndex);

    inline std::map<uint32_t, std::vector<MediaRemotePad> >&  getConnection() {
        return mConnections;
    }

    virtual bool start() { return true; }
    virtual bool stop() { return true; }
    virtual bool open() { return true; }
    virtual bool close() { return true; }
    virtual bool load(Json::Value& node) { return true; }
    virtual void setFormat(int nPort, MediaCap& res, int type) {
        mPadCaps[type][nPort] = res;
    }
    inline const MediaCap& getFormat(int nPort, int type) {
        return mPadCaps[type][nPort];
    }

    virtual bool needBackwardFormat(int nPort) { return true; }

    // only using for LocalDisplay
    inline void reigsterBufferCallBack(std::function<void(MediaBuffer*)> callback = nullptr) {
        mBufferCallback = callback;
    }
    // only using for LocalDisplay
    inline void removeBufferCallBack() {
        mBufferCallback = nullptr;
    }

    inline void setEndpoint(int channel, bool bEndpoint) { mEndpoint[channel] = bEndpoint; }
    inline bool isEndpoint(int channel) { return mEndpoint[channel]; }
    virtual bool setMode(int msgType, void* param) { return true; }
    virtual int jsonRequest(int port, int ctrlId, Json::Value& jsonRequest, Json::Value& jsonResponse) { return 0; }

    std::map<int, MediaBufferQueue> mPadQueue;  // only for sink pad.
    virtual void onGlobalPadBufferChanged(int type, int port) { }
    virtual void onConnectedPadBufferChanged(int port, MediaBufferQueue& queue, const char* remoteClass) { }
    void releasePadBuffer(int port, uint64_t addr);

 protected:
    IMediaModule();
    std::map<uint32_t, std::vector<MediaRemotePad> >mConnections;
    std::map<uint32_t, std::vector<MediaRemotePad> >mConnectionsReverse;
    std::map<int, MediaCap> mPadCaps[2];  // input/output
    std::map<std::string, double> mParamerter;

    std::map<int, bool> mEndpoint;

    std::vector<MediaPad> mSink;
    std::vector<MediaPad> mSrc;
    // nsecs_t m_costMillSeconds;
    int mPassCount;
    int mRefCount = 0;
    bool captureFlag = false;
    std::function<void(MediaBuffer*)> mBufferCallback = nullptr;
};

typedef IMediaModule* (*createCallback)(void);

class ModuleRegistry {
 public:
    ModuleRegistry() {
    }
    void add(const char* className, createCallback fun) {
        _classTable[className] = fun;
    }
    IMediaModule* create(const char* name);
    static ModuleRegistry* inst() {
        if (!mRegistry)
            mRegistry = new ModuleRegistry();
        return mRegistry;
    }
    static ModuleRegistry* mRegistry;
 private:
    typedef std::map<std::string, createCallback>  ClassTable;
    ClassTable  _classTable;
};

class RegisterClass {
 public:
    RegisterClass(const char* className, createCallback fun) {
        ModuleRegistry::inst()->add(className, fun);
    }
};

#define DECLARE_DYNAMIC_CLASS() public:\
    static IMediaModule* createObject();

#define IMPLEMENT_DYNAMIC_CLASS(name) \
    extern const  RegisterClass  registerClass##name(#name, name::createObject); \
    IMediaModule* name::createObject() { return new name; }

void SafeRelease(const IMediaModule*& pointer);
#endif  // DEVELOPER_MEDIACONTROL_SERVER_IMEDIAMODULE_H_
