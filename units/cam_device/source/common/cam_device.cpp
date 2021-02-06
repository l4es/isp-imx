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

#include "cam_device_api.hpp"
#include "ispcore_holder.hpp"
#include "macros.hpp"
#include "commitf_citfapi.hpp"

using namespace camdev;


int CAM_DEVICE::getHwResources(Json::Value &jRequest, Json::Value &jResponse) {
    return CitfApi::CitfGetHwResources(jRequest, jResponse);
}

int CAM_DEVICE::getHwStatus(Json::Value &jRequest, Json::Value &jResponse) {
    return CitfApi::CitfGetHwStatus(jRequest, jResponse);
}

int CAM_DEVICE::getIspFeature(Json::Value &jRequest, Json::Value &jResponse) {
    return CitfApi::CitfGetIspFeature(jRequest, jResponse);
}

int CAM_DEVICE::getFeatureVersion(Json::Value &jRequest, Json::Value &jResponse) {
    return CitfApi::CitfGetFeatureVersion(jRequest, jResponse);
}

int CAM_DEVICE::getVersions(Json::Value &jRequest, Json::Value &jResponse) {
    return CitfApi::CitfGetVersions(jRequest, jResponse);
}

CAM_DEVICE::CAM_DEVICE() {
    TRACE(CITF_INF, "CAM_DEVICE creating %s!\n", __func__);
    isp_idx = CAM_ISPCORE_ID_MAX;
}

CAM_DEVICE::~CAM_DEVICE() {
    TRACE(CITF_INF, "CAM_DEVICE deleting %s!\n", __func__);
    if (camdev_ctx != nullptr) {
        CitfApi * ctx = (CitfApi *)camdev_ctx;
        delete(ctx);
        camdev_ctx = nullptr;
    }
}

int CAM_DEVICE::initHardware(CAM_DEVICE_ID id, void ** handle) {
    CitfApi * ctx;
    if ((id < 0)||(id >= CAM_ISPCORE_ID_MAX)||(handle == nullptr)) {
        TRACE(CITF_INF, " %s: Invalid cam_device_id\n", __func__);
        return -1;
    }

    if(nullptr != Ispcore_Holder::getInstance()->ispcore[id].ctx) {
        TRACE(CITF_ERR, "%s ispcore %d in using, handing return!\n", __func__, id);
        *handle = (void *)(Ispcore_Holder::getInstance()->ispcore[id].handle);
        return -1;
    }

    ctx = new CitfApi(id);
    if (ctx == nullptr) {
        TRACE(CITF_ERR, "%s ispcore %d create fail!\n", __func__, id);
        return -1;
    }

    ctx->CitfCreateBuffItf();
    ctx->CitfCreateEvent();

    Ispcore_Holder::getInstance()->ispcore[id].ctx = ctx;
    Ispcore_Holder::getInstance()->ispcore[id].handle = this;

    camdev_ctx = (void*)ctx;
    *handle = (void *)this;

    TRACE(CITF_INF, "%s ispcore %d created!\n", __func__, id);
    return 0;
}

int CAM_DEVICE::releaseHardware() {
    if (camdev_ctx) {
        CitfApi * ctx = (CitfApi *)camdev_ctx;
        ctx->CitfDelEvent();
        ctx->CitfDelBuffItf();
        delete(ctx);
        camdev_ctx = nullptr;
    }
    return 0;
}

int CAM_DEVICE::getSensorInfo(Json::Value &jRequest, Json::Value &jResponse) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfGetSensorInfo(jRequest, jResponse);
}

int CAM_DEVICE::ioctl(int32_t ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    int ret = ctx->CitfIoctl(ctrlId, jRequest, jResponse);
    return ret;
}

int CAM_DEVICE::initOutChain(ISPCORE_BUFIO_ID chain, uint8_t skip_interval) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfInitOutChain(chain, skip_interval);
}

int CAM_DEVICE::deInitOutChain(ISPCORE_BUFIO_ID chain) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfDeinitOutChain(chain);
}

int CAM_DEVICE::startOutChain(ISPCORE_BUFIO_ID chain) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfStartOutChain(chain);
}

int CAM_DEVICE::stopOutChain(ISPCORE_BUFIO_ID chain) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfStopOutChain(chain);
}

int CAM_DEVICE::attachChain(ISPCORE_BUFIO_ID chain) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfAttachChain(chain);
}

int CAM_DEVICE::detachChain(ISPCORE_BUFIO_ID chain) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfDetachChain(chain);
}

int CAM_DEVICE::waitForBufferEvent(ISPCORE_BUFIO_ID chain, buffCtrlEvent_t * xomCtrlEvent, int timeout) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfWaitForBufferEvent(chain, xomCtrlEvent, timeout);
}

int CAM_DEVICE::DQBUF(ISPCORE_BUFIO_ID chain, MediaBuffer_t ** pBuf) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfDQBUF(chain, pBuf);
}

int CAM_DEVICE::QBUF(ISPCORE_BUFIO_ID chain, MediaBuffer_t * pBuf) {
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfQBUF(chain, pBuf);
}


int CAM_DEVICE::initBufferPoolCtrl(ISPCORE_BUFIO_ID chain){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfInitBufferPoolCtrl(chain);
}

int CAM_DEVICE::deInitBufferPoolCtrl(ISPCORE_BUFIO_ID chain){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfDeInitBufferPoolCtrl(chain);
}

int CAM_DEVICE::setBufferParameters(BUFF_MODE buff_mode){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfSetBufferParameters(buff_mode);
}

int CAM_DEVICE::bufferPoolAddEntity(ISPCORE_BUFIO_ID chain, BufIdentity* buf){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfBufferPoolAddEntity(chain, buf);
}

int CAM_DEVICE::bufferPoolClearBufList(ISPCORE_BUFIO_ID chain){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfBufferPoolClearBufList(chain);
}

int CAM_DEVICE::bufferPoolSetBufToEngine(ISPCORE_BUFIO_ID chain){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfBufferPoolSetBufToEngine(chain);
}

int CAM_DEVICE::bufferPoolKernelAddrMap(ISPCORE_BUFIO_ID chain, uint32_t needRemap){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfBufferPoolKernelAddrMap(chain, needRemap);
}

int CAM_DEVICE::bufferPoolGetBufIdList(ISPCORE_BUFIO_ID chain, std::list<BufIdentity *> * poolList){
    CitfApi * ctx = (CitfApi *)camdev_ctx;
    return ctx->CitfBufferPoolGetBufIdList(chain, poolList);
}
