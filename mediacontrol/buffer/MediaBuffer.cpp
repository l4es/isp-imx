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

#include "MediaBuffer.h"

#include <memory.h>
#include "log.h"
#include <IMemoryAllocator.h>

#define LOGTAG "MediaBuffer"

#ifdef ISP_DEC
#define TILE_SIZE 128
#define TILE_STATUS_BIT 4
#endif

static std::unordered_map<int, int> mapFormatBpp = {
    {MEDIA_PIX_FMT_YUV422SP,  16},
    {MEDIA_PIX_FMT_YUV422I,   16},
    {MEDIA_PIX_FMT_YUV420SP,  12},
    {MEDIA_PIX_FMT_YUV444,    24},
    {MEDIA_PIX_FMT_RGB888,    24},
    {MEDIA_PIX_FMT_RGB888P,   24},
    {MEDIA_PIX_FMT_RAW8,       8},
    {MEDIA_PIX_FMT_RAW10,     16},
    {MEDIA_PIX_FMT_RAW12,     16},  // aligned
};

MediaBuffer::MediaBuffer() {
    mWidth     = 0;
    mHeight    = 0;
    mStride    = 0;
    mFormat    = 0;
    mBpp       = 0;
    mSize      = 0;
    mPort      = 0;
}

void MediaBuffer::save(const char* pFileName) {
    FILE* pFile = NULL;;
    if (!(pFile = fopen(pFileName, "wb"))) {
        ALOGE("failed to create file %s", pFileName);
        return;
    }
    if (mFormat == MEDIA_PIX_FMT_RGB888 || mFormat == MEDIA_PIX_FMT_RGB888P) {
        fprintf(pFile, "P6\n%d %d\n255\n", mWidth, mHeight);  // ppm
    } else if (mFormat >= MEDIA_PIX_FMT_RAW8 && mFormat <= MEDIA_PIX_FMT_RAW12){
        fprintf(pFile, "P5\n%d %d\n255\n%d\n", mWidth, mHeight, (1 << mBpp) - 1);  // pgm
    }
    unsigned char* data = getBuffer() ;
    for (uint32_t j = 0; j < mHeight*mBpp/8; j++) {
        uint32_t indexd = j*mStride;
        fwrite(data + indexd, mWidth, 1, pFile);
    }

    fclose(pFile);
}


/*************MEDIA_BUFFER_TYPE_USER_PTR*************/
MediaUserBuffer::MediaUserBuffer() {
    mBuffer = NULL;
    mBufferType = MEDIA_BUFFER_TYPE_USER_PTR;
}


MediaUserBuffer::~MediaUserBuffer() {
    release();
}

bool MediaUserBuffer::create(uint32_t width, uint32_t height, uint32_t format) {
    auto iter = mapFormatBpp.find(format);
    if (iter == mapFormatBpp.end()) {
        ALOGE("can't support format %s: %d", __func__, format);
        return false;
    }

    mStride = ALIGN_UP(width, 16);
    mBpp    = iter->second;
    mSize   = mStride * height* mBpp / 8;
    mSize   = ALIGN_UP(mSize, 4096);
    mWidth  = width;
    mHeight = height;
    mFormat = format;
    // sometimes we need debug at user mode, can open here and skip Allocator
    mBuffer = new unsigned char[mSize];
    memset(mBuffer, 0, mSize);
    return true;
}


void MediaUserBuffer::release() {
    if (mBuffer) {
        delete mBuffer;
        mBuffer = NULL;
    }
}


void MediaUserBuffer::copyTo(MediaBuffer* pBuffer) {

    if (pBuffer->mStride != mStride || pBuffer->mWidth != mWidth ||
        pBuffer->mHeight != mHeight || pBuffer->mFormat != mFormat) {
        return;
    }
    memcpy(pBuffer->getBuffer(), mBuffer, mSize);
}
/*************MEDIA_BUFFER_TYPE_ADDR*************/
MediaAddrBuffer::MediaAddrBuffer() {
    mBufferType = MEDIA_BUFFER_TYPE_ADDR;
}

bool MediaAddrBuffer::create(uint32_t width, uint32_t height, uint32_t format) {
    if (width == 0 || height == 0) {
        ALOGE("fatal error, try to alloc 0 size buffer!");
        exit(1);
    }
    auto iter = mapFormatBpp.find(format);
    if (iter == mapFormatBpp.end()) {
        ALOGE("can't support format %s: %d", __func__, format);
        return false;
    }

    mStride = ALIGN_UP(width, 16);
    mBpp    = iter->second;
    mSize   = mStride * height* mBpp / 8;
	mSize	= ALIGN_UP(mSize, 4096);
    mWidth  = width;
    mHeight = height;
    mFormat = format;
    baseAddress = GALLOC(mSize);
    if (baseAddress) {
        void *mapAddr = GMAP(baseAddress, mSize);
        if (mapAddr)
            memset(mapAddr, 0, mSize);
    } else {
        ALOGE("faild to allocate buffer");
    }
    ALOGD("alloc buffer %p 0x%08x", (void*)baseAddress, mSize);
    return true;
}

bool MediaAddrBuffer::vcreate(uint32_t width, uint32_t height, uint32_t format) {
    auto iter = mapFormatBpp.find(format);
    if (iter == mapFormatBpp.end()) {
        ALOGE("can't support format %s: %d", __func__, format);
        return false;
    }
    mStride = ALIGN_UP(width, 16);
    mBpp    = iter->second;
    mSize   = mStride * height* mBpp / 8;
    mSize   = ALIGN_UP(mSize, 4096);
    mWidth  = width;
    mHeight = height;
    mFormat = format;
    baseAddress = 0;
    vFlag = true;
    return true;
}

void MediaAddrBuffer::copyTo(MediaBuffer* pBuffer) {

}

unsigned char* MediaAddrBuffer::getBuffer() {
    if (!data)
        data = GMAP(baseAddress, mSize);
    return data;
}

MediaAddrBuffer::~MediaAddrBuffer() {
    release();
}

void MediaAddrBuffer::release() {
    if (baseAddress && !vFlag) {
        ALOGD("free buffer %p 0x%08x", (void*)baseAddress, mSize);
        GFREE(baseAddress, mSize);
        baseAddress = 0;
    }

    if ((data != NULL) && vFlag)
        GUNMAP(data, mSize);

    data = NULL;
}
