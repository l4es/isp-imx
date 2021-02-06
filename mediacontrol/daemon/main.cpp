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

#include "log.h"
#include "MediaPipeline.h"
#include "MediaBufferQueue.h"

#define LOGTAG "ISP_MEDIA_SERVER"
MediaPipeline pipeline;   // manage all isp/dwe/vse/dec/v4l2 and link streams.
ESignal sig;
std::map<int, MediaBufferQueue> qmap;
int TEST_WIDTH;
int TEST_HEIGHT;
int TEST_FORMAT;

int frameNumber = 0;
void bufferCallBack(MediaBuffer* pBuffer) {
    ALOGD("bufferCallBack got buffer at port %d %d", pBuffer->mPort, frameNumber++);
    MediaBuffer *qb;
    ALOGI("bufferCallBack wxh: %dx%d", pBuffer->mWidth, pBuffer->mHeight);
    auto list = qmap[pBuffer->mPort].mBuffers;
    bool matched = false;
    for (auto& item : list) {
        if ( BASEADDR(item) == BASEADDR(pBuffer)) {
            qmap[pBuffer->mPort].queueBuffer(item);
            matched = true;
        }

        while ( (qb = qmap[pBuffer->mPort].acquiredBuffer()) ) {
            // do something, support multiple thread
            if (frameNumber == 10)
                qb->save("test.yuv");
            qmap[pBuffer->mPort].releaseBuffer(qb);
        }
        // return back buffer, support multiple thread
        while ( (qb = qmap[pBuffer->mPort].dequeueBuffer() )) {
            EventData data;
            data.stream_id = pBuffer->mPort;
            data.type = MEDIA_EVENT_QUEUE_BUFFER;
            data.data = (void*) BASEADDR(qb);
            pipeline.handleEvent(data);
        }
    }
    if (!matched)
        ALOGE("can't match buffer %p !!!!", (unsigned char*)BASEADDR(pBuffer));

}

int main(int argc, char* argv[]) {
    ALOGI("************************************************************");
    ALOGI("       VIV ISP Media Control Framework V%s", MEDIA_SERVER_VERSION);
    ALOGI("************************************************************\n");
    if (argc < 5) {
        ALOGE("args: config_file, width, height, format");
        return 1;
    }

    EventData data;

    pipeline.load(argv[1]);
    TEST_WIDTH = atoi(argv[2]);
    TEST_HEIGHT= atoi(argv[3]);
    TEST_FORMAT = mediaFormatStringTable[argv[4]];
    data.type = MEDIA_EVENT_QUERY_STREAM_INFO;
    pipeline.handleEvent(data);
    CapsArray* capsArray = (CapsArray*)data.data;
    for (int i = 0; i < data.stream_number; i++) {
        for(int j = 0; j < capsArray[i].caps_num; j++) {
            ALOGD("query stream %d caps: format %d   w %d    h %d", i,
                    capsArray[i].cap[j].format,
                    capsArray[i].cap[j].width,
                    capsArray[i].cap[j].height);
        }
    }

    for (int i = 0; i < data.stream_number; i++) {
        if (capsArray[i].caps_num == 0) {
            ALOGE("stream %d has not caps!", i);
            return false;
        }
        data.type = MEDIA_EVENT_STREAM_OPEN;
        data.stream_id = i;
        pipeline.handleEvent(data);

        data.type = MEDIA_EVENT_STREAM_SET_FMT;
        data.res.width = TEST_WIDTH;
        data.res.height = TEST_HEIGHT;
        data.res.format = TEST_FORMAT;
        pipeline.handleEvent(data);
        qmap[i].create( data.res.width, data.res.height, data.res.format, 6);
    }

    // set all buffer before start
    for (int i = 0; i < data.stream_number; i++) {
        data.stream_id = i;
        MediaBuffer* pBuffer = NULL;
        auto iter = qmap.find(i);
        if (iter != qmap.end()) {
            while ((pBuffer = qmap[i].dequeueBuffer()) != NULL) {
                data.type = MEDIA_EVENT_QUEUE_BUFFER;
                data.data = (void*) BASEADDR(pBuffer);
                pipeline.handleEvent(data);
            }
        }
    }

    // must start after set all stream formats
    for (int i = 0; i < data.stream_number; i++) {
        data.stream_id = i;
        data.type = MEDIA_EVENT_STREAM_START;
        pipeline.handleEvent(data);
    }

    pipeline.reigsterBufferCallBack(bufferCallBack);

    sig.wait();

    pipeline.removeBufferCallBack();

    for (int i = 0; i < data.stream_number; i++) {
        data.type = MEDIA_EVENT_STREAM_STOP;
        data.stream_id = i;
        pipeline.handleEvent(data);
        data.stream_id = i;
        data.type = MEDIA_EVENT_STREAM_CLOSE;
        pipeline.handleEvent(data);
    }
    return 0;
}
