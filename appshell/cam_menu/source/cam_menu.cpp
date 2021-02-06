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
#include <stdlib.h>
#include <stdio.h>

#include "cam_menu.hpp"
#include "log.h"
#include "pytool.hpp"

#define LOGTAG "cam_menu"

extern "C" {
#include "vsi_v4l2.h"
#include "vsi_drm.h"
}

#define MAX_CAM_DEV 2

int cam_menu_exit = 0;

void cam_menu_reg_rw_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_REG_RW", __func__);
        ALOGI("%s: (%d) ISP Register Read", __func__, CAM_MENU_REG_RW_READ);
        ALOGI("%s: (%d) ISP Register Write", __func__, CAM_MENU_REG_RW_WRITE);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_REG_RW_READ:
                ALOGE("ISP Register Read: %d", cam_submenu);

                break;

            case CAM_MENU_REG_RW_WRITE:
                ALOGE("ISP Register Write: %d", cam_submenu);

                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    } while (0 == *cam_submenu_exit);
}

void cam_menu_i2c_rw_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_I2C_RW", __func__);
        ALOGI("%s: (%d) SENSOR Registerr Read", __func__, CAM_MENU_I2C_RW_READ);
        ALOGI("%s: (%d) SENSOR Registerr Write", __func__, CAM_MENU_I2C_RW_WRITE);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_I2C_RW_READ:
                ALOGE("SENSOR Register Read: %d", cam_submenu);

                break;

            case CAM_MENU_I2C_RW_WRITE:
                ALOGE("SENSOR Register Write: %d", cam_submenu);

                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    } while (0 == *cam_submenu_exit);
}

void cam_menu_v4l2_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;
    unsigned int width, height;
    unsigned int bufferCount;
    unsigned int imageProcess;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_V4L2", __func__);
        ALOGI("%s: (%d) V4L2 Open Video", __func__, CAM_MENU_V4L2_OPEN);
        ALOGI("%s: (%d) V4L2 Query Driver Capability", __func__, CAM_MENU_V4L2_QUERYCAP);
        ALOGI("%s: (%d) V4L2 Get Supported Video Format", __func__, CAM_MENU_V4L2_ENUM_FMT);
        ALOGI("%s: (%d) V4L2 Set Video Capture Format", __func__, CAM_MENU_V4L2_SFMT);
        ALOGI("%s: (%d) V4L2 Requesti Data Buffers", __func__, CAM_MENU_V4L2_REQBUFS);
        // ALOGI("%s: (%d) V4L2 Mmap Buffers", __func__, CAM_MENU_V4L2_QUERYBUFS);
        ALOGI("%s: (%d) V4L2 Data Process Type", __func__, CAM_MENU_V4L2_PROCESS_IMAGE);
        ALOGI("%s: (%d) V4L2 Start Capturing Video", __func__, CAM_MENU_V4L2_STREAM_ON);
        ALOGI("%s: (%d) V4L2 Stop Capturing Video", __func__, CAM_MENU_V4L2_STREAM_OFF);
        ALOGI("%s: (%d) V4L2 Close Video", __func__, CAM_MENU_V4L2_CLOSE);
        ALOGI("%s: (%d) V4L2 Display Video", __func__, CAM_MENU_V4L2_DISPLAY);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_V4L2_OPEN:
                ALOGE("V4L2 Open Video: %d", cam_submenu);
                v4l2_openCamera();
                break;

            case CAM_MENU_V4L2_QUERYCAP:
                ALOGE("V4L2 Query Driver Capability: %d", cam_submenu);
                v4l2_querycap();
                break;

            case CAM_MENU_V4L2_ENUM_FMT:
                ALOGE("V4L2 Get Supported Video Format: %d", cam_submenu);
                v4l2_enum_fmt();
                break;

            case CAM_MENU_V4L2_SFMT:
                ALOGE("V4L2 Set Video Capture Format: %d", cam_submenu);
                scanf("input width height: %u %u", &width, &height);
                if ((width <= 0) || (height <= 0)) {
                    ALOGE("input error!: %u, %u\n", width, height);
                    break;
                }
                v4l2_set_fmt(width, height);
                break;

            case CAM_MENU_V4L2_REQBUFS:
                ALOGE("V4L2 Requesti Data Buffers: %d", cam_submenu);
                scanf("input buffer count: %u", &bufferCount);
                if (bufferCount <= 0) {
                    ALOGE("input error!: %u\n", bufferCount);
                    break;
                }
                v4l2_init_mmap(bufferCount);
                break;
/*
            case CAM_MENU_V4L2_QUERYBUFS:
                ALOGE("V4L2 Mmap Buffers: %d", cam_submenu);

                break;
*/
            case CAM_MENU_V4L2_PROCESS_IMAGE:
                ALOGE("V4L2 Data Process Type: %d", cam_submenu);
                ALOGI("Display by DRM:%d\n", OUTPUTDRM);
                ALOGI("Display by FB:%d\n", OUTPUTFB);
                ALOGI("Save to File:%d\n", SAVEIMAGE);
                scanf("Select process type: %u", &imageProcess);
                if (imageProcess > 3) {
                    ALOGE("input error!: %u\n", imageProcess);
                    break;
                }
                v4l2_process_image(imageProcess);
                break;

            case CAM_MENU_V4L2_STREAM_ON:
                ALOGE("V4L2 Start Capturing Video: %d", cam_submenu);
                v4l2_start_capture();
                break;

            case CAM_MENU_V4L2_STREAM_OFF:
                ALOGE("V4L2 Stop Capturing Video: %d", cam_submenu);
                v4l2_stop_capture();
                break;

            case CAM_MENU_V4L2_CLOSE:
                ALOGE("V4L2 Close Video: %d", cam_submenu);
                v4l2_closeCamera();
                break;

            case CAM_MENU_V4L2_DISPLAY:
                ALOGE("V4L2 Display Image: %d", cam_submenu);
                display_image();
                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    } while (0 == *cam_submenu_exit);
}

void cam_menu_virt_cam_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_VIRT_CAM", __func__);
        ALOGI("%s: (%d) Virtual Camera Open", __func__, CAM_MENU_VIRT_CAM_OPEN);
        ALOGI("%s: (%d) Virtual Camera Set Format", __func__, CAM_MENU_VIRT_CAM_SETFORMAT);
        ALOGI("%s: (%d) Virtual Camera Data Process Type", __func__, CAM_MENU_VIRT_CAM_DATA_PROCESS);
        ALOGI("%s: (%d) Virtual Camera Connect", __func__, CAM_MENU_VIRT_CAM_CONNECT);
        ALOGI("%s: (%d) Virtual Camera Start", __func__, CAM_MENU_VIRT_CAM_START);
        ALOGI("%s: (%d) Virtual Camera Ioctl", __func__, CAM_MENU_VIRT_CAM_IOCTL);
        ALOGI("%s: (%d) Virtual Camera Stop", __func__, CAM_MENU_VIRT_CAM_STOP);
        ALOGI("%s: (%d) Virtual Camera Disconnect", __func__, CAM_MENU_VIRT_CAM_DISCONNECT);
        ALOGI("%s: (%d) Virtual Camera Close", __func__, CAM_MENU_VIRT_CAM_CLOSE);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_VIRT_CAM_OPEN:
                ALOGE("Virtual Camera Open: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_SETFORMAT:
                ALOGE("Virtual Camera Set Format: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_DATA_PROCESS:
                ALOGE("Virtual Camera Data Process Type: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_CONNECT:
                ALOGE("Virtual Camera Connect: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_START:
                ALOGE("Virtual Camera Start: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_IOCTL:
                ALOGE("Virtual Camera Ioctl: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_STOP:
                ALOGE("Virtual Camera Stop: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_DISCONNECT:
                ALOGE("Virtual Camera Disconnect: %d", cam_submenu);

                break;

            case CAM_MENU_VIRT_CAM_CLOSE:
                ALOGE("Virtual Camera Close: %d", cam_submenu);

                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    }while (0 == *cam_submenu_exit);
}

void cam_menu_config_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_CONFIG", __func__);
        ALOGI("%s: (%d) Config Video Node", __func__, CAM_MENU_CONFIG_VIDEO_NODE);
        ALOGI("%s: (%d) Config FB Node", __func__, CAM_MENU_CONFIG_FB_NODE);
        ALOGI("%s: (%d) Config File Name And Path", __func__, CAM_MENU_CONFIG_FILE_NAME_PATH);
        ALOGI("%s: (%d) Config File Name Prefix", __func__, CAM_MENU_CONFIG_FILE_NAME_PREFIX);
        ALOGI("%s: (%d) Config Json Path", __func__, CAM_MENU_CONFIG_JSON_PATH);
        ALOGI("%s: (%d) Config Json Prefix", __func__, CAM_MENU_CONFIG_JSON_PREFIX);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_CONFIG_VIDEO_NODE:
                ALOGE("Config Video Node: %d", cam_submenu);

                break;

            case CAM_MENU_CONFIG_FB_NODE:
                ALOGE("Config FB Node: %d", cam_submenu);

                break;

            case CAM_MENU_CONFIG_FILE_NAME_PATH:
                ALOGE("Config File Name And Path: %d", cam_submenu);

                break;

            case CAM_MENU_CONFIG_FILE_NAME_PREFIX:
                ALOGE("Config File Name Prefix: %d", cam_submenu);

                break;

            case CAM_MENU_CONFIG_JSON_PATH:
                ALOGE("Config Json Path: %d", cam_submenu);

                break;

            case CAM_MENU_CONFIG_JSON_PREFIX:
                ALOGE("Config Json Prefix: %d", cam_submenu);

                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    } while (0 == *cam_submenu_exit);
}

void cam_menu_tools_func(int *cam_menu_exit, int *cam_submenu_exit) {
    int cam_submenu;

    *cam_submenu_exit = 0;

    do {
        ALOGI("%s: Verisilicon Camera Application Menu: CAM_MENU_TOOLS", __func__);
        ALOGI("%s: (%d) Video Information", __func__, CAM_MENU_TOOLS_VIDEO_INFO);
        ALOGI("%s: (%d) FB Information", __func__, CAM_MENU_TOOLS_FB_INFO);
        ALOGI("%s: (%d) V4L2 Compliance", __func__, CAM_MENU_TOOLS_V4L2_COMPLIANCE);
        ALOGI("%s: (%d) V4L2 Compliance Config", __func__, CAM_MENU_TOOLS_V4L2_COMPLIANCE_CFG);

        ALOGI("%s: (%d) Up to main menu", __func__, CAM_MENU_EXIT);
        // ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_SUB_ABORT);
        ALOGI("%s: Choose Sub Menu >", __func__);

        cam_submenu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_submenu);
        ALOGI("Selected sub-menu:%d\n", cam_submenu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_submenu) {
            case CAM_MENU_TOOLS_VIDEO_INFO:
                ALOGE("Video Information: %d", cam_submenu);

                break;

            case CAM_MENU_TOOLS_FB_INFO:
                ALOGE("FB Information: %d", cam_submenu);

                break;

            case CAM_MENU_TOOLS_V4L2_COMPLIANCE:
                ALOGE("V4L2 Compliance: %d", cam_submenu);

                break;

            case CAM_MENU_TOOLS_V4L2_COMPLIANCE_CFG:
                ALOGE("V4L2 Compliance Config: %d", cam_submenu);

                break;

            case CAM_MENU_EXIT:
                ALOGE("Up to Main Menu %d", cam_submenu);
                *cam_submenu_exit = 1;
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit program %d", cam_submenu);
                *cam_menu_exit = 1;
                *cam_submenu_exit = 1;
                break;
        }
    } while (0 == *cam_submenu_exit);
}

int main(int argc, char* argv[]) {
    int current_node = 0;
    int cam_menu;
    int cam_submenu;

    cam_menu_exit = 0;
    int cam_submenu_exit = 0;

    int py_init_flag = 0;
    struct cam_dev_parameters cam_devs[MAX_CAM_DEV];

    // Default value
    cam_devs[0].cam_dev_node = "/dev/video0";
    cam_devs[0].dev_fd = -1;
    cam_devs[0].sensor_resolution = CAM_RESOLUTION_1080P;
    cam_devs[0].pix_format = V4L2_PIX_FMT_YUYV;
    cam_devs[0].preview_status = 0;

    cam_devs[1].cam_dev_node = "/dev/video1";
    cam_devs[1].dev_fd = -1;
    cam_devs[1].sensor_resolution = CAM_RESOLUTION_1080P;
    cam_devs[1].pix_format = V4L2_PIX_FMT_YUYV;
    cam_devs[1].preview_status = 0;


    ALOGI("%s: open cam_menu app\n", __func__);
    ALOGI("%s: Verisilicon Camera Application Menu\n", __func__);

    if (argc > 6) {
        ALOGE("%s: \nUseage:\t cam_menu: combo\n", __func__);
        ALOGE("%s: \n{cam_device_node_0}, {sensor0_resolution}\n", __func__);
        ALOGE("%s: \n{cam_device_node_1}, {sensor1_resolution}\n", __func__);
        exit(1);
    }

    if (argc > 2) {
         cam_devs[0].cam_dev_node = argv[2];
    }
    if (argc > 3) {
        cam_devs[0].sensor_resolution = atoi(argv[3]);
    }
    if (argc > 4) {
         cam_devs[1].cam_dev_node = argv[4];
    }
    if (argc > 5) {
        cam_devs[1].sensor_resolution = atoi(argv[4]);
    }

    py_init_flag = py_init();
    if (py_init_flag != 0) {
        ALOGE("%s: Py tool init failure, exit", __func__);
        exit(1);
    }

    do {
        ALOGI("%s: Verisilicon Camera Application Menu", __func__);

        ALOGI("%s: (%d) Launch Cam App", __func__, CAM_MENU_CAM_APP);
        ALOGI("%s: (%d) ISP Register R/W Menu", __func__, CAM_MENU_REG_RW);
        ALOGI("%s: (%d) Sensor I2C R/W Menu", __func__, CAM_MENU_I2C_RW);
        ALOGI("%s: (%d) ISP V4l2 Menu", __func__, CAM_MENU_V4L2);
        ALOGI("%s: (%d) ISP Virtual Camera Menu", __func__, CAM_MENU_VIRT_CAM);
        ALOGI("%s: (%d) CAM Menu Shell configuration", __func__, CAM_MENU_CONFIG);
        ALOGI("%s: (%d) CAM Menu Shell Utilities and Tools", __func__, CAM_MENU_TOOLS);
        ALOGI("%s: (%d) CAM Menu Shell Exit", __func__, CAM_MENU_EXIT);
        ALOGI("%s: Choose Menu >", __func__);

        cam_menu = -1;
        fflush(stdout);
        fflush(stdin);
        scanf("%d", &cam_menu);
        ALOGI("Selected menu:%d\n", cam_menu);
        fflush(stdout);
        fflush(stdin);
        scanf("%*[^\n]%*c");
        usleep(300000);

        switch (cam_menu) {
            case CAM_MENU_EXIT:
                ALOGE("Exit cam_menu %d", cam_menu);
                cam_menu_exit = 1;
                break;

            case CAM_MENU_CAM_APP:
                ALOGI("%s Launch \n", __func__);
                cam_app_la(0);
                break;

            case CAM_MENU_REG_RW:
                cam_menu_reg_rw_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_I2C_RW:
                cam_menu_i2c_rw_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_V4L2:
                cam_menu_v4l2_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_VIRT_CAM:
                cam_menu_virt_cam_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_CONFIG:
                cam_menu_config_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_TOOLS:
                cam_menu_tools_func(&cam_menu_exit, &cam_submenu_exit);
                break;

            case CAM_MENU_SUB_ABORT:
                ALOGE("Exit cam_menu %d", cam_menu);
                cam_menu_exit = 1;
                break;

            default:
                ALOGE("%s: Invalid input, press Enter to re-type, Choose Menu >", __func__);
                usleep(1000);
                fflush(stdout);
                fflush(stdin);
                scanf("%*[^\n]%*c");
                break;
        }
    } while (0 == cam_menu_exit);

    // deinit py
    py_deinit();

    return 0;
}
