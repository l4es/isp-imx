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

#ifndef DEVELOPER_MEDIACONTROL_BUFFER_BUFFERMANAGER_H_
#define DEVELOPER_MEDIACONTROL_BUFFER_BUFFERMANAGER_H_

#include <deque>
#include <map>
#include <EAutoLock.h>

enum {
    BUFFER_TYPE_INPUT_FREE = 0,
    BUFFER_TYPE_OUTPUT_FREE = 1,
    BUFFER_TYPE_OUTPUT_READY = 2,
    BUFFER_TYPE_MAX = 3
};

class BufferManager {
 public:
    BufferManager() {}
    ~BufferManager() {}
    static BufferManager* inst();
    void push(void* owner, int type, int padid, uint64_t addr);
    uint64_t pop(void* owner, int type, int padid);
    int query(void* owner, int type, int padid);
    void clear(void *owner);
 private:
    std::map<void*, std::map<int, std::deque<uint64_t>>> qmap[3];  // padid, owner, address; inoput/output free/output ready
    static BufferManager* mInst;
    EMutex mMutex;
};

#define GPUSH(a, b, c, d) BufferManager::inst()->push((a), (b), (c), (d))
#define GPOP(a, b, c) BufferManager::inst()->pop((a), (b), (c))
#define GQUERY(a, b, c) BufferManager::inst()->query((a), (b), (c))
#define GCLEAR(x) BufferManager::inst()->clear((x))

class V4l2BufferManager {
 public:
    V4l2BufferManager() {}
    ~V4l2BufferManager() {}
    static V4l2BufferManager* inst();
    void push(void* file, uint64_t addr);
    void* find(uint64_t addr);

 private:
    std::map<uint64_t, void*> qlist;
    static V4l2BufferManager* mInst;
    EMutex mutex;
};

#endif  // DEVELOPER_MEDIACONTROL_BUFFER_BUFFERMANAGER_H_
