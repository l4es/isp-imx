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

#ifndef DEVELOPER_MEDIACONTROL_BUFFER_IMEMORYALLOCATOR_H_
#define DEVELOPER_MEDIACONTROL_BUFFER_IMEMORYALLOCATOR_H_

#include <stdint.h>
#include <set>
#include <EAutoLock.h>

// 64 bit buffer address
typedef struct mem_info {
    uint64_t addr;
    uint64_t size;
} mem_info;

enum {
    ALLOCATOR_TYPE_USER_PTR,
    ALLOCATOR_TYPE_V4L2,
    ALLOCATOR_TYPE_DIRECT_MAP,
};

class IMemoryAllocator {
 public:
    virtual ~IMemoryAllocator() { }

    // success: return physical/virtual address
    // failed:  return 0
    virtual uint64_t alloc(uint64_t size);

    // check size at this buffer.
    virtual bool free(uint64_t addr, uint64_t size);

    // map pa to va
    virtual unsigned char* map(uint64_t addr, uint64_t size) { return NULL; }
    virtual void unmap(unsigned char* addr, uint64_t size) { return; }

    // create global memory allocator
    // cmodel - UserAllocator
    // v4l2   - V4l2Allocator
    // no cmodel, no v4l2 - DirectAllocator
    static bool create(int type);

    static IMemoryAllocator* inst();

 private:
    static IMemoryAllocator* mInst;

 protected:
    uint64_t mTotalSize = 0;
    uint64_t mAddr = 0;
    std::set<mem_info> mList;
    EMutex mMutex;
};

class V4l2Allocator : public IMemoryAllocator {
 public:
    V4l2Allocator();
    ~V4l2Allocator();
    virtual uint64_t alloc(uint64_t size);
    virtual bool free(uint64_t addr, uint64_t size);
    virtual unsigned char* map(uint64_t addr, uint64_t size);
    virtual void unmap(unsigned char* addr, uint64_t size);
 private:
   // int mDeviceId; // error: private field 'mDeviceId' is not used [-Werror,-Wunused-private-field]
};

class UserAllocator : public IMemoryAllocator {
 public:
    UserAllocator();
    ~UserAllocator();
    virtual unsigned char* map(uint64_t addr, uint64_t size);
    virtual void unmap(unsigned char* addr, uint64_t size);

 private:
    unsigned char* mData;
};

class DirectAllocator : public IMemoryAllocator {
 public:
    DirectAllocator();
    ~DirectAllocator();
    virtual unsigned char* map(uint64_t addr, uint64_t size);
    virtual void unmap(unsigned char* addr, uint64_t size);

 private:
    //unsigned char* mData;
};

#define GALLOC(x)  IMemoryAllocator::inst()->alloc(x)
#define GFREE(x, y)  IMemoryAllocator::inst()->free(x, y)
#define GMAP(x, y) IMemoryAllocator::inst()->map(x, y)
#define GUNMAP(x, y) IMemoryAllocator::inst()->unmap(x, y)

#endif  // DEVELOPER_MEDIACONTROL_BUFFER_IMEMORYALLOCATOR_H_
