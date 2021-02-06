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

#include "buf_io.hpp"
#include "ispcore_holder.hpp"
#include <ebase/builtins.h>
#include "macros.hpp"
#include <bufferpool/media_buffer.h>
#include <vector>
#include <list>
#include "cam_common.hpp"
#include "cam_device_api.hpp"
#include <oslayer/oslayer.h>


#ifdef TRACE_CMD
#undef TRACE_CMD
#endif

#define TRACE_CMD TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__)

using namespace camdev;

Xim::Xim() {
    MimCtrlConfig_t config;
    memset(&config, 0, sizeof(config));

    config.MaxPendingCommands = 10;
    hCtrl = config.hMimContext;
    state = Idle;
}

Xim::~Xim() {
    TRACE_IN;

    stop();
    hCtrl = NULL;

    TRACE_OUT;
}

void Xim::cbCompletion(MimCtrlCmdId_t cmdId, int32_t result, void *pUserContext) {
    TRACE_IN;

    if (result != RET_SUCCESS) {
        TRACE_OUT;

        return;
    }

    DCT_ASSERT(pUserContext);

    Xim *pCtrlItf = (Xim *)pUserContext;

    int32_t ret = 0;

    switch (cmdId) {
    case MIM_CTRL_CMD_START:
        ret = osEventSignal(&pCtrlItf->eventStarted);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    case MIM_CTRL_CMD_STOP:
        ret = osEventSignal(&pCtrlItf->eventStopped);
        DCT_ASSERT(ret == OSLAYER_OK);
        break;

    default:
        break;
    }

    TRACE_OUT;
}

RESULT Xim::start(void *) {

    if (state == Running) {
        TRACE(CITF_ERR, "%s Wrong status(exit)\n", __PRETTY_FUNCTION__);
        return RET_WRONG_STATE;
    }
#if 0
    int32_t result = MimCtrlStart(hCtrl);
    DCT_ASSERT(result == RET_PENDING);
#endif
    int32_t ret = osEventWait(&eventStarted);
    DCT_ASSERT(ret == OSLAYER_OK);

    state = Running;

    TRACE_OUT;

    return RET_SUCCESS;
}

RESULT Xim::stop() {
    TRACE_IN;

    if (state == Idle) {
        TRACE_OUT;

        return RET_SUCCESS;
    }
#if 0
    int32_t result = MimCtrlStop(hCtrl);
    DCT_ASSERT(result == RET_PENDING);
#endif
    int32_t ret = osEventWait(&eventStopped);
    DCT_ASSERT(ret == OSLAYER_OK);

    state = Idle;

    TRACE_OUT;

    return RET_SUCCESS;
}


RESULT Xim::ximUsrLoadBuffer(PicBufMetaData_t *pPicBuffer, void *pContext)
{
    TRACE_IN;

    TRACE_OUT;

    return RET_SUCCESS;
}
RESULT ximCheckBufferFormat(PicBufMetaData_t *pPicBuffer)
{
    TRACE_IN;

    TRACE_OUT;

    return RET_SUCCESS;
}

RESULT Xim::ximSetParameters(uint32_t frame_cnt, PicBufMetaData_t *BufFormat)
{
    TRACE_IN;

    TRACE_OUT;

    return RET_SUCCESS;
}
/***************************Xim end*********************************/

/***************************Xom Begin*********************************/
Xom::Xom() {
    TRACE_IN;
    dma_idx = ISPCORE_BUFIO_WRITEMAX;
    state = Invalid;
    TRACE_OUT;
}

Xom::Xom(ISPCORE_BUFIO_ID idx, uint32_t eQlength, uint32_t bQlength):Xom(){
    TRACE_IN;
    dma_idx = idx;
    MaxPendingEvents = eQlength;
    MaxBuffers = bQlength;
    state = Init;
    TRACE_OUT;
}

Xom::~Xom() {
    TRACE_IN;
    dma_idx = ISPCORE_BUFIO_WRITEMAX;
    state = Invalid;
    TRACE_OUT;
}


RESULT Xom::xomInit(uint8_t skip_interval){
    FrameSkip_Max = (uint32_t)skip_interval;

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_CONFIG;
    }

    // create command queue
    if ( OSLAYER_OK != osQueueInit( &CommandQueue, MaxPendingEvents, sizeof(buffCtrlEvent_t) ) )
    {
        TRACE(CITF_ERR, "%s (creating command queue (depth: %d) failed)\n", __func__, MaxPendingEvents);
        return    RET_FAILURE;
    }

    // create full buffer queue
    if ( OSLAYER_OK != osQueueInit( &FullBufQueue, MaxBuffers, sizeof(MediaBuffer_t *) ) )
    {
            TRACE(CITF_ERR, "%s (creating buffer queue (depth: %d) failed)\n", __func__, MaxBuffers);
            osQueueDestroy( &CommandQueue );
            return RET_FAILURE;
    }

    state = Idle;
    return RET_SUCCESS;
}


RESULT Xom::xomDeInit(){

    int32_t osStatus;

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Invalid init for Xom)\n", __func__);
        return RET_WRONG_CONFIG;
    }

    if(state != Idle)
    {
        TRACE(CITF_ERR, "%s (Invalid Status)\n", __func__);
        return RET_WRONG_STATE;
    }

    do{
        // get next command from queue
        buffCtrlEvent_t event;
        osStatus = osQueueTryRead(&CommandQueue, &event );
    } while (osStatus == OSLAYER_OK);

    do{
        // get next buffer from queue
        MediaBuffer_t *pBuffer = NULL;
        osStatus = osQueueTryRead(&FullBufQueue, &pBuffer);
        if((osStatus == OSLAYER_OK)||(pBuffer != NULL))
        {
            MediaBufUnlockBuffer(pBuffer);
        }
    } while (osStatus == OSLAYER_OK);

    // destroy full buffer queue
    if ( OSLAYER_OK != osQueueDestroy( &FullBufQueue ) )
    {
            TRACE(CITF_ERR, "%s (destroying full buffer queue failed)\n", __func__);
            return RET_FAILURE;
    }

    // destroy command queue
    if ( OSLAYER_OK != osQueueDestroy( &CommandQueue ) )
    {
            TRACE(CITF_ERR, "%s (destroying command queue failed)\n", __func__);
            return RET_FAILURE;
    }

    state = Invalid;
    return RET_SUCCESS;
}


RESULT    Xom::xomStart(){
    RESULT ret = RET_SUCCESS;

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_CONFIG;
    }

    state = Running;
    FrameIdx = 0;

    buffCtrlEvent_t xomEvent;
    xomEvent.eventID = BUFF_CTRL_CMD_START;
    xomEvent.buffer_id = 0;
    xomEvent.buffer_skip_cnt = 0;
    ret = this->xomCtrlSendEvent(&xomEvent);

    return ret;
}

RESULT    Xom::xomStop(){
    RESULT ret = RET_SUCCESS;

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_CONFIG;
    }

    buffCtrlEvent_t xomEvent;
    xomEvent.eventID = BUFF_CTRL_CMD_STOP;
    xomEvent.buffer_id = 0;
    xomEvent.buffer_skip_cnt = 0;
    ret = this->xomCtrlSendEvent(&xomEvent);

    state = Idle;
    return ret;
}

void Xom::bufferCb(MediaBuffer_t *pBuffer) {

    if(pBuffer == nullptr)
    {
        TRACE(CITF_ERR, "%s (null MediaBuffer pointer input)\n", __func__);
        return;
    }

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return;
    }

    if(state != Running)
    {
        TRACE(CITF_ERR, "%s (Wrong Xom state)\n", __func__);
        return;
    }

    // prepare command
    buffCtrlEvent_t xomEvent;

    FrameIdx++;
    if((FrameIdx % (FrameSkip_Max + 1)) == 0)
    {
        //Update buffer to queue
        MediaBufLockBuffer( pBuffer );
        int32_t osStatus = osQueueTryWrite( &FullBufQueue, &pBuffer );
        if ( osStatus == OSLAYER_OK )
        {
            xomEvent.eventID = BUFF_CTRL_CMD_BUFFER_READY;
            xomEvent.buffer_id = FrameIdx;
            xomEvent.buffer_skip_cnt = FrameSkip_Max;

        }else{
            MediaBufUnlockBuffer( pBuffer );
        }
    }else{
        //return buffer to queue
        xomEvent.eventID = BUFF_CTRL_CMD_BUFFER_BYPASS;
        xomEvent.buffer_id = FrameIdx;
        xomEvent.buffer_skip_cnt = FrameSkip_Max;
    }

    RESULT ret = this->xomCtrlSendEvent(&xomEvent);
    if(ret != RET_SUCCESS) {
        TRACE(CITF_ERR, "%s (Send command error)\n", __func__);
    }
}

RESULT Xom::xomWaitForBufferEvent(buffCtrlEvent_t * xomCtrlEvent, int timeout)
{
    if(xomCtrlEvent == nullptr)
    {
        TRACE(CITF_ERR, "%s (null ctrlCmd buffer)\n", __func__);
        return RET_NULL_POINTER;
    }

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_STATE;
    }

    if(state != Running)
    {
        TRACE(CITF_ERR, "%s (Wrong Xom state)\n", __func__);
        return RET_WRONG_STATE;
    }

    /* wait pending for next command */
    int32_t osStatus = osQueueTimedRead( &CommandQueue, xomCtrlEvent, timeout);
    if (OSLAYER_OK != osStatus)
    {
            // TRACE( CITF_ERR, "%s (receiving command failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus );
            return RET_FAILURE;
    }

    return RET_SUCCESS;
}


RESULT Xom::xomDQBUF(MediaBuffer_t ** pBuf){
     if(pBuf == nullptr)
    {
        TRACE(CITF_ERR, "%s (null ctrlCmd buffer)\n", __func__);
        return RET_NULL_POINTER;
    }

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_STATE;
    }

    if(state != Running)
    {
        TRACE(CITF_ERR, "%s (Wrong Xom state)\n", __func__);
        return RET_WRONG_STATE;
    }

    int32_t osStatus = osQueueTryRead( &FullBufQueue, pBuf );
    if ( ( osStatus == OSLAYER_OK ) || ( *pBuf != NULL ) )
    {
        // TRACE(CITF_INF, "%s (DQ full buffer success fully)\n", __func__);
        return RET_SUCCESS;
    }else{
            TRACE(CITF_ERR, "%s (receiving full buffer failed -> OSLAYER_RESULT=%d)\n", __func__, osStatus);
            return RET_FAILURE;
    }
}

RESULT Xom::xomQBUF(MediaBuffer_t * pBuf){
     if(pBuf == nullptr)
    {
        TRACE(CITF_ERR, "%s (null ctrlCmd buffer)\n", __func__);
        return RET_NULL_POINTER;
    }

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_STATE;
    }

    if(state != Running)
    {
        TRACE(CITF_ERR, "%s (Wrong Xom state)\n", __func__);
        return RET_WRONG_STATE;
    }

    MediaBufUnlockBuffer( pBuf );
    return RET_SUCCESS;
}


RESULT Xom::xomCtrlSendEvent
(
        buffCtrlEvent_t                *pEvent
)
{
    if(pEvent == NULL)
    {
            TRACE(CITF_ERR, "%s command input null pointer\n", __func__);
            return RET_NULL_POINTER;
    }

    if(this->xomStatsValidCheck() != RET_SUCCESS)
    {
        TRACE(CITF_ERR, "%s (Init failed)\n", __func__);
        return RET_WRONG_CONFIG;
    }

    if(state != Running)
    {
        TRACE(CITF_ERR, "%s (Wrong Xom state)\n", __func__);
        return RET_CANCELED;
    }


    // send command
    int32_t osStatus = osQueueWrite( &CommandQueue, pEvent);
    if (osStatus != OSLAYER_OK)
    {
            TRACE(CITF_ERR, "%s (sending event to queue failed -> OSLAYER_STATUS=%d)\n", __func__, osStatus);
    }


    return ( (osStatus == OSLAYER_OK) ? RET_SUCCESS : RET_FAILURE);
}


RESULT Xom::xomStatsValidCheck(){
    if(dma_idx < ISPCORE_BUFIO_WRITEMAX)
    {
        return RET_SUCCESS;
    }else{
        return RET_WRONG_CONFIG;
    }
}

/***************************Xom End*********************************/


/***********************BuffPool begin******************************/
BuffPool::BuffPool(){
}

BuffPool::~BuffPool(){
}

RESULT BuffPool::buffPoolAddEntity(BufIdentity* buf){
    if(buf != nullptr)
    {
        buf_context.buf_list.push_back(buf);
        buf_context.frame_number ++;
    }else
    {
        return RET_NULL_POINTER;
    }

    return RET_SUCCESS;
}

RESULT BuffPool::buffPoolClearBufList()
{
    buf_context.buf_list.clear();
    buf_context.frame_number = 0;
    return RET_SUCCESS;
}

RESULT BuffPool::setBuffParameters(BUFF_MODE buff_mode){
    buf_context.buf_work_mode = buff_mode;
    if(BUFF_MODE_USRPTR == buff_mode)
    {
        buf_context.frame_number = 0;
        buf_context.buf_list.clear();

    }else if(BUFF_MODE_PHYLINEAR == buff_mode){
        buf_context.frame_number = 0;
        buf_context.buf_list.clear();
    }else{
        TRACE(CITF_ERR, "%s buff Mode %d Not supported yet\n", __func__, (int)buff_mode);
        return RET_NOTSUPP;
    }


    return RET_SUCCESS;
}

BuffPool::BufCtx *BuffPool::getBuffPrameters(){
    return &buf_context;
}

RESULT BuffPool::bufferPoolKernelAddrMap(uint32_t needRemap ){
    //default set to false, no need to remap to kernel

    return RET_SUCCESS;
}

std::list<BufIdentity *> * BuffPool::getBuffPoolList(){
    return &buf_context.buf_list;
}

/***********************BuffPool end******************************/

/***********************Bitf begin******************************/
Bitf::Bitf(){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    for (int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
    {
        bufio_ctrl[i].io.pXom = nullptr;
        bufio_ctrl[i].io.pXim = nullptr;
        bufio_ctrl[i].pBufPool = nullptr;
        bufio_ctrl[i].state = CamStatus::Init;
        //before BuffPoll, set to Running
        //bufio_ctrl[i].state = CamStatus::Running;
    }
    buf_work_mode = BUFF_MODE_INVALID;
}

Bitf::~Bitf(){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    for (int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
    {
        if(i < ISPCORE_BUFIO_WRITEMAX)
        {
            if(bufio_ctrl[i].io.pXom != nullptr)
            {
                delete(bufio_ctrl[i].io.pXom);
                bufio_ctrl[i].io.pXom = nullptr;
            }
        }

        if(i == ISPCORE_BUFIO_READ)
        {
            if(bufio_ctrl[i].io.pXim != nullptr)
            {
                delete(bufio_ctrl[i].io.pXim);
                bufio_ctrl[i].io.pXim = nullptr;
            }
        }

        if(bufio_ctrl[i].pBufPool != nullptr)
        {
            delete(bufio_ctrl[i].pBufPool);
            bufio_ctrl[i].pBufPool = nullptr;
        }
        bufio_ctrl[i].state = CamStatus::Invalid;
    }
}

RESULT Bitf::bitfSetOutputCtrl(ISPCORE_BUFIO_ID id, Xom * pXom){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    if(id >= ISPCORE_BUFIO_WRITEMAX)
    {
        TRACE(CITF_ERR, " %s Wrong ID channel\n", __PRETTY_FUNCTION__);
        return RET_INVALID_PARM;
    }

    if(pXom != nullptr)
    {
        if( 0) // (bufio_ctrl[id].state != CamStatus::Running))
        {
            TRACE(CITF_ERR, " %s Initialize BuffPool firstly, current bio_state:%d\n", __PRETTY_FUNCTION__, bufio_ctrl[id].state);
            return RET_WRONG_STATE;
        }
    }

    bufio_ctrl[id].io.pXom = pXom;
    return RET_SUCCESS;
}

Xom* Bitf::bitfGetOutputCtrl(ISPCORE_BUFIO_ID id){
    // TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    if(id >= ISPCORE_BUFIO_WRITEMAX)
    {
        TRACE(CITF_ERR, " %s Wrong ID channel\n", __PRETTY_FUNCTION__);
        return nullptr;
    }

    return bufio_ctrl[id].io.pXom;
}

RESULT Bitf::bitfSetInputCtrl(Xim * pXim){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    if(pXim != nullptr)
    {
        if((bufio_ctrl[ISPCORE_BUFIO_READ].state != CamStatus::Running))
        {
            TRACE(CITF_ERR, " %s Initialize BuffPool firstly\n", __PRETTY_FUNCTION__);
            return RET_WRONG_STATE;
        }
    }

    bufio_ctrl[ISPCORE_BUFIO_READ].io.pXim = pXim;

    return RET_SUCCESS;
}

Xim* Bitf::bitfGetInputCtrl(){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    return bufio_ctrl[ISPCORE_BUFIO_READ].io.pXim;;
}

RESULT Bitf::bitfSetBufferPoolCtrl(ISPCORE_BUFIO_ID id, BuffPool * pBufPool){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);

    if((id < ISPCORE_BUFIO_WRITEMAX) ||(id == ISPCORE_BUFIO_READ))
    {
        if(nullptr != bufio_ctrl[id].pBufPool)
        {
            TRACE(CITF_ERR, " %s BuffPool already initialized\n", __PRETTY_FUNCTION__);
            return RET_WRONG_STATE;
        }

        bufio_ctrl[id].pBufPool = pBufPool;
        bufio_ctrl[id].state = (pBufPool == nullptr)?(CamStatus::Running):(CamStatus::Init);
    }else{
        TRACE(CITF_ERR, " %s Wrong ID channel\n", __PRETTY_FUNCTION__);
        return RET_INVALID_PARM;
    }
    return RET_SUCCESS;
}

BuffPool* Bitf::bitfGetBufferPoolCtrl(ISPCORE_BUFIO_ID id){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);

 if((id < ISPCORE_BUFIO_WRITEMAX) ||(id == ISPCORE_BUFIO_READ))
    {
        return bufio_ctrl[id].pBufPool;
    }else{
        TRACE(CITF_ERR, " %s Wrong ID channel\n", __PRETTY_FUNCTION__);
        return nullptr;
    }
}

RESULT Bitf::bitfCheckBufSizeMax(uint32_t buf_size, ISPCORE_BUFIO_ID chain){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);
    uint32_t max_size = 0;

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        switch(chain){
            case ISPCORE_BUFIO_MP:
                    max_size = PIC_BUFFER_SIZE_MAIN_SENSOR;
                break;
            case ISPCORE_BUFIO_SP1:
                    max_size = PIC_BUFFER_SIZE_SELF1_SENSOR;
                break;
            case ISPCORE_BUFIO_SP2:
                    max_size = PIC_BUFFER_SIZE_SELF2_SENSOR;
                break;
            case ISPCORE_BUFIO_RDI:
                    max_size = PIC_BUFFER_SIZE_RDI_SENSOR;
                break;
            case ISPCORE_BUFIO_META:
                    max_size = PIC_BUFFER_SIZE_META_SENSOR;
                break;
            case ISPCORE_BUFIO_READ:
                    max_size = PIC_BUFFER_SIZE_INPUT;
                break;
            default:
                TRACE(CITF_ERR, " %s INPUT chain %d error\n", __PRETTY_FUNCTION__, chain);
                return RET_INVALID_PARM;
        }
        if(buf_size > max_size){
            TRACE(CITF_ERR, " %s INPUT chain %d, buffer Size:%d, max Size:%d\n", __PRETTY_FUNCTION__, chain, buf_size, max_size);
            return RET_OUTOFRANGE;
        }else{
            return RET_SUCCESS;
        }
    }else{
        TRACE(CITF_ERR, " %s INPUT chain %d error\n", __PRETTY_FUNCTION__, chain);
        return RET_INVALID_PARM;
    }
}

RESULT Bitf::bitfCheckBufNumberMax(uint32_t buf_num, ISPCORE_BUFIO_ID chain){
    TRACE(CITF_INF, " %s\n", __PRETTY_FUNCTION__);

    uint32_t max_num = 0;

    if((chain <= ISPCORE_BUFIO_WRITEMAX)||(chain == ISPCORE_BUFIO_READ))
    {
        switch(chain){
            case ISPCORE_BUFIO_MP:
                    max_num = PIC_BUFFER_NUM_MAIN_SENSOR;
                break;
            case ISPCORE_BUFIO_SP1:
                    max_num = PIC_BUFFER_NUM_SELF1_SENSOR;
                break;
            case ISPCORE_BUFIO_SP2:
                    max_num = PIC_BUFFER_NUM_SELF2_SENSOR;
                break;
            case ISPCORE_BUFIO_RDI:
                    max_num = PIC_BUFFER_NUM_RDI_SENSOR;
                break;
            case ISPCORE_BUFIO_META:
                    max_num = PIC_BUFFER_NUM_META_SENSOR;
                break;
            case ISPCORE_BUFIO_READ:
                    max_num = PIC_BUFFER_NUM_INPUT;
                break;
            default:
                TRACE(CITF_ERR, " %s INPUT chain %d error\n", __PRETTY_FUNCTION__, chain);
                return RET_INVALID_PARM;
        }
        if(buf_num > max_num){
            TRACE(CITF_ERR, " %s INPUT chain %d, buffer Size:%d, max Size:%d\n", __PRETTY_FUNCTION__, chain, buf_num, max_num);
            return RET_OUTOFRANGE;
        }else{
            return RET_SUCCESS;
        }
    }else{
        TRACE(CITF_ERR, " %s INPUT chain %d error\n", __PRETTY_FUNCTION__, chain);
        return RET_INVALID_PARM;
    }


}


#if 0
RESULT Bitf::connectOmBufToChain(std::list<ItfBufferCb *> chain, ItfBufferCb* omBuf){

    return RET_SUCCESS;
}

RESULT Bitf::removeOmBufFromChain(std::list<ItfBufferCb*> chain, ItfBufferCb* omBuf){
    return RET_SUCCESS;
}

RESULT Bitf::clearBufChain(std::list<ItfBufferCb*> chain){
    return RET_SUCCESS;
}
#endif

/***********************Bitf end******************************/
