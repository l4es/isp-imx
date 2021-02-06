/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")  *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

#ifndef __VVDEVICE_H__
#define __VVDEVICE_H__
#include "cam_device_api.hpp"
#include "cam_device_buf_defs.h"
#include "cam_device_sensor_defs.h"
#include <appshell_dom_ctrl/dom_ctrl_api.h>
#include <thread>
#include <functional>

void vvdevice(void);
void vvdevice_delay(int delay_interval);
#define MAX_CAM_NUM 2
#define MAX_BUFFER_QUEUES 32



struct buffer_cfg{
    int buffer_number;
    int buffer_size;
    BufIdentity* buf[MAX_BUFFER_QUEUES];
    std::function<int(MediaBuffer_t * buf, int show_channel)> callback;
};

struct cam_dev_cfg{
    struct buffer_cfg ctx[ISPCORE_BUFIO_MAX];
    
    
};

class vvisp_dev{

public:
    void buf_handle(int isp_id, ISPCORE_BUFIO_ID buf_io);
    int  buf_handle_start(int isp_id, ISPCORE_BUFIO_ID buf_io);
    void buf_handle_stop(int isp_id);
    int  registerBufferCallback(std::function<int(MediaBuffer_t * buf, int show_channel)> callback, int isp_id, ISPCORE_BUFIO_ID buf_io);

    std::thread mThread[MAX_CAM_NUM];
    int running_flag[MAX_CAM_NUM];

    CAM_DEVICE m_cam_dev[MAX_CAM_NUM];
    void * dev_handle[MAX_CAM_NUM];
    struct cam_dev_cfg io_buf[MAX_CAM_NUM];
    IsiSensorCaps_t caps[MAX_CAM_NUM];

    // display
    domCtrlHandle_t dom[MAX_CAM_NUM] = {NULL, NULL};
    void initDom(CAM_DEVICE_ID channel);
    vvisp_dev(struct vvdev_t * case_ctx);
    ~vvisp_dev();
    void showBuffer(MediaBuffer_t * buf, int show_channel);

    int mp_call_back(MediaBuffer_t * buf, int show_channel);
    int sp1_call_back(MediaBuffer_t * buf, int show_channel);
    int sp2_call_back(MediaBuffer_t * buf, int show_channel);
    int dummy_call_back(MediaBuffer_t * buf, int show_channel);
};

struct vvisp_path{
    ISPCORE_BUFIO_ID path;
    int path_enable;
    int buffer_number;
    unsigned int buffer_size;
    unsigned int width;
    unsigned int height;
    unsigned int format;
};

struct vvisp_sensor_cfg{
    unsigned int sensor_width;
    unsigned int sensor_height;
    char sensor_name[128];
    char sensor_dev_node[128];
    char calibration_name[128];
    unsigned int dev_id;
};

struct aaa_func_ctrl{
    int aec_enable;
    int awb_enable;
};

struct pipeline_func_ctrl{
    int wdr3_enable;
    int tdnr_enable;
    int sdnr_enable;
};

struct vvisp_func_ctrl{
    struct aaa_func_ctrl aaa_ctrl;
    struct pipeline_func_ctrl pipeline_ctrl;
};

struct vvisp_cfg{
    CAM_DEVICE_ID isp_id;
    int isp_enable;
    BUFF_MODE buff_mode;
    struct vvisp_path isp_path[ISPCORE_BUFIO_MAX];
    char input_type[32];

    struct vvisp_sensor_cfg sensor_cfg;
    struct vvisp_func_ctrl func_ctrl;
    
    int stream_duration;

};


struct vvdev_t{
    int total_isp;
    struct vvisp_cfg isp_cfg_ctx[MAX_CAM_NUM];
};

int execute_caseline(struct vvdev_t * case_ctx, struct vvcfg_t * cfg_ctx);
int get_hw_resources(CAM_DEVICE_ID isp_id);
int check_hw_status(CAM_DEVICE_ID isp_id);
int get_isp_features(CAM_DEVICE_ID isp_id);
int get_isp_versions(void);
int get_isp_versions(void);


//int mp_call_back(MediaBuffer_t * buf, int show_channel);
//int sp1_call_back(MediaBuffer_t * buf, int show_channel);
//int sp2_call_back(MediaBuffer_t * buf, int show_channel);
//int dummy_call_back(MediaBuffer_t * buf, int show_channel);

#endif //__VVDEVICE_H__