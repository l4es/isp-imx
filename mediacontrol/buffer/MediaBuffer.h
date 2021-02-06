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

#ifndef DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFER_H_
#define DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFER_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include "MediaCap.h"

#ifndef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#endif

#ifndef VIV_MEDIA_PIX_FMT
#define VIV_MEDIA_PIX_FMT
enum {
    MEDIA_PIX_FMT_YUV422SP = 0,
    MEDIA_PIX_FMT_YUV422I,
    MEDIA_PIX_FMT_YUV420SP,
    MEDIA_PIX_FMT_YUV444,
    MEDIA_PIX_FMT_RGB888,
    MEDIA_PIX_FMT_RGB888P,
    MEDIA_PIX_FMT_RAW8,
    MEDIA_PIX_FMT_RAW10,
    MEDIA_PIX_FMT_RAW12,
};
#endif

static std::unordered_map<std::string, int> mediaFormatStringTable = {
    {"YUV422SP",   MEDIA_PIX_FMT_YUV422SP},
    {"YUV422I",    MEDIA_PIX_FMT_YUV422I},
    {"YUV420SP",   MEDIA_PIX_FMT_YUV420SP},
    {"YUV444",     MEDIA_PIX_FMT_YUV444},
    {"RGB888",     MEDIA_PIX_FMT_RGB888},
    {"RGB888P",    MEDIA_PIX_FMT_RGB888P},
    {"RAW8",       MEDIA_PIX_FMT_RAW8},
    {"RAW10",      MEDIA_PIX_FMT_RAW10},
    {"RAW12",      MEDIA_PIX_FMT_RAW12},
};

enum {
    MEDIA_BUFFER_TYPE_USER_PTR = 0,
    MEDIA_BUFFER_TYPE_MMAP,
    MEDIA_BUFFER_TYPE_ADDR,
};

class MediaBuffer {
 public:
    virtual ~MediaBuffer() {}
    virtual bool create(uint32_t width, uint32_t height, uint32_t format) = 0;
    virtual void release() = 0;
    virtual unsigned char* getBuffer() = 0;
    virtual void copyTo(MediaBuffer* pBuffer) = 0;
    virtual void save(const char* pFileName);
    uint32_t mFrameNumber;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mStride;
    uint32_t mFormat;
    uint32_t mBpp;
    uint32_t mSize;
    uint32_t mStatus;
    uint32_t mBufferType;  // MMAP or USER_PTR image, metadata
    uint32_t mPort;

 protected:
    MediaBuffer();
};

class MediaUserBuffer : public MediaBuffer{
 public:
    MediaUserBuffer();
    ~MediaUserBuffer();
    bool create(uint32_t width, uint32_t height, uint32_t format);
    void release();
    unsigned char* getBuffer() { return mBuffer; }
    virtual void copyTo(MediaBuffer* pBuffer);
 private:
    unsigned char* mBuffer;
};


class MediaAddrBuffer : public MediaBuffer{
 public:
    MediaAddrBuffer();
    virtual ~MediaAddrBuffer();
    bool create(uint32_t width, uint32_t height, uint32_t format) override;
    bool vcreate(uint32_t width, uint32_t height, uint32_t format);
    void release() override;
    unsigned char* getBuffer() override;
    virtual void copyTo(MediaBuffer* pBuffer) override;
    uint64_t baseAddress = 0;
    bool vFlag = false;
    unsigned char* data = NULL;
};

#define BASEADDR(x) ((MediaAddrBuffer*)(x))->baseAddress

#endif  // DEVELOPER_MEDIACONTROL_BUFFER_MEDIABUFFER_H_

