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

#ifndef __VVBPARSER_H__
#define __VVBPARSER_H__
#include "cam_device_buf_defs.h"
#include <json/json.h>
#include "cam_device_buf_defs_common.h"

#define VVBENCH_VERSION "v1.0"

#define VVCASE_EXEC_PASS 0
#define VVCASE_EXEC_FAIL -1
#define VVCASE_EXEC_PARSER_FAIL -2

int vvb_config_parser(const char* file_name, struct vvcfg_t * cfg_ctx );
int vvb_list_execute(const char* file_name, struct vvcfg_t * cfg_ctx);
//int vvb_case_parser()
int vvb_case_parser(const char* file_name, struct vvdev_t * case_ctx );

BUFF_MODE from_str_to_buff_mode(const char * buffer_string);
ISPCORE_BUFIO_ID from_str_to_buff_io(const char * buffer_string);
int from_str_to_buff_format(const char * format_string);
int from_str_to_input_type(const char * input_type);

int vvb_parse_sensor_info(Json::Value &sensor_info, struct vvisp_sensor_cfg * sensor_cfg);
int vvb_parse_func_info(Json::Value &func_info, struct vvisp_func_ctrl * func_cfg);


#endif //__VVBPARSER_H__