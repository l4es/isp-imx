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

#include "IMediaModule.h"

IMediaModule::IMediaModule() {
    mPassCount = 1;
    // m_costMillSeconds = 0;
}

void IMediaModule::triger(MediaBuffer* image, uint32_t nSinkIndex) {
    if (nSinkIndex >= mSink.size())
        return;
    mSink[nSinkIndex].image = image;
    triger(nSinkIndex);
}
void IMediaModule::triger(int nVal, uint32_t nSinkIndex) {
    if (nSinkIndex >= mSink.size())
        return;
    mSink[nSinkIndex].nVal = nVal;
    triger(nSinkIndex);
}

void IMediaModule::triger(float fVal, uint32_t nSinkIndex) {
    if (nSinkIndex >= mSink.size())
        return;
    mSink[nSinkIndex].fVal = fVal;
    triger(nSinkIndex);
}

void IMediaModule::triger(const char* szVal, uint32_t nSinkIndex) {
    if (nSinkIndex >= mSink.size())
        return;
    mSink[nSinkIndex].strVal = szVal;
    triger(nSinkIndex);
}

void IMediaModule::triger(uint32_t nSinkIndex) {
    setSinkState(nSinkIndex, true);
    if (run(nSinkIndex)) {
        trigerNext(nSinkIndex);
    }
}

bool IMediaModule::connect(IMediaModule* pNext, uint32_t nSinkIndex, uint32_t nSrcIndex) {
    if (!pNext)
        return false;
    if (queryType(PadTypeSrc, nSrcIndex) !=
        pNext->queryType(PadTypeSink, nSinkIndex))
        return false;

    // support link one src to multiple sink pad, user need take care the link to avoid memory copy.
    mConnections[nSrcIndex].push_back(MediaRemotePad(nSinkIndex, pNext));
    pNext->mConnectionsReverse[nSinkIndex].push_back(MediaRemotePad(nSrcIndex, this));
    return true;
}

void IMediaModule::disconnect(IMediaModule* pNext, uint32_t nSinkIndex, uint32_t nSrcIndex) {
    auto iter = mConnections.begin();
    for (; iter != mConnections.end(); iter++) {
        if (iter->first == nSrcIndex) {
            mConnections.erase(iter);
            break;
        }
    }
    for (iter = pNext->mConnectionsReverse.begin(); iter != pNext->mConnectionsReverse.end(); iter++) {
        if (iter->first == nSinkIndex) {
            pNext->mConnectionsReverse.erase(iter);
			break;
        }
    }
}

void IMediaModule::trigerNext(uint32_t nSrcIndex) {
    // printf("IMediaModule triger next %s srcpad: %d\n", getClassName(), nSrcIndex);
    for (auto& item : mConnections) {
        if (nSrcIndex == item.first) {
            int type = queryType(PadTypeSrc, item.first);
            for (auto& link : item.second) {
                if (type == PAD_TYPE_IMAGE)
                    link.pNext->triger(mSrc[item.first].image, link.nSinkIndex);
                else if (type == PAD_TYPE_INT)
                    link.pNext->triger(mSrc[item.first].nVal, link.nSinkIndex);
                else if (type == PAD_TYPE_FLOAT)
                    link.pNext->triger(mSrc[item.first].fVal, link.nSinkIndex);
                else if (type == PAD_TYPE_STRING)
                    link.pNext->triger(mSrc[item.first].strVal.c_str(), link.nSinkIndex);
            }
        }
    }
}

void IMediaModule::setSinkState(uint32_t nIndex, bool bState) {
    mSink[nIndex].state = bState;
}

bool IMediaModule::getSinkState(uint32_t nIndex) {
    return mSink[nIndex].state;
}

int IMediaModule::queryType(int nSinkSrc, uint32_t nIndex) {
    if (nSinkSrc == PadTypeSink) {
        if (nIndex >= mSink.size())
            return PAD_TYPE_UNKNOWN;
        return mSink[nIndex].type;
    }
    if (nSinkSrc == PadTypeSrc) {
        if (nIndex >= mSrc.size()) {
            return PAD_TYPE_UNKNOWN;
        }
        return mSrc[nIndex].type;
    }
    return PAD_TYPE_UNKNOWN;
}

void IMediaModule::releasePadBuffer(int port, uint64_t addr) {
    for (auto& item : mPadQueue[port].mBuffers) {
        if (addr == BASEADDR(item)) {
            mPadQueue[port].releaseBuffer(item);
            for (auto& prePad : mConnectionsReverse[port]) {
                prePad.pNext->onConnectedPadBufferChanged(prePad.nSinkIndex, mPadQueue[port], getClassName());
            }
            break;
        }
    }
}

ModuleRegistry* ModuleRegistry::mRegistry = NULL;

IMediaModule* ModuleRegistry::create(const char* name) {
    ClassTable::iterator pos = _classTable.find(name);
    if (pos == _classTable.end()) {
        printf("Not found the class of this name in rtti table\n");
        return NULL;
    }
    return pos->second();
}

void SafeRelease(const IMediaModule*& pointer) {
    if (pointer != NULL) {
        delete pointer;
        pointer = NULL;
    }
}
