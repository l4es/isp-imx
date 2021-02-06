/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <assert.h>
#include <memory.h>
#include <linux/videodev2.h>
#include <semaphore.h>

#include <thread>
#include <vector>

#include <MediaBuffer.h>
#include <linux/fb.h>

#include "log.h"
#include "viv_video_kevent.h"

#define LOGTAG "VIDEOTEST"

#include "IDisplay.h"

int fd = -1;
std::string strDeviceName;

static std::unordered_map<std::string, int> string2MediaFormat = {
    {"YUYV", MEDIA_PIX_FMT_YUV422I},
    {"NV12", MEDIA_PIX_FMT_YUV420SP},
    {"NV16", MEDIA_PIX_FMT_YUV422SP},
    {"RAW8", MEDIA_PIX_FMT_RAW8},
    {"RAW10", MEDIA_PIX_FMT_RAW10},
    {"RAW12", MEDIA_PIX_FMT_RAW12},
};

static std::unordered_map<int, int> mediaFormat2V4l2Format = {
    {MEDIA_PIX_FMT_YUV422I, V4L2_PIX_FMT_YUYV},
    {MEDIA_PIX_FMT_YUV420SP, V4L2_PIX_FMT_NV12},
    {MEDIA_PIX_FMT_YUV422SP, V4L2_PIX_FMT_NV16},
};

static std::unordered_map<std::string, int> ctrlIDList = {
    {"sensor.mode", 0},
    {"sensor.resw", 0},
    {"sensor.resh", 0},
    /* To be added */
};

static struct v4l2_query_ext_ctrl vivExtQctrl; /* query for viv_ext_ctrl */

static void updateCtrlIDList(int fd) {
    struct v4l2_query_ext_ctrl qctrl;
    int id = 0;
    do {
        memset(&qctrl, 0xff, sizeof(qctrl));
        qctrl.id = id |
                V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;

        if (ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &qctrl) < 0) {
            ALOGD("VIDIOC_QUERY_EXT_CTRL: %s", strerror(errno));
            break;
        } else if (qctrl.type == V4L2_CTRL_TYPE_STRING) {
            ALOGD("V4L2 ext ctrl id: 0x%x", qctrl.id);
            ALOGD("V4L2 ext ctrl name: %s", qctrl.name);
            ALOGD("V4L2 ext ctrl elem size: %d", qctrl.elem_size);
            memcpy(&vivExtQctrl, &qctrl, sizeof(qctrl));
        } else {
            if (ctrlIDList.find(qctrl.name) != ctrlIDList.end()) {
                ctrlIDList[qctrl.name] = qctrl.id;
                ALOGD("V4L2 ext ctrl id: 0x%x", qctrl.id);
                ALOGD("V4L2 ext ctrl name: %s", qctrl.name);
                ALOGD("V4L2 ext ctrl elem size: %d", qctrl.elem_size);
            }
        }
        id = qctrl.id;
    } while (true);
}

int openCamera() {
    if (fd >=0)
        return fd;
    for (int i = 0; i < 20; i++) {
        char szFile[64];
        sprintf(szFile, "/dev/video%d", i);
        fd = ::open(szFile, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            ALOGE("can't open video file %s", szFile);
            continue;
        }
        v4l2_capability caps;
        int result = ioctl(fd, VIDIOC_QUERYCAP, &caps);
        if (result  < 0) {
            ALOGE("failed to get device caps for %s (%d = %s)", szFile, errno, strerror(errno));
            close(fd);
            continue;
        }

        ALOGI("Open Device: %s (fd=%d)", szFile, fd);
        ALOGI("  Driver: %s", caps.driver);

        if (strcmp((const char*)caps.driver, "viv_v4l2_device") == 0) {
            ALOGI("found viv video dev %s", szFile);
            strDeviceName = szFile;
            break;
        }
        close(fd);
    }
    return fd;
}

void dump_args() {
    ALOGE("video_test width height format display_type");
    ALOGE("format list:");
    for (auto& item : string2MediaFormat) {
        printf("%s, ", item.first.c_str());
    }
    printf("\n");

    ALOGE("display_tpye:");
    printf("0-dryrun, 1-drm, 2-file, 3-fb\n");
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        dump_args();
        return 1;
    }
    sleep(6);
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    if ((width < 0) || (height < 0)) {
        dump_args();
        return 1;
    }
    const char* szFormat = argv[3];
    if (string2MediaFormat.find(szFormat) == string2MediaFormat.end()) {
        ALOGE("unsupported format: %s", szFormat);
        dump_args();
        return 1;
    }

    int displayType = atoi(argv[4]);
    
    IDisplay* pDisplay = IDisplay::createObject(displayType);
    if (!pDisplay) {
        ALOGE("unsupported displaytype: %d", displayType);
        dump_args();
        return 1;
    }

    int camDevice = openCamera();
    ALOGI("open camdev ret : %d", camDevice);
    v4l2_capability caps;
    {
        int result = ioctl(camDevice, VIDIOC_QUERYCAP, &caps);
        if (result  < 0) {
            ALOGE("failed to get device caps for %s (%d = %s)", strDeviceName.c_str(), errno, strerror(errno));
            return 0;
        }
    }

    // Report device properties
    ALOGI("Open Device: %s (fd=%d)", strDeviceName.c_str(), camDevice);
    ALOGI("  Driver: %s", caps.driver);
    ALOGI("  Card: %s", caps.card);
    ALOGI("  Version: %u.%u.%u",
            (caps.version >> 16) & 0xFF,
            (caps.version >> 8)  & 0xFF,
            (caps.version)       & 0xFF);
    ALOGI("  All Caps: %08X", caps.capabilities);
    ALOGI("  Dev Caps: %08X", caps.device_caps);

    if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
        !(caps.capabilities & V4L2_CAP_STREAMING)) {
        ALOGE("Streaming capture not supported by %s.", strDeviceName.c_str());
        return false;
    }

    struct viv_caps_supports caps_supports;
    memset(&caps_supports,0,sizeof(caps_supports));
    int ret = ioctl(camDevice, VIV_VIDIOC_GET_CAPS_SUPPORTS, &caps_supports);
    if (ret == 0)
    {
        printf("Video Test:caps supports:{\n");
        printf("\tcount = %d\n",caps_supports.count);
        for(unsigned int i=0; i<caps_supports.count; i++)
        {
            printf("\t{\n");
            printf("\tindex    = %d\n",caps_supports.mode[i].index);
            printf("\twidth    = %d\n",caps_supports.mode[i].width);
            printf("\theight   = %d\n",caps_supports.mode[i].height);
            printf("\tfps      = %d\n",caps_supports.mode[i].fps);
            printf("\thdr_mode = %d\n",caps_supports.mode[i].hdr_mode);
            printf("\t}\n");
        }
        printf("}\n");
    }else
    {
        ALOGE("Video Test:Get Caps Supports Failed[%d]\n",ret);
        return false;
    }

    if (argc == 6)
    {
        struct viv_caps_mode_s caps_mode;
        memset(&caps_mode,0,sizeof(caps_mode));
        caps_mode.mode = atoi(argv[5]);
        if (ioctl(camDevice, VIV_VIDIOC_S_CAPS_MODE, &caps_mode) == 0)
        {
            ALOGI("Video Test: Set mode[%d]\n",caps_mode.mode);
        }else
        {
            ALOGE("Video Test: Set mode[%d] Failed\n",caps_mode.mode);
            return false;
        }
        
    }
    

    v4l2_fmtdesc formatDescriptions;
    formatDescriptions.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (int i=0; true; i++) {
        formatDescriptions.index = i;
        if (ioctl(camDevice, VIDIOC_ENUM_FMT, &formatDescriptions) == 0) {
            ALOGI("  %2d: %s 0x%08X 0x%X",
                   i,
                   formatDescriptions.description,
                   formatDescriptions.pixelformat,
                   formatDescriptions.flags
            );
            switch(formatDescriptions.pixelformat)
            {
                case V4L2_PIX_FMT_SBGGR8:
                case V4L2_PIX_FMT_SGBRG8:
                case V4L2_PIX_FMT_SGRBG8:
                case V4L2_PIX_FMT_SRGGB8:
                    mediaFormat2V4l2Format[MEDIA_PIX_FMT_RAW8] = formatDescriptions.pixelformat;
                    break;
                case V4L2_PIX_FMT_SBGGR10:
                case V4L2_PIX_FMT_SGBRG10:
                case V4L2_PIX_FMT_SGRBG10:
                case V4L2_PIX_FMT_SRGGB10:
                    mediaFormat2V4l2Format[MEDIA_PIX_FMT_RAW10] = formatDescriptions.pixelformat;
                    break;
                case V4L2_PIX_FMT_SBGGR12:
                case V4L2_PIX_FMT_SGBRG12:
                case V4L2_PIX_FMT_SGRBG12:
                case V4L2_PIX_FMT_SRGGB12:
                     mediaFormat2V4l2Format[MEDIA_PIX_FMT_RAW12] = formatDescriptions.pixelformat;
                     break;
                case V4L2_PIX_FMT_NV16:
                    mediaFormat2V4l2Format[MEDIA_PIX_FMT_YUV422SP] = formatDescriptions.pixelformat;
                    break;
                case V4L2_PIX_FMT_YUYV:
                    mediaFormat2V4l2Format[MEDIA_PIX_FMT_YUV422I] = formatDescriptions.pixelformat;
                    break;
                case V4L2_PIX_FMT_NV12:
                    mediaFormat2V4l2Format[MEDIA_PIX_FMT_YUV420SP] = formatDescriptions.pixelformat;
                    break;
            }
            
        } else {
            break;
        }
    }

    unsigned int pixelFormat = mediaFormat2V4l2Format[string2MediaFormat[szFormat]];
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = pixelFormat;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    if (ioctl(camDevice, VIDIOC_S_FMT, &format) < 0) {
        ALOGE("VIDIOC_S_FMT: %s", strerror(errno));
        return -1;
    }

    updateCtrlIDList(camDevice);

    /* Sample for querying sensor caps */
    while (vivExtQctrl.id > 0) {
        struct v4l2_ext_controls ectrls;
        struct v4l2_ext_control ectrl;
        memset(&ectrls, 0, sizeof(ectrls));
        memset(&ectrl, 0, sizeof(ectrl));
        ectrl.string = new char [vivExtQctrl.elem_size];
        if (!ectrl.string)
            break;
        ectrl.id = vivExtQctrl.id;
        ectrl.size = vivExtQctrl.elem_size;
        ectrls.controls = &ectrl;
        ectrls.count = 1;

        /* JSON format string request */
        strncpy(ectrl.string, "{<id>:<sensor.query>}", vivExtQctrl.elem_size - 1);

        /* Set the control: 'sensor.query' */
        if (ioctl(fd, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_S_EXT_CTRLS: %s", strerror(errno));
            delete [] ectrl.string;
            break;
        }

        /* Get the result */
        if (ioctl(fd, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_G_EXT_CTRLS: %s", strerror(errno));
        }
        ALOGD("Sensor's caps (JSON format string):%s", ectrl.string);
        delete [] ectrl.string;
        break;
    }

    /* Sample for how to use G/S_CTRL */
    /*set sensor mode need updata xml file, so now use VIV_VIDIOC_S_CAPS_MODE.not use VIDIOC_S_CTRL
    if (ctrlIDList["sensor.mode"] > 0) {
        struct v4l2_control ctrl;
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = ctrlIDList["sensor.mode"];
        if (ioctl(camDevice, VIDIOC_G_CTRL, &ctrl) < 0) {
            ALOGE("VIDIOC_G_CTRL: %s", strerror(errno));
        } else {
            ALOGI("Current sensor's mode: %d", ctrl.value);
        }

        if (ioctl(camDevice, VIDIOC_S_CTRL, &ctrl) < 0) {
            ALOGE("VIDIOC_S_CTRL: %s", strerror(errno));
        } else {
            ALOGI("Change sensor's mode to: %d", ctrl.value);
        }
    }*/

    /* Sample for sensor lib preloading & warm-up */
    while (vivExtQctrl.id > 0) {
        struct v4l2_ext_controls ectrls;
        struct v4l2_ext_control ectrl;
        memset(&ectrls, 0, sizeof(ectrls));
        memset(&ectrl, 0, sizeof(ectrl));
        ectrl.string = new char [vivExtQctrl.elem_size];
        if (!ectrl.string)
            break;
        ectrl.id = vivExtQctrl.id;
        ectrl.size = vivExtQctrl.elem_size;
        ectrls.controls = &ectrl;
        ectrls.count = 1;

        /* JSON format string request */
        strncpy(ectrl.string, "{<id>:<sensor.lib.preload>}", vivExtQctrl.elem_size - 1);

        /* Set the control 'sensor lib preload' */
        if (ioctl(camDevice, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_S_EXT_CTRLS: %s", strerror(errno));
            delete [] ectrl.string;
            break;
        }

        /* Get the result */
        if (ioctl(camDevice, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_G_EXT_CTRLS: %s", strerror(errno));
            delete [] ectrl.string;
            break;
        }

        strncpy(ectrl.string, "{<id>:<pipeline.warm.up>}", vivExtQctrl.elem_size - 1);

        /* Set the control: 'warm up' */
        if (ioctl(camDevice, VIDIOC_S_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_S_EXT_CTRLS: %s", strerror(errno));
            delete [] ectrl.string;
            break;
        }

        /* Get the result */
        if (ioctl(camDevice, VIDIOC_G_EXT_CTRLS, &ectrls) < 0) {
            ALOGE("VIDIOC_G_EXT_CTRLS: %s", strerror(errno));
        }
        delete [] ectrl.string;
        break;
    }

    int bufferCount = 4;
    v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = bufferCount;
    if (ioctl(camDevice, VIDIOC_REQBUFS, &bufrequest) < 0) {
        ALOGE("VIDIOC_REQBUFS: %s", strerror(errno));
        return -1;
    }

    std::vector<void*> vData;
    v4l2_buffer buffer = {};
    for (int i = 0; i < bufferCount; i++) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory   = V4L2_MEMORY_MMAP;
        buffer.index    = i;
        if (ioctl(camDevice, VIDIOC_QUERYBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QUERYBUF: %s", strerror(errno));
            return false;
        }
        ALOGI("Buffer description:");
        ALOGI("  offset: %d", buffer.m.offset);
        ALOGI("  length: %d", buffer.length);
        if (ioctl(camDevice, VIDIOC_QBUF, &buffer) < 0) {
            ALOGE("VIDIOC_QBUF: %s", strerror(errno));
            return false;
        }
        void* data = mmap(
                NULL,
                buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                camDevice,
                buffer.m.offset
        );
        if (data == MAP_FAILED) {
            ALOGE("mmap: %s", strerror(errno));
            return false;
        }
        ALOGI("map buffer %p", data);
        vData.push_back(data);
    }

    int type = buffer.type;
    if (ioctl(camDevice, VIDIOC_STREAMON, &type) < 0) {
        ALOGE("VIDIOC_STREAMON: %s", strerror(errno));
        return false;
    }

    while (1) {
        fd_set fds;
        FD_ZERO (&fds);
        FD_SET (camDevice, &fds);
        select (camDevice + 1, &fds, NULL, NULL, NULL);
        if (ioctl(camDevice, VIDIOC_DQBUF, &buffer) < 0) {
            ALOGE("VIDIOC_DQBUF: %s", strerror(errno));
        } else {
            ALOGI("VIDIOC_DQBUF success");
//#ifndef ANDROID
            pDisplay->showBuffer((unsigned char*)vData[buffer.index], width, height, pixelFormat, buffer.length);
//#endif
			ioctl(camDevice, VIDIOC_QBUF, &buffer);
        }   
    }

    if (ioctl(camDevice, VIDIOC_STREAMOFF, &type) < 0) {
        ALOGE("VIDIOC_STREAMOFF: %s", strerror(errno));
        return false;
    }

    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 0;
    if (ioctl(camDevice, VIDIOC_REQBUFS, &bufrequest) < 0) {
        ALOGE("VIDIOC_REQBUFS: %s", strerror(errno));
        return -1;
    }

    close(camDevice);
    return 0;
}

