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

#include "BufferManager.h"
#include "log.h"
#define LOGTAG "BUFFERMANAGER"
BufferManager* BufferManager::mInst = NULL;
V4l2BufferManager* V4l2BufferManager::mInst = NULL;

BufferManager* BufferManager::inst() {
    if (mInst == NULL) {
        mInst = new BufferManager();
    }
    return mInst;
}

void BufferManager::push(void* owner, int type, int padid, uint64_t addr) {
    EAutoLock l(&mMutex);
    // ALOGE("## push %d %d %d %p %p", __LINE__, type, padid, owner, (unsigned char*)addr);
    qmap[type][owner][padid].push_back(addr);
}

uint64_t BufferManager::pop(void* owner, int type, int padid) {
    EAutoLock l(&mMutex);
    if (query(owner, type, padid) == 0)
        return 0;
    uint64_t ret = qmap[type][owner][padid].front();
    qmap[type][owner][padid].pop_front();
    return ret;
}

int BufferManager::query(void* owner, int type, int padid) {
    if (type >= BUFFER_TYPE_MAX || type < 0) return 0;
    auto iter1 = qmap[type].find(owner);
    if (iter1 == qmap[type].end())
        return 0;
    auto iter2 = iter1->second.find(padid);
    if (iter2 == iter1->second.end())
        return 0;
    return iter2->second.size();
}

void BufferManager::clear(void* owner) {
    for (int i = 0; i < 3; i++) {
        qmap[i][owner].clear();
    }
}

V4l2BufferManager* V4l2BufferManager::inst() {
    if (mInst == NULL) {
        mInst = new V4l2BufferManager();
    }
    return mInst;
}

// physical address map, does not need lock
void V4l2BufferManager::push(void* file, uint64_t addr) {
    qlist[addr] = file;
}

void* V4l2BufferManager::find(uint64_t addr) {
    auto item = qlist.find(addr);
    if (item == qlist.end())
        return NULL;
    return item->second;
}
