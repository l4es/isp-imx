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

#ifndef DEVELOPER_MEDIACONTROL_INCLUDE_MEDIAEVENT_H_
#define DEVELOPER_MEDIACONTROL_INCLUDE_MEDIAEVENT_H_
#include <set>
#include "EAutoLock.h"
#include "MediaCap.h"
#include <json/json.h>

enum {
    // v4l2 event
    MEDIA_EVENT_STREAM_OPEN = 0,
    MEDIA_EVENT_STREAM_CLOSE,
    MEDIA_EVENT_STREAM_START,
    MEDIA_EVENT_STREAM_STOP,
    MEDIA_EVENT_STREAM_SET_FMT,
    MEDIA_EVENT_QUERY_SESSION_INFO,
    MEDIA_EVENT_QUERY_STREAM_INFO,
    MEDIA_EVENT_SET_STRING_PARAM,
    MEDIA_EVENT_QUEUE_BUFFER,
    MEDIA_EVENT_SET_MODE,
    MEDIA_EVENT_PASS_JSON,
    MEDIA_EVENT_QUERY_STATUS,
};

#ifndef SUPPORT_PASS_JSON
#define SUPPORT_PASS_JSON
#endif

typedef struct EventData {
    // input
    int type;
    int frame_number;
    int stream_id;
    void *data;     // update: MediaBuffer*,  output: Metadata[]
                    // used to set params  string type[]
    int userValue;  // used to set params  int type

    MediaCap res;
    // output
    int session_number;
    int stream_number;
#ifdef SUPPORT_PASS_JSON
    int ctrlId;
    Json::Value jsonRequest;
    Json::Value jsonResponse;
#endif
} EventData;

class EventListener {
 public:
    EventListener() {}
    virtual ~EventListener() {}
    virtual bool handleEvent(/* IN OUT */EventData& data) = 0;
};

class EventSource {
 public:
    EventSource() { }
    virtual ~EventSource() {}
    virtual void registerListener(EventListener* pListener) {
        EAutoLock l(&mMutex);
        mListeners.insert(pListener);
    }

    virtual void removeListener(EventListener* pListener) {
        EAutoLock l(&mMutex);
        mListeners.erase(pListener);
    }

    virtual bool notifyAll(EventData& data) {
        EAutoLock l(&mMutex);
        for (auto& item : mListeners) {
            item->handleEvent(data);
        }
        return true;
    }

 private:
    std::set<EventListener*> mListeners;
    EMutex mMutex;
};

#endif  // DEVELOPER_MEDIACONTROL_INCLUDE_MEDIAEVENT_H_
