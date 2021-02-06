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

#ifndef _VSI_3RDLIBS_INCLUDE_EAUTOLOCK_H_
#define _VSI_3RDLIBS_INCLUDE_EAUTOLOCK_H_

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

class ILock {
 public:
    virtual ~ILock() {}
    virtual void lock()  const = 0;
    virtual void unlock() const = 0;
};

class EMutex : public ILock {
 public:
    inline EMutex();
    inline virtual ~EMutex();
    inline virtual void lock() const;
    inline virtual void unlock() const;

 private:
    mutable pthread_mutex_t mMutex;
};

class EAutoLock {
 public:
    inline EAutoLock(ILock* lock);
    inline ~EAutoLock();

 private:
    ILock* mLock = NULL;
};

class ESignal {
 public:
    inline ESignal();
    inline virtual ~ESignal();
    inline void wait();
    inline bool waitMillSeconds(int ms);
    inline void post();

 private:
    sem_t sem;
};

EMutex::EMutex() {
    pthread_mutex_init(&mMutex, NULL);
}

EMutex::~EMutex() {
    pthread_mutex_destroy(&mMutex);
}

void EMutex::lock() const {
    pthread_mutex_lock(&mMutex);
}

void EMutex::unlock() const {
    pthread_mutex_unlock(&mMutex);
}

EAutoLock::EAutoLock(ILock* lock) : mLock(lock) {
    if (mLock != NULL) mLock->lock();
}

EAutoLock::~EAutoLock() {
    if (mLock != NULL) mLock->unlock();
}

ESignal::ESignal() {
    sem_init(&sem, 0, 0);
}

ESignal::~ESignal() {
    sem_destroy(&sem);
}

void ESignal::wait() {
    sem_wait(&sem);
}

bool ESignal::waitMillSeconds(int ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += ms*1000000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;
    int ret = sem_timedwait(&sem, &ts);
    return ret == 0;
}

void ESignal::post() {
    sem_post(&sem);
}

#endif  // _VSI_3RDLIBS_INCLUDE_EAUTOLOCK_H_


