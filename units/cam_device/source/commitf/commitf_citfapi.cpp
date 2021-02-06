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
#include "commitf_citfapi.hpp"
#include "exception.hpp"
#include "macros.hpp"
#include "commitf_features.hpp"

#include "commitf_sensor.hpp"

#include <algorithm>

#include "commitf_event.hpp"
#include <cam_engine/cam_engine_api.h>
#include "cam_common.hpp"

using namespace camdev;

CitfApi::CitfApi(int ispID) {
    TRACE(CITF_INF, "CommItf creating %s, ispID:%d!\n", __func__, ispID);
    if (!ctifHandle.pHalHolder) {
    	ctifHandle.pHalHolder = new HalHolder(ispID);
	}

    list.push_back(new CitfAe(&ctifHandle));
    list.push_back(new CitfAf(&ctifHandle));
    list.push_back(new CitfAvs(&ctifHandle));
    list.push_back(new CitfAwb(&ctifHandle));
    list.push_back(new CitfBls(&ctifHandle));
    list.push_back(new CitfCac(&ctifHandle));
    list.push_back(new CitfCproc(&ctifHandle));
    list.push_back(new CitfDemosaic(&ctifHandle));
    list.push_back(new CitfDevice(&ctifHandle));
    list.push_back(new Citf2Dnr(&ctifHandle));
    list.push_back(new Citf3Dnr(&ctifHandle));
    list.push_back(new CitfDpcc(&ctifHandle));
    list.push_back(new CitfDpf(&ctifHandle));
    list.push_back(new CitfEc(&ctifHandle));
    list.push_back(new CitfEe(&ctifHandle));
    list.push_back(new CitfFileSystem(&ctifHandle));
    list.push_back(new CitfHdr(&ctifHandle));
    list.push_back(new CitfIe(&ctifHandle));
    list.push_back(new CitfLsc(&ctifHandle));
    list.push_back(new CitfReg(&ctifHandle));
    list.push_back(new CitfSensor(&ctifHandle));
    list.push_back(new CitfSimp(&ctifHandle));
    list.push_back(new CitfWdr(&ctifHandle));
    list.push_back(new CitfGc(&ctifHandle));
    list.push_back(new CitfFilter(&ctifHandle));
    list.push_back(new CitfCnr(&ctifHandle));
    list.push_back(new CitfPipeline(&ctifHandle));

    TRACE(CITF_INF, "CommItf create done %s!\n", __func__);
}

CitfApi::~CitfApi() {
    TRACE(CITF_INF, "CommItf release done %s!\n", __func__);
    for (auto item : list) {
        if (item) {
            delete item;
        }
    }
    list.clear();
    delete(ctifHandle.pHalHolder);
    TRACE(CITF_INF, "CommItf release done %s!\n", __func__);
}

RESULT CitfApi::CitfGetHwResources(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);
    jResponse["resource"] = CamEngineGetHwResources();
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}


RESULT CitfApi::CitfGetHwStatus(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);
    jResponse["status"] = CamEngineGetIspStatus();
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}

RESULT CitfApi::CitfGetIspFeature(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);
    jResponse["feature"] = CamEngineGetIspFeatrue();
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}

RESULT CitfApi::CitfGetFeatureVersion(Json::Value &jRequest, Json::Value &jResponse) {
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);
    jResponse["feature_version"] = CamEngineGetFeatureVersion();
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}

RESULT CitfApi::CitfGetVersions(Json::Value &jRequest, Json::Value &jResponse)
{
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);
    jResponse["native_api_version"] = "tbd";
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}

RESULT CitfApi::CitfGetSensorInfo(Json::Value &jRequest, Json::Value &jResopnse) {
    TRACE(CITF_INF, "CommItf %s! IN\n", __func__);

    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);

    return RET_SUCCESS;
}

RESULT CitfApi::CitfIoctl(int ctrlId, Json::Value &jRequest, Json::Value &jResponse) {
    int32_t hit = 0;
    RESULT ret = RET_FAILURE;
    for (auto pCitf : list) {
        if (!pCitf) continue;
        if(hit != false) {
            TRACE(CITF_INF, "CommItf %s duplicate process!\n", __func__);
            continue;
        }
        if(pCitf->checkIsSubId((int32_t)ctrlId) != RET_SUCCESS) {
            continue;
        }
        hit = true;
        jResponse.clear();
        ret = pCitf->process(ctrlId, jRequest, jResponse);
        break;
    }

    return ret;
}

Bitf &CitfApi::CitfCreateBuffItf() {
    TRACE(CITF_INF, "CommItf %s! IN, create Bitf\n", __func__);
    if(pBitf != nullptr)
    {
         TRACE(CITF_INF, "CommItf Bitf already exist\n", __func__);
         return *pBitf;
    }

    pBitf = new Bitf();
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return *pBitf;
}

RESULT CitfApi::CitfDelBuffItf() {
    TRACE(CITF_INF, "CommItf %s! IN, recycle Bitf\n", __func__);
    if(pBitf != nullptr)
    {
        delete(pBitf);
        pBitf = nullptr;
    }
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}


CitfEvent &CitfApi::CitfCreateEvent() {
    TRACE(CITF_INF, "CommItf %s! IN, create Bitf\n", __func__);
    if(pEvent != nullptr)
    {
        TRACE(CITF_INF, "CommItf CitfEvent already exist\n", __func__);
        return *pEvent;
    }
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    pEvent = new CitfEvent();

    return *pEvent;
}

RESULT CitfApi::CitfDelEvent() {
    TRACE(CITF_INF, "CommItf %s! IN, recycle Bitf\n", __func__);
    if(pEvent != nullptr)
    {
        delete(pEvent);
        pEvent = nullptr;
    }
    TRACE(CITF_INF, "CommItf %s OUT!\n", __func__);
    return RET_SUCCESS;
}


RESULT CitfApi::CitfInitOutChain(ISPCORE_BUFIO_ID chain, uint8_t skip_interval){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d, frame_skip:%d\n", __func__, chain, skip_interval);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    Xom * pXom = new Xom(chain, 10, 10);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Xom null!\n", __func__);
        return RET_NULL_POINTER;
    }

    ret = pXom->xomInit(skip_interval);
    if(ret != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s: Xom init error!\n", __func__);
        delete(pXom);
        return RET_FAILURE ;
    }

    ret = pBitf->bitfSetOutputCtrl(chain, pXom);
    return ret;
}

RESULT CitfApi::CitfDeinitOutChain(ISPCORE_BUFIO_ID chain)
{
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }


    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_INF, "%s: Chain %dd output already deinited!\n", __func__);
        return RET_SUCCESS;
    }

    pXom->xomDeInit();
    pXom = nullptr;
    ret = pBitf->bitfSetOutputCtrl(chain, pXom);
    return ret;
}


RESULT CitfApi::CitfStartOutChain(ISPCORE_BUFIO_ID chain){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }


    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
        return RET_WRONG_STATE;
    }

    ret = pXom->xomStart();
    return ret;
}

RESULT CitfApi::CitfStopOutChain(ISPCORE_BUFIO_ID chain){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
        return RET_WRONG_STATE;
    }

    ret = pXom->xomStop();
    return ret;
}

RESULT CitfApi::CitfAttachChain(ISPCORE_BUFIO_ID chain){
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if((ctifHandle.pOperation == nullptr)||(ctifHandle.pOperation->pBufferCbContext == nullptr))
        {
            TRACE(CITF_ERR, "%s: init Operation firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
        if(pXom == nullptr)
        {
            TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__, chain);
            return RET_WRONG_STATE;
        }

        if( ISPCORE_BUFIO_MP == chain ){
            ctifHandle.pOperation->pBufferCbContext->mainPath.push_back(pXom);
        }else if(ISPCORE_BUFIO_SP1 == chain){
            ctifHandle.pOperation->pBufferCbContext->selfPath1.push_back(pXom);
        }else if(ISPCORE_BUFIO_SP2 == chain){
            ctifHandle.pOperation->pBufferCbContext->selfPath2.push_back(pXom);
        }else if(ISPCORE_BUFIO_RDI == chain){
            ctifHandle.pOperation->pBufferCbContext->rdiPath.push_back(pXom);
        }else if(ISPCORE_BUFIO_META == chain){
            ctifHandle.pOperation->pBufferCbContext->metaPath.push_back(pXom);
        }else{
            TRACE(CITF_ERR, "%s: chain %d not support!\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }
        return RET_SUCCESS;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }
}

RESULT CitfApi::CitfDetachChain(ISPCORE_BUFIO_ID chain){
 TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if((ctifHandle.pOperation == nullptr)||(ctifHandle.pOperation->pBufferCbContext == nullptr))
        {
            TRACE(CITF_ERR, "%s: init Operation firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
        if(pXom == nullptr)
        {
            TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
            return RET_WRONG_STATE;
        }

        if( ISPCORE_BUFIO_MP == chain ){
            ctifHandle.pOperation->pBufferCbContext->mainPath.clear();
        }else if(ISPCORE_BUFIO_SP1 == chain){
            ctifHandle.pOperation->pBufferCbContext->selfPath1.clear();
        }else if(ISPCORE_BUFIO_SP2 == chain){
            ctifHandle.pOperation->pBufferCbContext->selfPath2.clear();
        }else if(ISPCORE_BUFIO_RDI == chain){
            ctifHandle.pOperation->pBufferCbContext->rdiPath.clear();
        }else if(ISPCORE_BUFIO_META == chain){
            ctifHandle.pOperation->pBufferCbContext->metaPath.clear();
        }else{
            TRACE(CITF_ERR, "%s: chain %d not support!\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }
        return RET_SUCCESS;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }
}

RESULT CitfApi::CitfWaitForBufferEvent(ISPCORE_BUFIO_ID chain, buffCtrlEvent_t * xomCtrlEvent, int timeout)
{
    RESULT ret = RET_SUCCESS;
    // TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
        return RET_WRONG_STATE;
    }

    ret = pXom->xomWaitForBufferEvent(xomCtrlEvent, timeout);
    return ret;
}


RESULT CitfApi::CitfDQBUF(ISPCORE_BUFIO_ID chain, MediaBuffer_t ** pBuf){
    RESULT ret = RET_SUCCESS;
    // TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
        return RET_WRONG_STATE;
    }

    ret = pXom->xomDQBUF(pBuf);
    return ret;
}

RESULT CitfApi::CitfQBUF(ISPCORE_BUFIO_ID chain, MediaBuffer_t * pBuf){
    RESULT ret = RET_SUCCESS;
    // TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    Xom * pXom = pBitf->bitfGetOutputCtrl(chain);
    if(pXom == nullptr)
    {
        TRACE(CITF_ERR, "%s: Chain %d not initialized\n", __func__);
        return RET_WRONG_STATE;
    }

    ret = pXom->xomQBUF(pBuf);
    return ret;
}

RESULT CitfApi::CitfInitBufferPoolCtrl(ISPCORE_BUFIO_ID chain){
    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        RESULT ret;
        BuffPool* pBuffPool = new BuffPool();
        if (pBitf->buf_work_mode == BUFF_MODE_USRPTR)
            pBuffPool->setBuffParameters(pBitf->buf_work_mode);
        ret = pBitf->bitfSetBufferPoolCtrl(chain, pBuffPool);
        return ret;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }
}

RESULT CitfApi::CitfDeInitBufferPoolCtrl(ISPCORE_BUFIO_ID chain){
    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        RESULT ret;
        BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
        if(pBuffPool != nullptr)
        {
            delete(pBuffPool);
        }
        pBuffPool = nullptr;
        ret = pBitf->bitfSetBufferPoolCtrl(chain, pBuffPool);
        return ret;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }
}


RESULT CitfApi::CitfSetBufferParameters(BUFF_MODE buff_mode){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, buffMode:%d\n", __func__, buff_mode);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    if(buff_mode >= BUFF_MODE_NAX)
    {
        TRACE(CITF_ERR, "%s: BUFF_MODE %d input error!\n", __func__, buff_mode);
        return RET_WRONG_STATE;
    }

    pBitf->buf_work_mode = buff_mode;
    return ret;
}

RESULT CitfApi::CitfBufferPoolAddEntity(ISPCORE_BUFIO_ID chain, BufIdentity* buf){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if((pBitf == nullptr)||(buf == nullptr))
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly, or buf point is nullptr!\n", __func__);
        return RET_NULL_POINTER;
    }

    BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
    if(pBuffPool == nullptr)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffPool is null \n", __func__, chain);
        return RET_NULL_POINTER;
    }

    if(pBitf->buf_work_mode >= BUFF_MODE_NAX)
    {
        TRACE(CITF_ERR, "%s: BUFF_MODE %d set not proper!\n", __func__, pBitf->buf_work_mode);
        return RET_WRONG_STATE;
    }

    ret = Bitf::bitfCheckBufSizeMax(buf->buff_size, chain);
    if(ret != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffer size check failed\n", __func__, chain);
        return ret;
    }

    ret = pBuffPool->buffPoolAddEntity(buf);

    if(ret != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffPoolAddEntity error \n", __func__, chain);
        return RET_FAILURE;
    }

    return ret;
}

RESULT CitfApi::CitfBufferPoolClearBufList(ISPCORE_BUFIO_ID chain){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
    if(pBuffPool == nullptr)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffPool is null \n", __func__, chain);
        return RET_NULL_POINTER;
    }

    ret = pBuffPool->buffPoolClearBufList();

    if(ret != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffPoolClearBufList error \n", __func__, chain);
        return RET_FAILURE;
    }

    return ret;
}

RESULT CitfApi::CitfBufferPoolSetBufToEngine(ISPCORE_BUFIO_ID chain){
    RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(ctifHandle.pHalHolder == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Hal firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        //for loop for kernel addr map

        BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
        if(pBuffPool == nullptr)
        {
            TRACE(CITF_ERR, "%s!, chain ID:%d, buffPool is null \n", __func__, chain);
            return RET_NULL_POINTER;
        }

        ret = HalSetBuffMode(ctifHandle.pHalHolder->hHal, pBitf->buf_work_mode);
        if(RET_SUCCESS != ret)
        {
            TRACE(CITF_ERR, "%s!, chain ID:%d, HalSetBuffMode return error \n", __func__, chain);
            return RET_FAILURE;
        }

        uint32_t buf_idx = 0;
        uint32_t buf_max = (ISPCORE_BUFIO_READ == chain)?(BUFF_POOL_MAX_INPUT_BUF_NUMBER):(BUFF_POOL_MAX_OUTPUT_BUF_NUMBER);
        BufIdentity * pListHead =  HalGetPoolList(ctifHandle.pHalHolder->hHal, chain);
        uint32_t * buf_num = HalGetPoolNumHandle(ctifHandle.pHalHolder->hHal, chain);

        if ((nullptr == pListHead)||(nullptr == buf_num))
        {
            TRACE(CITF_ERR, "%s!, chain ID:%d input error \n", __func__, chain);
            return RET_WRONG_STATE;
        }

        //pBuffPool->buf_context.buf_list
        std::for_each(pBuffPool->buf_context.buf_list.begin(), pBuffPool->buf_context.buf_list.end(),
            [&](BufIdentity * pbufId) {

            if(buf_idx >= buf_max)
            {
                    TRACE(CITF_ERR, "%s!, chain ID:%d input error \n", __func__, chain);
                    return RET_OUTOFRANGE;
            }

            pbufId->filled = 1;
            pbufId->buffer_idx = buf_idx;
            *(pListHead + buf_idx) = *pbufId;

            buf_idx ++;
            return RET_SUCCESS;
        });

		TRACE(CITF_ERR, "%s!, chain ID:%d buffer counter number , buf_context:%d, set to Engine:%d \n",
		    __func__, chain, buf_idx, pBuffPool->buf_context.frame_number);
        *buf_num = buf_idx;
        if( buf_idx != pBuffPool->buf_context.frame_number )
        {
            TRACE(CITF_ERR, "%s!, chain ID:%d buffer counter number error, buf_context:%d, set to Engine:%d \n",
                __func__, chain, buf_idx, pBuffPool->buf_context.frame_number);
            return RET_WRONG_STATE;
        }

        ret = Bitf::bitfCheckBufNumberMax(*buf_num, chain);

        return ret;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }

    return ret;
}

RESULT CitfApi::CitfBufferPoolKernelAddrMap(ISPCORE_BUFIO_ID chain, uint32_t needRemap){
       RESULT ret = RET_SUCCESS;
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        if(pBitf == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(ctifHandle.pHalHolder == nullptr)
        {
            TRACE(CITF_ERR, "%s: init Hal firstly!\n", __func__);
            return RET_WRONG_STATE;
        }

        if(chain == ISPCORE_BUFIO_READ)
        {
            TRACE(CITF_ERR, "%s: do not support in current stage %d\n", __func__, chain);
            return RET_NOTAVAILABLE;
        }

        //for loop for kernel addr map

        BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
        if(pBuffPool == nullptr)
        {
            TRACE(CITF_ERR, "%s!, chain ID:%d, buffPool is null \n", __func__, chain);
            return RET_NULL_POINTER;
        }

        //pBuffPool->buf_context.buf_list
        std::for_each(pBuffPool->buf_context.buf_list.begin(), pBuffPool->buf_context.buf_list.end(),
            [&](BufIdentity * pbufId) {

            ret =  HalKernelAddrMap(ctifHandle.pHalHolder->hHal , pbufId);
            if(ret != RET_SUCCESS)
            {
                TRACE(CITF_ERR, "%s!, chain ID:%d, HalKernelAddrMap error \n", __func__, chain);
                return RET_FAILURE;
            }
            return RET_SUCCESS;
        });

        return RET_SUCCESS;
    }else{
        TRACE(CITF_ERR, "%s: input chain %d out of region!\n", __func__, chain);
        return RET_INVALID_PARM;
    }
    return ret;
}

RESULT CitfApi::CitfBufferPoolGetBufIdList(ISPCORE_BUFIO_ID chain, std::list<BufIdentity *> * poolList){
    TRACE(CITF_INF, "%s!, chain ID:%d\n", __func__, chain);

    if(pBitf == nullptr)
    {
        TRACE(CITF_ERR, "%s: init Bitf firstly!\n", __func__);
        return RET_WRONG_STATE;
    }

    BuffPool* pBuffPool = pBitf->bitfGetBufferPoolCtrl(chain);
    if(pBuffPool == nullptr)
    {
        TRACE(CITF_ERR, "%s!, chain ID:%d, buffPool is null \n", __func__, chain);
        return RET_NULL_POINTER;
    }

    poolList = pBuffPool->getBuffPoolList();

    return RET_SUCCESS;
}





