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

#include <unistd.h>
#include <memory.h>
#include <algorithm>
#include <IMemoryAllocator.h>
#include "FpgaDriver.h"
#include "V4l2File.h"

#include <log.h>
#define LOGTAG "IMemoryAllocator"

IMemoryAllocator* IMemoryAllocator::mInst = NULL;

static bool operator < (const mem_info& lmi, const mem_info& rmi) { return lmi.addr < rmi.addr; }
static bool operator==(const mem_info& lmi, const mem_info& rmi) { return lmi.addr == rmi.addr && lmi.size == rmi.size; }

bool IMemoryAllocator::create(int type) {
    if (mInst)
        return false;
    switch (type) {
        case ALLOCATOR_TYPE_USER_PTR:
            mInst = new UserAllocator();
            break;
        case ALLOCATOR_TYPE_V4L2:
            mInst = new V4l2Allocator();
            break;
        case ALLOCATOR_TYPE_DIRECT_MAP:
            mInst = new DirectAllocator();
            break;
        default:
            return false;
    }
    return true;
}

IMemoryAllocator* IMemoryAllocator::inst() {
    return mInst;
}

uint64_t IMemoryAllocator::alloc(uint64_t size) {
    EAutoLock l(&mMutex);
    if (mList.empty()) {
        mem_info mi = {mAddr, size};
        mList.insert(mi);
        return mAddr;
    }
    auto iter = mList.begin();
    if (iter->addr - mAddr >= size) {
        mem_info mi = {mAddr, size};
        mList.insert(mi);
        return mAddr;
    }
    for (; iter != mList.end(); ) {
        uint64_t laddr = iter->addr + iter->size;
        iter++;
        uint64_t raddr = (iter ==  mList.end() ) ? mAddr + mTotalSize : iter->addr;
        if ((raddr - laddr) >= size) {
            mem_info mi = {laddr, size};
            mList.insert(mi);
            return laddr;
        }
    }
    return 0;
}

bool IMemoryAllocator::free(uint64_t addr, uint64_t size) {
    EAutoLock l(&mMutex);
    mem_info info = {addr, size};
    auto iter = find(mList.begin(), mList.end(), info);
    if (iter == mList.end()) {
        ALOGE("failed to free buffer 0x%lx 0x%lx", addr, size);
        return false;
    }
    mList.erase(iter);
    return true;
}

UserAllocator::UserAllocator() {
    mTotalSize = 1024*1024*256;
    mData = new unsigned char[mTotalSize];
    mAddr = (uint64_t)(uintptr_t)mData;
}

UserAllocator::~UserAllocator() {
    delete mData;
}

unsigned char* UserAllocator::map(uint64_t addr, uint64_t size) {
    // user ptr no need to map buffer.
    return (unsigned char*)addr;
}

void UserAllocator::unmap(unsigned char* addr, uint64_t size) {
    return;
}

DirectAllocator::DirectAllocator() {
    uint32_t addr, size;
    FpgaDriver::inst()->getInfo(addr, size);
    mAddr = (uint64_t)addr;
    mTotalSize = (uint64_t)size;
}

DirectAllocator::~DirectAllocator() {

}

unsigned char* DirectAllocator::map(uint64_t addr, uint64_t size) {
    unsigned char* data = FpgaDriver::inst()->getBuffer();
    return data + (addr - mAddr);
}

void DirectAllocator::unmap(unsigned char* addr, uint64_t size) {
    return;
}

V4l2Allocator::V4l2Allocator() {
    V4l2File::inst()->open();
}

V4l2Allocator::~V4l2Allocator() {
}

uint64_t V4l2Allocator::alloc(uint64_t size) {
    return V4l2File::inst()->alloc(size);
}

bool V4l2Allocator::free(uint64_t addr, uint64_t size) {
    V4l2File::inst()->free(addr);
    return true;
}

unsigned char* V4l2Allocator::map(uint64_t addr, uint64_t size) {
    return (unsigned char*)V4l2File::inst()->mmap(addr, size);
}

void V4l2Allocator::unmap(unsigned char* addr, uint64_t size) {
    V4l2File::inst()->munmap(addr, size);
    return;
}

