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
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "log.h"
#include "V4l2Event.h"
#include "MediaPipeline.h"

#define LOGTAG "ISP_MEDIA_SERVER"
#define LOCKFILE "/var/run/isp_media_server.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int server_already_running(void)
{
    int     result;
    int     fd;
    char    pid_buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
    if(fd < 0)
    {
        ALOGE("can't open %s\n", LOCKFILE);
        return -2;
    }

    if(flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
        ALOGE("isp_media_server is running(errno=%d)!\n", errno);
        close(fd);
        return -2;
    }

    result = ftruncate(fd, 0);
    if(result < 0)
    {
        ALOGE("ftruncate error!\n");
        return fd;
    }
    sprintf(pid_buf, "%ld\n", (long)getpid());
    result = write(fd, pid_buf, strlen(pid_buf)+1);
    if(result < 0)
    {
        ALOGE("write pid error!\n");
        return fd;
    }
    return fd;
}

MediaPipeline pipeline;   // manage all isp/dwe/vse/dec/v4l2 and link streams.
int main(int argc, char* argv[]) {
    int     fd_running;
    fd_running = server_already_running();
    if(fd_running >= 0)
    {
        ALOGI("************************************************************");
        ALOGI("       VIV ISP Media Control Framework V%s", MEDIA_SERVER_VERSION);
        ALOGI("************************************************************\n");

        if (!pipeline.load(argv[1]))
        {
            close(fd_running);
            return 1;
        }
        V4l2Event vEvent;
        vEvent.registerListener(&pipeline);
        ESignal sig;
        sig.wait();
    }
    return 0;
}
