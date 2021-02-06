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

#include "DewarpDriver.h"

#include <memory.h>
#include <unistd.h>
#include <unordered_map>

#include "DewarpMap.h"
#include <IMemoryAllocator.h>
#include <BufferManager.h>
#include "dwe_regs.h"
#include <log.h>

#include <sys/time.h>

#define LOGTAG "DewarpDriver"

#define IP_NAME "viv dewarp processor"
#define IP_TYPE "DW"
#define VERSION_NUMBER "100"

int DewarpDriver::open() {
    ALOGI("%s: opening dw100 driver.", __func__);
    mHardware = new DewarpHardware();

    if (!mHardware->create()) {
        ALOGE("%s: failed to create dewarp instance!", __func__);
        delete mHardware;
        mHardware = NULL;
        return -1;
    }
    mDewarpMap[0] = new unsigned int[MAX_MAP_SIZE];
    mDewarpMap[1] = new unsigned int[MAX_MAP_SIZE];
    return 0;
}

void DewarpDriver::close(int fd) {
    if (!mHardware) return;
    delete mHardware;
    mHardware = NULL;
    GFREE(dweMapAddr[0], MAX_MAP_SIZE);
    GFREE(dweMapAddr[1], MAX_MAP_SIZE);
    if (mDewarpMap[0]) {
        delete [] mDewarpMap[0];
        mDewarpMap[0] = NULL;
    }
    if (mDewarpMap[1]) {
        delete [] mDewarpMap[1];
        mDewarpMap[1] = NULL;
    }
}

bool DewarpDriver::start() {
    if (!mHardware) return false;
    mHardware->reset();
    if (mParams.pix_fmt_in == MEDIA_PIX_FMT_YUV422I)
        info.src_stride *= 2;
    if (mParams.pix_fmt_out == MEDIA_PIX_FMT_YUV422I)
        info.dst_stride *= 2;
    ALOGI("start %d %d %d %d, stride: %d %d", info.src_w, info.src_h, info.map_w,
        info.map_h, info.src_stride, info.dst_stride);
    mHardware->disableBus();
    mHardware->disableIrq();
    mHardware->setMapLutAddr(0, dweMapAddr[0]);
    mHardware->setMapLutAddr(1, dweMapAddr[1]);

    info.scale_factor = (uint32_t)((512*1024.0f / mParams.scale_factor)) & 0xffff;
    info.in_format = mParams.pix_fmt_in;
    info.out_format = mParams.pix_fmt_out;
    info.hand_shake = 0;
    info.boundary_y = mParams.boundary_pixel.Y;
    info.boundary_u = mParams.boundary_pixel.U;
    info.boundary_v = mParams.boundary_pixel.V;
    info.src_auto_shadow = info.dst_auto_shadow = 0;
    info.split_h = mParams.split_horizon_line;
    info.split_v1 = mParams.split_vertical_line_up;
    info.split_v2 = mParams.split_vertical_line_down;

    if (info.out_format == MEDIA_PIX_FMT_YUV420SP)
        info.dst_size_uv = ALIGN_UP(info.dst_stride*info.dst_h/2, DEWARP_BUFFER_ALIGNMENT);
    else
        info.dst_size_uv = ALIGN_UP(info.dst_stride*info.dst_h, DEWARP_BUFFER_ALIGNMENT);

    mHardware->setParams(&info);
    mHardware->start();
    usleep(10000);
#ifndef ENABLE_IRQ
    mRunMode = RUN;
    mStreamThread = std::thread([this](){ mainStream(); });
    mRequestThread = std::thread([this](){ processRequest(); });
#endif
    return true;
}

bool DewarpDriver::stop() {
    if (!mHardware) return false;
#ifndef ENABLE_IRQ
    int prevRunMode = mRunMode.fetch_or(STOPPING);
    if (prevRunMode == STOPPED) {
        mRunMode = STOPPED;
    } else if (prevRunMode & STOPPING) {
        return false;
    } else {
        if (mRequestThread.joinable()) {
            //frameDoneSignal.post();
            mRequestThread.join();
        }
        if (mStreamThread.joinable()) {
            mStreamThread.join();
        }
        mHardware->stop();
    }
#else
    mHardware->stop();
#endif
    mInputBufferCount = 0;
    mOutputBufferCount = 0;
    return true;
}

bool DewarpDriver::setParams(dewarp_parameters* params) {
    if (!mHardware) return false;
    if (!params) return false;
    memcpy(&mParams, params, sizeof(mParams));
    dewarp_type = params->dewarp_type;

    const uint32_t blocksize = 16;
    const uint32_t blockshift = 4;
    const uint32_t MAX_IMG_WIDTH = 4096;
    info.src_w = params->image_size.width;
    info.src_h = params->image_size.height;
    info.roi_x = params->roi_start.width;
    info.roi_y = params->roi_start.height;
    info.map_w = (ALIGN_UP(info.src_w, blocksize) >> blockshift) + 1;
    info.map_h = (ALIGN_UP(info.src_h, blocksize) >> blockshift) + 1;
    if (params->dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
        if (params->split_horizon_line > info.src_h &&
            params->split_vertical_line_up > info.src_w &&
            params->split_vertical_line_down > info.src_w) {
            // one screen
            info.dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(info.src_h * VS_PI) , 16));
            info.dst_h = ALIGN_UP(info.src_h / 2 , 8);
        } else if ((params->split_vertical_line_up > info.src_w ||
                 params->split_vertical_line_down > info.src_w) &&
                (params->split_horizon_line > blocksize && params->split_horizon_line < info.src_h)) {
            // up and down two screen or three
            info.dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(info.src_h / 2 * VS_PI) , 16));
            info.dst_h = info.src_h;
        } else if ((params->split_vertical_line_up > blocksize &&
                      params->split_vertical_line_up < info.src_w) &&
                     (params->split_vertical_line_down > blocksize &&
                     params->split_vertical_line_down < info.src_w) &&
                     (params->split_horizon_line > blocksize &&
                     params->split_horizon_line < info.src_h)) {
            // four
            info.dst_w = info.src_w;
            info.dst_h = info.src_h;
        } else if ((params->split_vertical_line_up > blocksize &&
                  params->split_vertical_line_up < info.src_w) &&
                  (params->split_vertical_line_down > blocksize &&
                   params->split_vertical_line_down < info.src_w) &&
                   params->split_vertical_line_up == params->split_vertical_line_down &&
                   params->split_horizon_line > info.src_h) {
            // left right
            info.dst_w = MIN(MAX_IMG_WIDTH, ALIGN_UP((uint32_t)(info.src_h * VS_PI) , 16));
            info.dst_h = ALIGN_UP(info.src_h / 2 , 8);
        }

        info.map_w = (ALIGN_UP(info.dst_w, blocksize) >> blockshift) + 1;
        info.map_h = (ALIGN_UP(info.dst_h, blocksize) >> blockshift) + 1;
        info.map_w++;
        info.map_h++;
    } else {
        info.roi_x = ((info.roi_x >> blockshift) << blockshift);
        info.roi_y = ((info.roi_y >> blockshift) << blockshift);

        info.dst_w = ALIGN_UP(((info.src_w - info.roi_x)*params->scale_factor) >> 12, 16);
        info.dst_h = ALIGN_UP(((info.src_h - info.roi_y)*params->scale_factor) >> 12, 8);
    }

    if (params->image_size_dst.width > 0) {
        info.dst_w = ALIGN_UP(params->image_size_dst.width, 16);
        info.dst_h = ALIGN_UP(params->image_size_dst.height, 8);
    }
    info.src_stride = ALIGN_UP(info.src_w, DEWARP_BUFFER_ALIGNMENT);
    info.dst_stride = ALIGN_UP(info.dst_w, DEWARP_BUFFER_ALIGNMENT);
    info.split_line = (params->dewarp_type == DEWARP_MODEL_SPLIT_SCREEN);
    return true;
}

void createBypassMap(unsigned int* warpMap, int widthMap, int heightMap)
{
    int y = 0;
    for (int i = 0; i < heightMap; i++, y += BLOCK_SIZE) {
        int x = 0;
        for (int j = 0; j < widthMap; j++, x += BLOCK_SIZE) {
            int dx = (x*16) & 0xffff;
            int dy = (y*16) & 0xffff;
            warpMap[i*widthMap + j] = (dy << 16) | dx;
        }
    }
}

void DewarpDriver::setMap(dewarp_distortion_map& dmap, int index) {
    if (dmap.userMapSize > 0) {
        unsigned int* mapBuffer = (unsigned int*)GMAP(dweMapAddr[index], MAX_MAP_SIZE);
        memcpy(mapBuffer, dmap.pUserMap, dmap.userMapSize*4);
        return;
    }
    float fovSet = 1.0;
    int offsetx_p = 0;
    int offsety_p = 0;
    double scalef_p = 1.0;

    computePerspectiveFront(dmap.perspective_matrix, info.src_w, info.src_h, &scalef_p, &offsetx_p, &offsety_p);
    int map_bits = 16;
    if (mParams.bypass) {
        createBypassMap(mDewarpMap[index], info.map_w, info.map_h);
    } else if (mParams.dewarp_type == DEWARP_MODEL_SPLIT_SCREEN) {
        CreateUpdateWarpPolarMap(mDewarpMap[index], info.map_w, info.map_h, map_bits, 4, info.src_w, info.src_h, info.dst_w, 
                        info.dst_h, info.src_w / 2, info.src_h / 2, info.src_h / 2,
                        mParams.split_horizon_line, mParams.split_vertical_line_up,
                        mParams.split_vertical_line_down, BLOCK_SIZE, BLOCK_SIZE, 0x20);
    } else {
        switch (dewarp_type) {
        case DEWARP_MODEL_LENS_DISTORTION_CORRECTION:
            CreateUpdateDewarpMap(mDewarpMap[index], info.map_w, info.map_h, map_bits, 4, dmap.camera_matrix, dmap.distortion_coeff,
                                        info.src_w, info.src_h, 1.0, BLOCK_SIZE, BLOCK_SIZE);
            break;
        case DEWARP_MODEL_FISHEYE_EXPAND:
            CreateUpdateFisheyeExpandMap(mDewarpMap[index], info.map_w, info.map_h, map_bits, 4, info.src_w, info.src_h, info.src_w, 
                    info.src_h, info.src_w / 2, info.src_h / 2, info.src_h / 2, BLOCK_SIZE, BLOCK_SIZE);
            break;
        case DEWARP_MODEL_FISHEYE_DEWARP:
            CreateUpdateFisheyeDewarpMap(mDewarpMap[index], info.map_w, info.map_h, map_bits, 4,  dmap.camera_matrix, dmap.distortion_coeff,
                    info.src_w, info.src_h, fovSet, BLOCK_SIZE, BLOCK_SIZE);
            break;
        }
    }

    setFlip(index, mParams.hflip, mParams.vflip);
    return;
}

void DewarpDriver::setFlip(int port, bool hflip, bool vflip) {
    mParams.hflip = hflip;
    mParams.vflip = vflip;
    unsigned int* tempBuffer = (unsigned int*)GMAP(dweMapAddr[port], MAX_MAP_SIZE);
    for (uint32_t i = 0; i < info.map_h; i++) {
        for (uint32_t j = 0; j < info.map_w; j++) {
            uint32_t x = mParams.hflip ? info.map_w-1-j : j;
            uint32_t y = mParams.vflip ? info.map_h-1-i : i;
            tempBuffer[y*info.map_w+j] = mDewarpMap[port][i*info.map_w+x];
        }
    }
}

bool DewarpDriver::setDistortionMap(dewarp_distortion_map* pMap) {
    if (!mHardware || !pMap) return false;
    if (!dweMapAddr[0])
        dweMapAddr[0] = GALLOC(MAX_MAP_SIZE);
    if (!dweMapAddr[1])
        dweMapAddr[1] = GALLOC(MAX_MAP_SIZE);

    for (int i = 0; i < 2; i++) {
        setMap(pMap[i], i);
    }
    return true;
}

bool DewarpDriver::queryCaps(dewarp_capability* caps) {
    strncpy(caps->name, IP_NAME, sizeof(caps->name));
    strncpy(caps->version_number, VERSION_NUMBER, sizeof(caps->version_number));
    strncpy(caps->type, IP_TYPE, sizeof(caps->type));
    caps->capabilities = DEWARP_MODEL_LENS_DISTORTION_CORRECTION |
                         DEWARP_MODEL_FISHEYE_EXPAND | DEWARP_MODEL_SPLIT_SCREEN;
    return true;
}

void DewarpDriver::processRequest() {
    while (mRunMode == RUN) {
        if (GQUERY(this, 1, 0) == 0 || (GQUERY(this, 0, 0) == 0 && GQUERY(this, 0, 1) == 0)) {
            usleep(1000);
            continue;
        }
        uint64_t dstAddress = GPOP(this, 1, 0);
        uint64_t srcAddress = GPOP(this, BUFFER_TYPE_INPUT_FREE, 0);
        int port = 0;
        if (srcAddress == 0) {
            srcAddress = GPOP(this, 0, 1);
            port = 1;
        }
        // IMPORTANT, relate to buffer sync.
        // if get new input buffer, write it into dwe immediately
        mInputBufferCount++;
        // ALOGI("%s: new input buffer  %d", __func__, mInputBufferCount);
        //system("sync");
        mHardware->setDstBufferAddress(info.dst_stride, info.dst_h, dstAddress);
        mHardware->setMapLutAddr(0, dweMapAddr[port]);
        mHardware->startDmaRead(info.src_stride, info.src_h, srcAddress);
        mHardware->clearIrq();
        mHardware->enableBus();

#ifdef HAL_CMODEL
        frameDoneSignal.wait();
#else
        if (!frameDoneSignal.waitMillSeconds(1000)) {
            ALOGE("%s wait dewarp framedone timeout!", __func__);
            //TODO: reset hardware and reconfig all registers/map
        }
#endif
        GPUSH(this, BUFFER_TYPE_OUTPUT_READY, port, dstAddress);
        mBufferCallback(srcAddress, dstAddress, port);
    }
}

void DewarpDriver::mainStream() {
    while (mRunMode == RUN) {
        uint32_t irqStatus = mHardware->readIrq();
        irqEvent(irqStatus);
        usleep(1000);
    }
    mRunMode = STOPPED;
    mHardware->disableBus();
    mHardware->disableIrq();
}

void DewarpDriver::irqEvent(int irqStatus) {
    // printf("RECIEVED IRQ 0x%x\n", irqStatus);
    if (irqStatus & INT_FRAME_DONE) {
        mHardware->disableBus();
        mHardware->disableIrq();
        frameDoneSignal.post();
    }
    if (irqStatus & INT_ERR_STATUS_MASK) {
        // printf("dewarp error: %d.\n",  (irqStatus & INT_ERR_STATUS_MASK) >> INT_ERR_STATUS_SHIFT);
    }
    if (irqStatus & INT_FRAME_BUSY) {
        // printf("dewarp error: frame busy.\n");
    }
}

void DewarpDriver::clear() {
}
