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

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

#include <algorithm>

#include "log.h"
#include "BufferManager.h"
#include "MediaBuffer.h"
#include "V4l2File.h"

#include "viv_video_kevent.h"
#include "viv_v4l2_ioctl.h"
#include "viv_v4l2_ioctl2.h"

#include "V4l2Event.h"

#define LOGTAG "V4l2Event"

static std::map<int, int> v4l2_format_cov {
    {V4L2_PIX_FMT_YUYV, MEDIA_PIX_FMT_YUV422I},
    {V4L2_PIX_FMT_NV12, MEDIA_PIX_FMT_YUV420SP},
    {V4L2_PIX_FMT_NV16, MEDIA_PIX_FMT_YUV422SP},
    {V4L2_PIX_FMT_SRGGB8, MEDIA_PIX_FMT_RAW8},
    {V4L2_PIX_FMT_SGRBG8, MEDIA_PIX_FMT_RAW8},
    {V4L2_PIX_FMT_SGBRG8, MEDIA_PIX_FMT_RAW8},
    {V4L2_PIX_FMT_SBGGR8, MEDIA_PIX_FMT_RAW8},
    {V4L2_PIX_FMT_SRGGB10, MEDIA_PIX_FMT_RAW10},
    {V4L2_PIX_FMT_SGRBG10, MEDIA_PIX_FMT_RAW10},
    {V4L2_PIX_FMT_SGBRG10, MEDIA_PIX_FMT_RAW10},
    {V4L2_PIX_FMT_SBGGR10, MEDIA_PIX_FMT_RAW10},
    {V4L2_PIX_FMT_SRGGB12, MEDIA_PIX_FMT_RAW12},
    {V4L2_PIX_FMT_SGRBG12, MEDIA_PIX_FMT_RAW12},
    {V4L2_PIX_FMT_SGBRG12, MEDIA_PIX_FMT_RAW12},
    {V4L2_PIX_FMT_SBGGR12, MEDIA_PIX_FMT_RAW12}
};

V4l2Event::V4l2Event() {
    bRunning = true;
    mThread = std::thread([this]() { eventLoop(); });
    qbufThread = std::thread([this]() { qbufLoop(); });
}

V4l2Event::~V4l2Event() {
    bRunning = false;
    qbufThread.join();
    mThread.join();
}

/* should not block qbuf event at anytime */
void V4l2Event::qbufLoop() {
    int eventDevice = 0;
    for (int i = 0; i < 20; i++) {
        char szFile[64];
        sprintf(szFile, "/dev/video%d", i);
        eventDevice = ::open(szFile, O_RDWR | O_NONBLOCK);
        if (eventDevice < 0) {
            ALOGE("can't open video file %s", szFile);
            continue;
        }
        v4l2_capability caps;
        int result = ioctl(eventDevice, VIDIOC_QUERYCAP, &caps);
        if (result  < 0) {
            ALOGE("failed to get device caps for %s (%d = %s)", szFile, errno, strerror(errno));
            return;
        }

        ALOGI("Open Device: %s (fd=%d)", szFile, eventDevice);
        ALOGI("  Driver: %s", caps.driver);

        if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
            ALOGI("found viv video dev %s", szFile);
            int streamid = -1;
            ioctl(eventDevice, VIV_VIDIOC_S_STREAMID, &streamid);
            break;
        }
    }
    EventData data;
    struct v4l2_event_subscription subscribe;
    struct v4l2_event kevent;

    memset(&subscribe, 0, sizeof(subscribe));
    subscribe.type = VIV_VIDEO_EVENT_TYPE;
    subscribe.id = VIV_VIDEO_EVENT_QBUF;
    if (ioctl(eventDevice, VIDIOC_SUBSCRIBE_EVENT, &subscribe) < 0) {
        ALOGE("subscribe failed.");
        return;
    }

    while (bRunning) {
        fd_set fds;
        FD_ZERO (&fds);
        FD_SET (eventDevice, &fds);
        select (eventDevice + 1, &fds, NULL, NULL, NULL);
        if (ioctl(eventDevice, VIDIOC_DQEVENT, &kevent) == 0) {
            struct viv_video_event event;
            memcpy(&event, kevent.u.data, sizeof(event));
            switch (kevent.id) {
            case VIV_VIDEO_EVENT_QBUF: {
                V4l2BufferManager::inst()->push(event.file, event.addr);
                data.type = MEDIA_EVENT_QUEUE_BUFFER;
                data.data = (void*) event.addr;
                data.stream_id = event.stream_id;
                notifyAll(data);
                break;
            }
            }
            if (event.sync) {
                ioctl(eventDevice, VIV_VIDIOC_EVENT_COMPLETE, &event);
            }
        } else {
            ALOGE("%s faile to pool event %s", __func__, strerror(errno));
        }
    }
}

void V4l2Event::eventLoop() {
    int eventDevice = V4l2File::inst()->open();
    EventData data;
    struct v4l2_event_subscription subscribe;
    struct v4l2_event kevent;

    memset(&subscribe, 0, sizeof(subscribe));
    subscribe.type = VIV_VIDEO_EVENT_TYPE;

    for (int i = VIV_VIDEO_EVENT_MIN+1; i < VIV_VIDEO_EVENT_QBUF; i++) {
        subscribe.id = i;
        if (ioctl(eventDevice, VIDIOC_SUBSCRIBE_EVENT, &subscribe) < 0) {
            ALOGE("subscribe failed.");
            return;
        }
    }

    std::map<uint64_t, char*> jsonBufferMap;

    while (bRunning) {
        fd_set fds;
        FD_ZERO (&fds);
        FD_SET (eventDevice, &fds);
        select (eventDevice + 1, &fds, NULL, NULL, NULL);
        data.jsonRequest.clear();
        data.jsonResponse.clear();
        if (ioctl(eventDevice, VIDIOC_DQEVENT, &kevent) == 0) {
            struct viv_video_event event;
            memcpy(&event, kevent.u.data, sizeof(event));
            switch (kevent.id) {
            case VIV_VIDEO_EVENT_NEW_STREAM :
                data.type = MEDIA_EVENT_STREAM_OPEN;
                data.stream_id = event.stream_id;
                notifyAll(data);
                break;
            case VIV_VIDEO_EVENT_DEL_STREAM :
                data.type = MEDIA_EVENT_STREAM_CLOSE;
                data.stream_id = event.stream_id;
                notifyAll(data);
                break;
            case VIV_VIDEO_EVENT_START_STREAM:
                data.type = MEDIA_EVENT_STREAM_START;
                data.stream_id = event.stream_id;
                notifyAll(data);
                break;
            case VIV_VIDEO_EVENT_STOP_STREAM:
                data.type = MEDIA_EVENT_STREAM_STOP;
                data.stream_id = event.stream_id;
                notifyAll(data);
                break;
	        case VIV_VIDEO_EVENT_SET_FMT:
                data.type = MEDIA_EVENT_STREAM_SET_FMT;
                data.stream_id = event.stream_id;
                data.res.width = event.addr;
                data.res.height = event.response;
                data.res.format = v4l2_format_cov[event.buf_index];
                notifyAll(data);
                break;
            case VIV_VIDEO_EVENT_PASS_JSON: {
                data.type = MEDIA_EVENT_PASS_JSON;
                data.stream_id = event.stream_id;
                data.ctrlId = event.buf_index;
                if (jsonBufferMap.find(event.addr) == jsonBufferMap.end()) {
                    jsonBufferMap[event.addr] = (char*)V4l2File::inst()->mmap(event.addr, VIV_JSON_BUFFER_SIZE);
                }
                if (jsonBufferMap.find(event.response) == jsonBufferMap.end()) {
                    jsonBufferMap[event.response] = (char*)V4l2File::inst()->mmap(event.response, VIV_JSON_BUFFER_SIZE);
                }
                Json::Reader reader;
                std::string str = jsonBufferMap[event.addr];
                ALOGW("get json request: %x %s", data.ctrlId, jsonBufferMap[event.addr]);
                reader.parse(str, data.jsonRequest, true);
                notifyAll(data);
                strcpy(jsonBufferMap[event.response], data.jsonResponse.toStyledString().c_str());
                break;
            }
            case VIV_VIDEO_EVENT_EXTCTRL: {
                if (jsonBufferMap.find(event.addr) == jsonBufferMap.end()) {
                    jsonBufferMap[event.addr] = (char*)V4l2File::inst()->mmap(event.addr, VIV_JSON_BUFFER_SIZE);
                }
                Json::Reader reader;
                std::string str = jsonBufferMap[event.addr];
                std::string::size_type startpos = 0;
                while (startpos!= std::string::npos) {
                    if ((startpos = str.find(';')) != std::string::npos)
                        str.replace(startpos, 1, ",");
                }
                startpos = 0;
                while (startpos!= std::string::npos) {
                    if ((startpos = str.find('<')) != std::string::npos)
                        str.replace(startpos, 1, "\"");
                }
                startpos = 0;
                while (startpos!= std::string::npos) {
                    if ((startpos = str.find('>')) != std::string::npos)
                        str.replace(startpos, 1, "\"");
                }

                ALOGW("get json request: %s", str.c_str());
                reader.parse(str, data.jsonRequest, true);

                data.type = MEDIA_EVENT_PASS_JSON;

                if (!data.jsonRequest["streamid"].isObject())
                    data.jsonRequest["streamid"] = 0;

                Json::Value id = data.jsonRequest["id"];
                if (id.isString()) {
                    std::string strCid = id.asCString();
                    if (v4l2_cid_map.find(strCid) != v4l2_cid_map.end()) {
                        data.ctrlId = v4l2_cid_map[strCid];
                        notifyAll(data);
                    } else
                        data.jsonResponse["result"] = -1;
                } else
                    data.jsonResponse["result"] = -1;
                strcpy(jsonBufferMap[event.addr], data.jsonResponse.toStyledString().c_str());
                break;
            }
            case VIV_VIDEO_EVENT_QUERYCAPS: {
                data.type = MEDIA_EVENT_PASS_JSON;
                data.stream_id = event.stream_id;
                data.jsonRequest["streamid"] = 0;
                data.jsonRequest["index"] = event.buf_index;
                data.ctrlId = ISPCORE_MODULE_SENSOR_QUERY;
                notifyAll(data);
                ALOGW("get modeinfo:\n   %s", data.jsonResponse.toStyledString().c_str());
                struct vvcam_constant_modeinfo modeinfo;
                uint32_t current = data.jsonResponse["current"].asUInt();
                for (auto& item : data.jsonResponse["caps"]) {
                    /* We presently just configure the current sensor resolution
                     * information into the driver and will be got enumerated by
                     * the users since we cannot support resolution switching on
                     * the fly yet.
                     */
                    if (current == item["index"].asUInt()) {
                        modeinfo.index = item["index"].asUInt();
                        modeinfo.w = item["width"].asUInt();
                        modeinfo.h = item["height"].asUInt();
                        modeinfo.fps = item["fps"].asUInt();
                        modeinfo.brpat = item["bayer_pattern"].asUInt();
                        modeinfo.bitw = item["bit_width"].asUInt();
                        ioctl(eventDevice, VIV_VIDIOC_S_MODEINFO, &modeinfo);
                        break;
                    }
                }
                break;
            }
            case VIV_VIDEO_EVENT_EXTCTRL2: {
                struct v4l2_ctrl_data *pData = (struct v4l2_ctrl_data *)
                        V4l2File::inst()->mmap(event.addr, VIV_JSON_BUFFER_SIZE);
                std::map<int, struct vvctrl> *pCIdMap;
                if (!pData)
                    break;
                if (pData->ctrls.count != 1) {
                    ALOGE("not supported currently!");
                    pData->ret = -1;
                    break;
                }
                if (pData->dir == V4L2_CTRL_GET)
                    pCIdMap = &v4l2_cid_map_get;
                else
                    pCIdMap = &v4l2_cid_map_set;

                struct v4l2_ext_control *pCtrl =
                        nextof(pData, struct v4l2_ext_control *);
                if (pCIdMap->find(pCtrl->id) ==  pCIdMap->end()) {
                    pData->ret = -1;
                    break;
                }
                struct vvctrl &ctrl = pCIdMap->at(pCtrl->id);

                if (ctrl.supported && ctrl.begin)
                    ctrl.begin(pData, data);
                else {
                    pData->ret = -1;
                    break;
                }

                data.type = MEDIA_EVENT_PASS_JSON;
                data.ctrlId = ctrl.ctrlId;
                notifyAll(data);
                if (ctrl.end)
                    ctrl.end(pData, data);
                ALOGW("extctrl2: cid=0x%x,ret=%d", pCtrl->id, pData->ret);
                break;
            }
            case VIV_VIDEO_EVENT_SET_CAPSMODE:{
                struct viv_caps_mode_s caps_mode;
                memset(&caps_mode,0,sizeof(caps_mode));
                ioctl(eventDevice, VIV_VIDIOC_G_CAPS_MODE, &caps_mode);

                data.type = MEDIA_EVENT_PASS_JSON;
                data.stream_id = event.stream_id;
                data.jsonRequest["streamid"] = 0;
                data.jsonRequest["index"] = event.buf_index;
                data.jsonRequest["mode"] = caps_mode.mode;
                data.jsonRequest["CalibXmlName"] = caps_mode.CalibXmlName;
                data.ctrlId = ISPCORE_MODULE_SENSOR_MODE_SET;
                notifyAll(data);

                int ret = data.jsonResponse["result"].asInt();
                ioctl(eventDevice, VIV_VIDIOC_EVENT_RESULT, &ret);
                break;
            }
            case VIV_VIDEO_EVENT_GET_CAPS_SUPPORTS:{
                data.type = MEDIA_EVENT_PASS_JSON;
                data.stream_id = event.stream_id;
                data.jsonRequest["streamid"] = 0;
                data.jsonRequest["index"] = event.buf_index;
                data.ctrlId = ISPCORE_MODULE_SENSOR_QUERY;
                notifyAll(data);
           
                struct viv_caps_supports caps_supports;
                memset(&caps_supports,0,sizeof(caps_supports));

                for (auto& item : data.jsonResponse["caps"]) {
                   caps_supports.count++; 
                   caps_supports.mode[caps_supports.count-1].index = item["index"].asUInt();
                   caps_supports.mode[caps_supports.count-1].width = item["width"].asUInt();
                   caps_supports.mode[caps_supports.count-1].height = item["height"].asUInt();
                   caps_supports.mode[caps_supports.count-1].hdr_mode = item["hdr_mode"].asUInt();
                   caps_supports.mode[caps_supports.count-1].fps = item["fps"].asUInt();
                }
                
                ioctl(eventDevice, VIV_VIDIOC_SET_CAPS_SUPPORTS, &caps_supports);

                break;
            }

            }
            if (event.sync) {
                ioctl(eventDevice, VIV_VIDIOC_EVENT_COMPLETE, &event);
            }
        } else {
            ALOGE("%s faile to pool event %s", __func__, strerror(errno));
        }
    }
    close(eventDevice);
}
