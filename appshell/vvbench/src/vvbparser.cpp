/******************************************************************************\
|* Copyright (c) 2020 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
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
#include "vvbench.hpp"
#include "vvbase.hpp"
#include "vvbparser.hpp"
#include <stdio.h>
#include <json/json.h>
#include <fstream>

#define LOGTAG "PARSER"
#include "vlog.h"

#include <unistd.h>
#include "vvdevice.hpp"

#include <time.h>


int vvb_config_parser(const char* file_name, struct vvcfg_t * cfg_ctx )
{
	int ret = 0;
	

	if((cfg_ctx == NULL)||(file_name == NULL))
	{
		LOGE("null inputs");
		return -1;
	}

	LOGW("Input file name:%s", file_name);
	Json::Value root;

	std::ifstream ifs;
	ifs.open(file_name);
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	JSONCPP_STRING errs;
	if (!parseFromStream(builder, ifs, &root, &errs)) 
	{
		LOGE("%s parser failed, err:%s", __func__, errs.c_str());
		
		ret = -1;
	}else{
		Json::Value device_node = root["device"];
		cfg_ctx->cam_dev_id = device_node["cam_dev_id"].asUInt();

		Json::Value memory_pool_cfg = root["memory_pool"];
		cfg_ctx->phy_address_base = memory_pool_cfg["phy_address_base"].asUInt();
		cfg_ctx->align_mask = memory_pool_cfg["align_mask"].asUInt();
		cfg_ctx->total_pool_size = memory_pool_cfg["total_pool_size"].asUInt();


		LOGW("cam_dev_id is %d", cfg_ctx->cam_dev_id);
		LOGW("phy_address_base is 0x%x", cfg_ctx->phy_address_base);
		LOGW("align_mask is 0x%x", cfg_ctx->align_mask);
		LOGW("total pool size is 0x%x", cfg_ctx->total_pool_size);
	}

	ifs.close();
	return ret;
}


int vvb_list_execute(const char* file_name, struct vvcfg_t * cfg_ctx)
{
	int ret = 0;
	int test_loop;
	int test_interval;

	if((cfg_ctx == NULL)||(file_name == NULL))
	{
		LOGE("null inputs");
		return -1;
	}

	LOGW("Input case list name:%s", file_name);
	Json::Value list_root;

	std::ifstream ifs;
	ifs.open(file_name);
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	JSONCPP_STRING errs;
	if (!parseFromStream(builder, ifs, &list_root, &errs)) 
	{
		LOGE("%s parser failed, err:%s", __func__, errs.c_str());
		
		ret = -1;
	}else{
		Json::Value case_cfg = list_root["config"];
		test_loop = case_cfg["case_loop"].asUInt();
		test_interval = case_cfg["case_interval"].asUInt();
		LOGW("test_loop is %d", test_loop);
		LOGW("test_interval is %d", test_interval);
		Json::Value case_lists = list_root["case_lists"];
		if (!case_lists.isArray())
		{
			LOGW("case list parser error");
			ifs.close();
			return -1;
		}
		Json::Value case_report;
		char report_name_str[128];
		int item = 0;

		time_t time_begin;
		time (&time_begin);
		LOGE("Begin Time:%s",asctime(gmtime(&time_begin)));

		sprintf(report_name_str, "%2d: Summary Title", item++);
		case_report[report_name_str] = "VVbench test report";

		sprintf(report_name_str, "%2d: VVbench Version", item++);
		case_report[report_name_str] = VVBENCH_VERSION;

		sprintf(report_name_str, "%2d: Test list", item++);
		case_report[report_name_str] = file_name;

		sprintf(report_name_str, "%2d: Test bebin time", item++);
		case_report[report_name_str] = asctime(gmtime(&time_begin));

		Json::Value case_result;
		Json::Value loop_result;
		Json::Value fail_result;
		int fail_case_cnt = 0;
		int pass_case_cnt = 0;
		int total_cnt = 0;
		int record_fail_idx = 0;
		for (auto&& case_idx : case_lists) 
		{
 			std::string case_name = case_idx.asCString();
			LOGW("case: %s", case_name.c_str());

			int failed_flag = 0;
			loop_result.clear();
			for(int loop_idx = 0; loop_idx < test_loop; loop_idx ++)
			{
				int result = 0;
				LOGW("Round %d", loop_idx);
				LOGD("calling cases");

				//run cases
				struct vvdev_t vvdev_ctx;
				ret = vvb_case_parser(case_name.c_str(), &vvdev_ctx );
				if(ret != 0)
				{
					LOGE("vvb_case_parser error, case name: %s,  exit", case_name.c_str());
					result = VVCASE_EXEC_PARSER_FAIL;
				}else{
					result = execute_caseline(&vvdev_ctx, cfg_ctx);
				}

				if( VVCASE_EXEC_PASS== result)
				{
					loop_result[loop_idx] = "PASS";
					pass_case_cnt ++;
					LOGW("case %s Pass, pass cnt:%d", case_name.c_str(), pass_case_cnt);
				}else if(VVCASE_EXEC_FAIL == result)
				{
					loop_result[loop_idx] = "FAILED";
					failed_flag = 1;
					fail_case_cnt++;
					LOGW("case %s Failed, fail cnt:%d", case_name.c_str(), fail_case_cnt);
				}else if(VVCASE_EXEC_PARSER_FAIL == result)
				{
					loop_result[loop_idx] = "PARSER FAIL";
					failed_flag = 1;
					fail_case_cnt++;
					LOGW("case %s Parser Failed, fail cnt:%d", case_name.c_str(), fail_case_cnt);
				}else{
					loop_result[loop_idx] = "UNDEFINED FAIL";
					failed_flag = 1;
					fail_case_cnt++;
					LOGW("case %s Parser Failed, fail cnt:%d", case_name.c_str(), fail_case_cnt);
				}

				LOGW("calling delay:%d", test_interval);
				vvdevice_delay(test_interval);
				total_cnt ++;
			}

			if(failed_flag)
			{
				fail_result[record_fail_idx++] =  case_name.c_str();
			}

			case_result[case_name.c_str()] = loop_result;
		}
		if(fail_case_cnt == 0)
		{
			fail_result[fail_case_cnt] =  "NONE";
		}

		time_t time_end;
		time (&time_end);
		LOGE("End Time:%s",asctime(gmtime(&time_end)));

		sprintf(report_name_str, "%2d: Test end time", item++);
		case_report[report_name_str] = asctime(gmtime(&time_end));

		sprintf(report_name_str, "%2d: Result", item++);
		case_report[report_name_str] = case_result;

		sprintf(report_name_str, "%2d: Failed Case list", item++);
		case_report[report_name_str] = fail_result;

		sprintf(report_name_str, "%2d: Total Cases", item++);
		case_report[report_name_str] = total_cnt;

		sprintf(report_name_str, "%2d: Totally Pass Cases", item++);
		case_report[report_name_str] = pass_case_cnt;

		sprintf(report_name_str, "%2d: Totally Fail Cases", item++);
		case_report[report_name_str] = fail_case_cnt;

		//output test summary
		std::ofstream ofs;
		char report_file_name[256];
		struct tm * end_time = gmtime(&time_end);
		sprintf(report_file_name, "vvb_report_%d_%d_%d_%d_%d.json", end_time->tm_mon + 1, end_time->tm_mday, end_time->tm_hour, end_time->tm_min, end_time->tm_sec);
		LOGE("Output name: %s", report_file_name);

		ofs.open(report_file_name);
		ofs << case_report.toStyledString();
		ofs.close();

		LOGE("%s Case End", __func__);
	}

	ifs.close();
	return ret;
}

int vvb_case_parser(const char* file_name, struct vvdev_t * case_ctx ){
	int ret = 0;
	char str_buf[256];

	LOGI("Input case list name:%s", file_name);

	if((case_ctx == NULL)||(file_name == NULL))
	{
		LOGE("null inputs");
		return -1;
	}

	Json::Value list_root;

	std::ifstream ifs;
	sprintf(str_buf, "vvbcfg/case/%s", file_name);
	LOGI("Full case list name:%s", str_buf);
	ifs.open(str_buf);
	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	JSONCPP_STRING errs;
	if (!parseFromStream(builder, ifs, &list_root, &errs)) 
	{
		LOGE("%s parser failed, err:%s", __func__, errs.c_str());
		
		ret = -1;
	}else{
		
		case_ctx->total_isp = list_root["isp_number"].asUInt();;
		if(case_ctx->total_isp > CAM_ISPCORE_ID_MAX)
		{
			LOGE("%s: parameter error: total_isp", __func__);
			return -1;
		}

		Json::Value isp_cfg_array = list_root["isp_cfg"];
		if (!isp_cfg_array.isArray())
		{
			LOGW("case list isp_cfg parser error");
			ifs.close();
			return -1;
		}else{
			int index = 0;
	
			//disable all data by default
			for(int i = 0; i < CAM_ISPCORE_ID_MAX; i ++)
			{
				case_ctx->isp_cfg_ctx[i].isp_enable = 0;
			}

			for (auto&& isp_cfg : isp_cfg_array) {

				try{
					int isp_enable = isp_cfg["isp_enalbe"].asUInt();

					int isp_id = isp_cfg["isp_id"].asUInt();
					if(isp_id >= CAM_ISPCORE_ID_MAX )
					{
						LOGW("case list isp_id parser error");
						throw LogicError(-1, "Parse isp_id failed");						
					}

					case_ctx->isp_cfg_ctx[isp_id].isp_enable = isp_enable;
					case_ctx->isp_cfg_ctx[isp_id].isp_id = (CAM_DEVICE_ID)isp_id;
					LOGD("Parse ISPcore %d", case_ctx->isp_cfg_ctx[isp_id].isp_id);

					if(isp_enable == 0)
					{
						LOGD("ISP %d closed", isp_id);
						continue;
					}

					std::string byf_type = isp_cfg["buffer_type"].asString();
					BUFF_MODE buf_mod = from_str_to_buff_mode(byf_type.c_str());
					case_ctx->isp_cfg_ctx[isp_id].buff_mode = buf_mod;

					Json::Value isp_path_cfg_array = isp_cfg["path_cfg"];
					if (!isp_path_cfg_array.isArray())
					{
						LOGW("case list isp_path_cfg_array parser error");
						throw LogicError(-1, "Parse isp_path_cfg_array failed");
					}

					//disable all data by default
					for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
					{
						case_ctx->isp_cfg_ctx[isp_id].isp_path[i].path_enable = 0;
					}

					for(auto & isp_pach_cfg : isp_path_cfg_array )
					{
						int path_enable;
						path_enable = isp_pach_cfg["path_enable"].asUInt();

						std::string path_name = isp_pach_cfg["path_name"].asString();
						if(path_enable == 0)
						{
							LOGD("PATH %s closed", path_name.c_str());
							continue;
						}

						ISPCORE_BUFIO_ID buf_path = from_str_to_buff_io(path_name.c_str());
						LOGD("Parse Path Cfg of isp:%d, buf_mode: %d(%s), path_id:%d(%s)", case_ctx->isp_cfg_ctx[isp_id].isp_id, buf_mod, byf_type.c_str(), buf_path, path_name.c_str());
						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].path_enable = 1;
						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].path = buf_path;
						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].buffer_number = isp_pach_cfg["buffer_number"].asUInt();
						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].buffer_size = isp_pach_cfg["buffer_alloc_size"].asUInt();

						LOGW("path:%s, buf_type:%d(%s), num:%d, buf_alloc_size: 0x%x", path_name.c_str(), buf_mod, byf_type.c_str(), \
							case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].buffer_number, case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].buffer_size
						);

						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].width = isp_pach_cfg["width"].asUInt();
						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].height = isp_pach_cfg["height"].asUInt();

						std::string format_name = isp_pach_cfg["format"].asString();

						int buff_format = from_str_to_buff_format(format_name.c_str());

						if(buff_format >= CAMERA_PIX_FMT_MAX)
						{
							LOGW("case list from_str_to_buff_format invalid format error");
							throw LogicError(-1, "Parse from_str_to_buff_format invalid format failed");
						}

						LOGW("path:%s, width: %d, height:%d, format(%s):%d", path_name.c_str(),\
							case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].width, case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].height,\
							format_name.c_str(), buff_format);

						case_ctx->isp_cfg_ctx[isp_id].isp_path[buf_path].format = buff_format;

					}

					std::string input_type_str = isp_cfg["input_type"].asString();
					strcpy(case_ctx->isp_cfg_ctx[isp_id].input_type, input_type_str.c_str()); 
					LOGW("input type parse: %s", case_ctx->isp_cfg_ctx[isp_id].input_type);

					Json::Value sensor_info = isp_cfg["sensor_info"];
					ret = vvb_parse_sensor_info(sensor_info, &case_ctx->isp_cfg_ctx[isp_id].sensor_cfg);
					if(ret != 0)
					{
						LOGE("vvb_parse_sensor_info parser error");
						throw LogicError(-1, "vvb_parse_sensor_info failed");
					}

					Json::Value func_info = isp_cfg["function_control"];
					ret = vvb_parse_func_info(func_info, &case_ctx->isp_cfg_ctx[isp_id].func_ctrl);
					if(ret != 0)
					{
						LOGE("vvb_parse_sensor_info parser error");
						throw LogicError(-1, "vvb_parse_sensor_info failed");
					}

					int stream_duration = isp_cfg["stream_duration"].asUInt();
					case_ctx->isp_cfg_ctx[isp_id].stream_duration = stream_duration;

					index++;

				} catch (const std::exception &e) {
					LOGE("ERR: %s \n", e.what());
					ifs.close();
					return -1;
				}

			}
		}

	}

	return ret;
}


BUFF_MODE from_str_to_buff_mode(const char * buffer_string){

	BUFF_MODE buff_mode_value;

	if(strcmp(buffer_string, "phylinear") == 0)
	{
		LOGE("Input %s equal to %s",buffer_string, "phylinear");
		buff_mode_value = BUFF_MODE_PHYLINEAR;
	}else if(strcmp(buffer_string, "userptr") == 0)
	{
		LOGE("Input %s equal to %s",buffer_string, "userptr");
		buff_mode_value = BUFF_MODE_USRPTR;
	}else{
		LOGE("Not supprted buffer mode");
		buff_mode_value = BUFF_MODE_INVALID;
		throw LogicError(-1, "Parse BUFF_MODE failed");
	}

	LOGD("%s: Buff mode: %d", __func__, buff_mode_value);

	return buff_mode_value;	
}

ISPCORE_BUFIO_ID from_str_to_buff_io(const char * buffer_string){

	ISPCORE_BUFIO_ID buff_io_value;

	if(strcmp(buffer_string, "MP") == 0)
	{
		LOGE("Input %s equal to %s",buffer_string, "MP");
		buff_io_value = ISPCORE_BUFIO_MP;
	}else if(strcmp(buffer_string, "SP1") == 0)
	{
		LOGE("Input %s equal to %s",buffer_string, "SP1");
		buff_io_value = ISPCORE_BUFIO_SP1;
	}else if(strcmp(buffer_string, "SP2") == 0)
	{
		LOGE("Input %s equal to %s",buffer_string, "SP2");
		buff_io_value = ISPCORE_BUFIO_SP2;
	}else{
		LOGE("Not supprted buffeio");
		buff_io_value = ISPCORE_BUFIO_MAX;
		throw LogicError(-1, "Parse BUFF_IO failed");
	}

	LOGD("%s: Buff IO: %d", __func__, buff_io_value);

	return buff_io_value;	
}



int from_str_to_buff_format(const char * format_string){

	int img_format;

	if(strcmp(format_string, "YUV422SP") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "YUV422SP");
		img_format = CAMERA_PIX_FMT_YUV422SP;
	}else if(strcmp(format_string, "YUV422I") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "YUV422I");
		img_format = CAMERA_PIX_FMT_YUV422I;
	}else if(strcmp(format_string, "YUV420SP") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "YUV420SP");
		img_format = CAMERA_PIX_FMT_YUV420SP;
	}else if(strcmp(format_string, "YUV444") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "YUV444");
		img_format = CAMERA_PIX_FMT_YUV444;
	}else if(strcmp(format_string, "RGB888") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "RGB888");
		img_format = CAMERA_PIX_FMT_RGB888;
	}else if(strcmp(format_string, "RGB888P") == 0)
	{
		LOGE("Input %s equal to %s",format_string, "RGB888P");
		img_format = CAMERA_PIX_FMT_RGB888P;
	}else{
		LOGE("Not supprted format");
		img_format = CAMERA_PIX_FMT_MAX;
		throw LogicError(-1, "Parse image format failed");
	}

	LOGD("%s: Img Format: %d", __func__, img_format);

	return img_format;	
}


//enum Type { Invalid, Sensor, Image, Tpg, Max };

int from_str_to_input_type(const char * input_type){

	int input_type_format;

	if(strcmp(input_type, "Invalid") == 0)
	{
		LOGE("Input %s equal to %s",input_type, "Invalid");
		input_type_format = 0;
	}else if(strcmp(input_type, "Sensor") == 0)
	{
		LOGE("Input %s equal to %s",input_type, "Sensor");
		input_type_format = 1;
	}else if(strcmp(input_type, "Image") == 0)
	{
		LOGE("Input %s equal to %s",input_type, "Image");
		input_type_format = 2;
	}else if(strcmp(input_type, "Tpg") == 0)
	{
		LOGE("Input %s equal to %s",input_type, "Tpg");
		input_type_format = 3;
	}else if(strcmp(input_type, "Max") == 0)
	{
		LOGE("Input %s equal to %s",input_type, "Max");
		input_type_format = 4;
	}else{
		LOGE("Not supprted format");
		input_type_format = 4;
		throw LogicError(-1, "Parse input_type failed");
	}

	LOGD("%s: input_type_format: %d", __func__, input_type_format);

	return input_type_format;	
}


int vvb_parse_sensor_info(Json::Value &sensor_info, struct vvisp_sensor_cfg * sensor_cfg)
{
	int result = 0;
	LOGD("%s runs", __func__);

	if(sensor_cfg == NULL)
	{
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	std::string sensor_name = sensor_info["sensor_name"].asString();
	strcpy(sensor_cfg->sensor_name, sensor_name.c_str()); 
	LOGD("sensor name: %s, copied: %s", sensor_name.c_str(), sensor_cfg->sensor_name);

	std::string calib_name = sensor_info["calibration_name"].asString();
	strcpy(sensor_cfg->calibration_name, calib_name.c_str()); 
	LOGD("calib name: %s, copied: %s", calib_name.c_str(), sensor_cfg->calibration_name);

	unsigned int sensor_dev_id = sensor_info["sensor_dev_id"].asUInt();
	sensor_cfg->dev_id = sensor_dev_id; 
	LOGD("sensor_dev_id: %d", sensor_cfg->dev_id);

	int width = sensor_info["resolution_width"].asUInt();
	int height = sensor_info["resolution_height"].asUInt();

	sensor_cfg->sensor_width = width;
	sensor_cfg->sensor_height = height;
	LOGD("sensor resolution:%dx%d", sensor_cfg->sensor_width, sensor_cfg->sensor_height);

	return result;
}


int vvb_parse_func_info(Json::Value &func_info, struct vvisp_func_ctrl * func_cfg)
{
	int result = 0;
	LOGD("%s runs", __func__);

	if(func_cfg == NULL)
	{
		LOGE("%s INPUT NULL pointer", __func__);
		return -1;
	}

	func_info["aec_enable"].asUInt();
	func_info["awb_enable"].asUInt();
	Json::Value wdr3_enable = func_info["wdr3_enalbe"];
	if(wdr3_enable.isNull() == 1)
	{
		LOGE("wdr3 is empty");
	}else{
		LOGE("wdr3 is not empty, value:%d", wdr3_enable.asUInt());
	}
	//int wdr3_enable = 



	return result;
}



