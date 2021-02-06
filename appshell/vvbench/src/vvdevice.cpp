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
#include "vvbase.hpp"
#include <stdio.h>
#include <json/json.h>

#define LOGTAG "DEV"
#include "vlog.h"

#include <unistd.h>
#include "vvdevice.hpp"
#include "vvbparser.hpp"
 
void vvdevice(void)
{
//	int a = 0;
}

void vvdevice_delay(int delay_interval)
{
//	int a = 0;
	sleep(delay_interval);
}

int get_hw_resources(CAM_DEVICE_ID isp_id)
{
	int result;
	Json::Value request;
	Json::Value response;
	result = CAM_DEVICE::getHwResources(request, response);
	if(result != 0)
	{
		LOGE("getHwResources failed");
		return -1;
	}

	if(response["resource"].isNull() == 1)
	{
		LOGE("getHwResources failed");
		return -1;
	}

	int isp_max = response["resource"].asUInt();
	if (isp_id > isp_max)
	{
		LOGE("resource count failed");
		return -1;
	}
	return result;
}

int check_hw_status(CAM_DEVICE_ID isp_id)
{
	int result;
	Json::Value request;
	Json::Value response;
	result = CAM_DEVICE::getHwStatus(request, response);
	if(result != 0)
	{
		LOGE("getHwStatus failed");
		return -1;
	}

	if(response["status"].isNull() == 1)
	{
		LOGE("getHwStatus failed");
		return -1;
	}

	int isp_status = response["status"].asUInt();
	if (isp_status != 1)
	{
		LOGE("resource inuse,return");
		return -1;
	}

	return result;
}

int get_isp_features(CAM_DEVICE_ID isp_id)
{
	int result;
	Json::Value request;
	Json::Value response;
	result = CAM_DEVICE::getIspFeature(request, response);
	if(result != 0)
	{
		LOGE("getIspFeature failed");
		return -1;
	}

	if(response["feature"].isNull() == 1)
	{
		LOGE("getIspFeature failed");
		return -1;
	}

	unsigned int isp_feature = response["status"].asUInt();
	LOGD("isp feature: %d", isp_feature);

	//Todo: print mask bits.

	return result;
}


int get_isp_versions(void)
{
	int result;
	Json::Value request;
	Json::Value response;
	
	result = CAM_DEVICE::getVersions(request, response);
	if(result != 0)
	{
		LOGE("getIspFeature failed");
		return -1;
	}

	if(response["native_api_version"].isNull() == 1)
	{
		LOGE("getIspFeature failed");
		return -1;
	}

	std::string versions = response["native_api_version"].asString();
	LOGD("ISP_Versions: %s", versions.c_str());

	return result;
}

//class vvisp_dev mdev;
int execute_caseline(struct vvdev_t * case_ctx, struct vvcfg_t * cfg_ctx)
{
    unsigned int total_pool_size_each_ch;
    unsigned int phy_address_base_ch;

	if((case_ctx == NULL )||(cfg_ctx == NULL))
	{
		LOGE("%s: NULL pointer", __func__);
		return VVCASE_EXEC_FAIL;
	}

	int total_isp = case_ctx->total_isp;
	if (total_isp > CAM_ISPCORE_ID_MAX)
	{
		total_isp = CAM_ISPCORE_ID_MAX;
	}

	//init memory allocations
	cfg_ctx->stream_duration = 0;
    total_pool_size_each_ch = (total_isp > 0) ? cfg_ctx->total_pool_size/total_isp : cfg_ctx->total_pool_size;

    class vvisp_dev mdev(case_ctx);

	for(int index = 0; index < total_isp; index++)
	{
		CAM_DEVICE_ID isp_id = case_ctx->isp_cfg_ctx[index].isp_id;
		if(case_ctx->isp_cfg_ctx[index].isp_enable  == 0 )
		{
			
			LOGD("Skip isp %d, loop:%d", isp_id, index);
			continue;
		}

		LOGE("Testing ISP %d", isp_id);
		phy_address_base_ch = cfg_ctx->phy_address_base + (unsigned int)isp_id*total_pool_size_each_ch;
		cfg_ctx->offset_used = 0;

		try
		{
			//get common parameters for case settings
			if (case_ctx->isp_cfg_ctx[index].stream_duration > cfg_ctx->stream_duration)
			{
				LOGE("Update streaming duration %d to case cfg from ISP_id %d", case_ctx->isp_cfg_ctx[index].stream_duration, isp_id);
				cfg_ctx->stream_duration = case_ctx->isp_cfg_ctx[index].stream_duration;
			}

			//query hw resources
			int result;
			result = get_hw_resources(isp_id);
			if(result != 0)
			{
				LOGE("get_hw_resources error, exit");
				return -1;
			}

			result = check_hw_status(isp_id);
			if(result != 0)
			{
				LOGE("check_hw_status error, exit");
				return -1;
			}

			result = get_isp_features(isp_id);
			if(result != 0)
			{
				LOGE("get_isp_features error, exit");
				return -1;
			}

			result = get_isp_versions();
			if(result != 0)
			{
				LOGE("get_isp_versions error, exit");
				return -1;
			}

			//Open isp for initials
			result = mdev.m_cam_dev[index].initHardware(case_ctx->isp_cfg_ctx[index].isp_id, &mdev.dev_handle[index]);
			if(result != 0)
			{
				LOGE("open camera error, release cam_handler");
				CAM_DEVICE * pcam_dev = (CAM_DEVICE *)mdev.dev_handle[index];
				pcam_dev->releaseHardware();
				return -1;
			}

			{
				Json::Value jRequest, jResponse;
				jRequest[DEVICE_CALIBRATION_FILE_PARAMS] = case_ctx->isp_cfg_ctx[index].sensor_cfg.calibration_name;
				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT failed for xml file:%s", case_ctx->isp_cfg_ctx[index].sensor_cfg.calibration_name);
					return -1;
				}
			}

			BUFF_MODE buff_mode = case_ctx->isp_cfg_ctx[index].buff_mode;
			result = mdev.m_cam_dev[index].setBufferParameters(buff_mode);
			if(result != 0)
			{
				LOGE("setBufferParameters set buff_mode %d error!", buff_mode);
				return -1;
			}

			int output_en = 0;
			for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
			{

				if(case_ctx->isp_cfg_ctx[index].isp_path[i].path_enable == 0)
				{
					continue;
				}else{
					if(i < ISPCORE_BUFIO_WRITEMAX)
					{
						output_en = 1;
					}
				}

				ISPCORE_BUFIO_ID bufio = case_ctx->isp_cfg_ctx[index].isp_path[i].path;
				LOGD("allocation for path %d", bufio);
				result = mdev.m_cam_dev[index].initBufferPoolCtrl(bufio);
				if(result != 0)
				{
					LOGE("initBufferPoolCtrl set buf_io %d error!", bufio);
					return -1;
				}


				//add buffer address.
				int buffer_max = case_ctx->isp_cfg_ctx[index].isp_path[i].buffer_number;
				if(buffer_max >= MAX_BUFFER_QUEUES)
				{
					LOGW("buffer number %d too big, clip to %d", buffer_max, MAX_BUFFER_QUEUES);
					buffer_max = MAX_BUFFER_QUEUES;
				}

				mdev.io_buf[index].ctx[i].buffer_number = buffer_max;
				int buffer_size = case_ctx->isp_cfg_ctx[index].isp_path[i].buffer_size;
				mdev.io_buf[index].ctx[i].buffer_size = buffer_size;


				for(int buf_idx = 0; buf_idx < buffer_max; buf_idx ++)
				{
					BufIdentity * p_buf = new BufIdentity;

					unsigned int allocate_buf_size = ALIGN_UP(buffer_size, cfg_ctx->align_mask);
					//phy address allocator
					if((cfg_ctx->offset_used + allocate_buf_size) > total_pool_size_each_ch)
					{
						LOGE("Overflow memory assignment, exit");
						return -1;
					}

					unsigned int buffer_address_reg = phy_address_base_ch + cfg_ctx->offset_used;
					cfg_ctx->offset_used = cfg_ctx->offset_used + allocate_buf_size;

					p_buf->buff_size = allocate_buf_size;
					p_buf->address_reg = buffer_address_reg;
					p_buf->buffer_idx = buf_idx;
					//todo: user to map it

					LOGD("buf[%d]: Phy_Addr:0x%x, base:0x%x, used:0x%x, size:0x%x", buf_idx, p_buf->address_reg, phy_address_base_ch, cfg_ctx->offset_used, p_buf->buff_size);
					mdev.io_buf[index].ctx[i].buf[buf_idx] = p_buf;

					result = mdev.m_cam_dev[index].bufferPoolAddEntity(bufio, p_buf);
					if(result != 0)
					{
						LOGE("bufferPoolAddEntity for buf_io:%d error, buf_idx:%d", bufio, buf_idx);
						return -1;
					}

				}

				result = mdev.m_cam_dev[index].bufferPoolKernelAddrMap(bufio, true);
				if(result != 0)
				{
					LOGE("bufferPoolKernelAddrMap failed for bifio:%d", bufio);
					return -1;
				}
	
				result = mdev.m_cam_dev[index].bufferPoolSetBufToEngine(bufio);
				if(result != 0)
				{
					LOGE("bufferPoolSetBufToEngine failed for bifio:%d", bufio);
					return -1;
				}


			}

			if(0 == output_en)
			{
				LOGE("At least one of ISPCORE_BUFIO need to be enabled\n");
				return -1;
			}

			{
				Json::Value jRequest, jResponse;
				jRequest[SENSOR_SENSOR_DRIVER_PARAMS] = case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name;
				jRequest[SENSOR_SENSOR_DEVICEID] = case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id;
				jRequest[SENSOR_SENSOR_I2C_NUMBER] = 0; //tbd remove

				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_SENSOR_OPEN, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_SENSOR_OPEN failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}
			}

			//get sensor capabilities
			{
				Json::Value jRequest, jResponse;
				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_SENSOR_CAPS, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_SENSOR_CAPS failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}
				//binDecode(jRequest[SENSOR_CAPS_PARAMS_BASE64], mdev.caps[index]);
			}


			//ISPCORE_MODULE_DEVICE_INPUTINFO, query for input info
			{
				//ISPCORE_MODULE_DEVICE_INPUTINFO
				Json::Value jRequest, jResponse;

				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_INPUTINFO, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_INPUTINFO failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}

				int sensor_size = jResponse[DEVICE_COUNT_PARAMS].asInt();
				int input_type = jRequest[DEVICE_INDEX_PARAMS].asInt();
				LOGD("INPUTINFO Result, input channed:%d, sensor_size:%d", input_type, sensor_size);
			}

			for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
			{
				if(case_ctx->isp_cfg_ctx[index].isp_path[i].path_enable == 0)
				{
					continue;
				}else{
					int ret = 0;
					
					std::function<int(MediaBuffer_t * buf, int show_channel)> callback;
					ISPCORE_BUFIO_ID bufio = case_ctx->isp_cfg_ctx[index].isp_path[i].path;

					switch(bufio)
					{

						case ISPCORE_BUFIO_MP:
							callback = std::bind(&vvisp_dev::mp_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;

						case ISPCORE_BUFIO_SP1:
							callback = std::bind(&vvisp_dev::sp1_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;

						case ISPCORE_BUFIO_SP2:
							callback = std::bind(&vvisp_dev::sp2_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;

						case ISPCORE_BUFIO_RDI:
							callback = std::bind(&vvisp_dev::dummy_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;

						case ISPCORE_BUFIO_META:
							callback =  std::bind(&vvisp_dev::dummy_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;

						default:
							callback = std::bind(&vvisp_dev::dummy_call_back, &mdev, std::placeholders::_1, std::placeholders::_2);
						break;
					}

        			int frame_skip_interval = 0;
        			result = mdev.m_cam_dev[index].initOutChain(bufio, frame_skip_interval);
        			if(result != 0)
        			{
        				LOGE("initOutChain failed for bifio:%d", bufio);
        				return -1;
        			}

        			result = mdev.m_cam_dev[index].startOutChain(bufio);
        			if(result != 0)
        			{
        				LOGE("startOutChain failed for bifio:%d", bufio);
        				return -1;
        			}
        			result = mdev.m_cam_dev[index].attachChain(bufio);
        			if(result != 0)
        			{
        				LOGE("attachChain failed for bifio:%d", bufio);
        				return -1;
        			}
        			

					//register callback
					ret = mdev.registerBufferCallback(callback, isp_id, bufio);
					if(ret != 0)
					{
						LOGE("%s: registerBufferCallback Fails %d, return", __func__, ret);
						return -1;
					}

					//start the callback

					ret = mdev.buf_handle_start(isp_id, bufio);
					if(ret != 0)
					{
						LOGE("%s: buf_handle_start Fails %d, return", __func__, ret);
						return -1;
					}
				}
			}
#if 0 
			//select from input switch, tbd extended
			{
				//ISPCORE_MODULE_DEVICE_INPUTSWITCH
				Json::Value jRequest, jResponse;
				int input_type_val = from_str_to_input_type(case_ctx->isp_cfg_ctx[index].input_type);
				LOGE("setting input type: %s-->%d", case_ctx->isp_cfg_ctx[index].input_type, input_type_val);
				jRequest[DEVICE_INDEX_PARAMS] = input_type_val;

				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_INPUTSWITCH, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_INPUTSWITCH failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}

				int input_type = jResponse[DEVICE_INPUT_TYPE_PARAMS].asInt();
				std::string sensor_name = jResponse[DEVICE_CALIBRATION_FILE_PARAMS].asString();
    			LOGE("INPUT SWITCH Result, input channed:%d, sensor name:%s", input_type, sensor_name.c_str());
			}

#endif
            {
                Json::Value jRequest, jResponse;
                jRequest["calibration.file"] = "OV2775.xml";
                mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_CALIBRATIONLOADANDINIT, jRequest, jResponse);
            }
            //Todo: separate multiple path formats
            {
                Json::Value jRequest, jResponse;
                jRequest[SENSOR_SENSOR_DRIVER_PARAMS] = "OV2775.drv";
                mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_SENSOR_OPEN, jRequest, jResponse);
            }
            
            
            {
                Json::Value jRequest, jResponse;
                jRequest[DEVICE_CAMCONNECT_PREVIEW] = true;
                jRequest[DEVICE_WIDTH_PARAMS] = case_ctx->isp_cfg_ctx[index].isp_path[ISPCORE_BUFIO_MP].width;  //TODO: fix for other paths
                jRequest[DEVICE_HEIGHT_PARAMS] =  case_ctx->isp_cfg_ctx[index].isp_path[ISPCORE_BUFIO_MP].height;
                jRequest[DEVICE_FORMAT_PARAMS] = case_ctx->isp_cfg_ctx[index].isp_path[ISPCORE_BUFIO_MP].format;
                jRequest[DEVICE_FRAME_PARAMS] = 0;
                jRequest[HDR_ENABLE_PARAMS] = false;
                jRequest[SENSOR_BAYER_PATTERN_PARAMS] = 2;//Todo: move later ISI_BPAT_GRGRBGBG;
            
            
                result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_CAMERA_CONNECT, jRequest, jResponse);
                if(result != 0)
                {
                    LOGE("ISPCORE_MODULE_DEVICE_CAMERA_CONNECT failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
                    return -1;
                }
            }

			//start preview
//			ISPCORE_MODULE_DEVICE_PREVIEW_START
			{
				Json::Value jRequest, jResponse;
				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_PREVIEW_START, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_PREVIEW_START failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}
			}
			//Register call backs.


		} catch (const std::exception &e) {
			LOGE("ERR: %s \n", e.what());
			return -1;
		}
	}


	//Add time delay for preview durations (currently find max time for both ISP cfg)

	LOGE("vvbench stream on duration:%d", cfg_ctx->stream_duration);
	vvdevice_delay(cfg_ctx->stream_duration );


     LOGE("vvbench end of stream");

	//Stop ISPs
	for(int index = 0; index < total_isp; index++)
	{
		CAM_DEVICE_ID isp_id = case_ctx->isp_cfg_ctx[index].isp_id;
		if(case_ctx->isp_cfg_ctx[index].isp_enable  == 0 )
		{
			
			LOGD("Skip isp %d, loop:%d", isp_id, index);
			continue;
		}

		LOGE("Testing ISP %d", isp_id);

		try
		{
			//query hw resources
			int result;

			//ISPCORE_MODULE_DEVICE_PREVIEW_STOP
			{
				Json::Value jRequest, jResponse;
				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_PREVIEW_STOP, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_PREVIEW_STOP failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}
			}

			//ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT
			{
				Json::Value jRequest, jResponse;
				result = mdev.m_cam_dev[index].ioctl(ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT, jRequest, jResponse);
				if(result != 0)
				{
					LOGE("ISPCORE_MODULE_DEVICE_CAMERA_DISCONNECT failed for sensor name:%s, dev_id:%d", case_ctx->isp_cfg_ctx[index].sensor_cfg.sensor_name, case_ctx->isp_cfg_ctx[index].sensor_cfg.dev_id);
					return -1;
				}
			}

			//stop call backs.
			for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
			{
				if(case_ctx->isp_cfg_ctx[index].isp_path[i].path_enable == 0)
				{
					continue;
				}else{
					//stop the callback
					mdev.buf_handle_stop(isp_id);
				}
			}

			vvdevice_delay(1);

			for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
			{
				if(case_ctx->isp_cfg_ctx[index].isp_path[i].path_enable == 0)
				{
					continue;
				}else{
					//stop the callback
					mdev.mThread[isp_id].join();
				}
			}

			//deattach buffer chain
			for(int i = 0; i < ISPCORE_BUFIO_MAX; i ++)
			{

				if(case_ctx->isp_cfg_ctx[index].isp_path[i].path_enable == 0)
				{
					continue;
				}else{
					ISPCORE_BUFIO_ID bufio = case_ctx->isp_cfg_ctx[index].isp_path[i].path;

					result = mdev.m_cam_dev[index].detachChain(bufio);
					if(result != 0)
					{
						LOGE("detachChain failed for bifio:%d", bufio);
						return -1;
					}

					result = mdev.m_cam_dev[index].stopOutChain(bufio);
					if(result != 0)
					{
						LOGE("stopOutChain failed for bifio:%d", bufio);
						return -1;
					}

					result = mdev.m_cam_dev[index].deInitOutChain(bufio);
					if(result != 0)
					{
						LOGE("deInitOutChain failed for bifio:%d", bufio);
						return -1;
					}
				
					result = mdev.m_cam_dev[index].bufferPoolClearBufList(bufio);
					if(result != 0)
					{
						LOGE("bufferPoolClearBufList failed for bifio:%d", bufio);
						return -1;
					}
				}
			}

			result = mdev.m_cam_dev[index].releaseHardware();
			if(result != 0)
			{
				LOGE("releaseHardware camera error return");
				return -1;
			}

		} catch (const std::exception &e) {
			LOGE("ERR: %s \n", e.what());
			return -1;
		}
	}

	return 0;
}


vvisp_dev::vvisp_dev(struct vvdev_t * case_ctx) {
    if(case_ctx)
    {
        for(int i = 0; i < MAX_CAM_NUM; i++)
        {
            if(case_ctx->isp_cfg_ctx[i].isp_enable)
            {
                initDom(case_ctx->isp_cfg_ctx[i].isp_id);
            }
        }
    }
    else
    {
        initDom(CAM_ISPCORE_ID_0);
    }

}

vvisp_dev::~vvisp_dev() {
    for(int i = 0; i < MAX_CAM_NUM; i++)
    {
        if (dom[i]) {
            domCtrlStop(dom[i]);
            domCtrlShutDown(dom[i]);
        }
    }
    
}


void vvisp_dev::initDom(CAM_DEVICE_ID channel) {
    domCtrlConfig_t ctrlConfig;
    memset( &ctrlConfig, 0, sizeof( domCtrlConfig_t ));

    ctrlConfig.domId                = (uint32_t)channel;
    ctrlConfig.MaxPendingCommands   = 10;
    ctrlConfig.MaxBuffers           = 1;
    ctrlConfig.domCbCompletion      = NULL;
    ctrlConfig.pUserContext         = (void *)this;
    ctrlConfig.hParent              = NULL;
    ctrlConfig.width                = 0;
    ctrlConfig.height               = 0;
    ctrlConfig.ImgPresent           = DOMCTRL_IMAGE_PRESENTATION_3D_VERTICAL;
    ctrlConfig.domCtrlHandle        = NULL;

    
    domCtrlInit( &ctrlConfig );
    dom[channel] = ctrlConfig.domCtrlHandle;
    domCtrlStart(dom[channel]);

}

BufIdentity outBuf[MAX_CAM_NUM];
void vvisp_dev::showBuffer(MediaBuffer_t * pBuffer, int show_channel) {
    PicBufMetaData_t *pPicBufMetaData = (PicBufMetaData_t *)(pBuffer->pMetaData);
    int width = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicWidthPixel;
    int height = pPicBufMetaData->Data.YCbCr.semiplanar.Y.PicHeightPixel;
    int size = width * height * 2; // YUV422SP
    if(show_channel >= MAX_CAM_NUM)
    {
        return;
    }
    
    outBuf[show_channel].width = width;
    outBuf[show_channel].height = height;
    outBuf[show_channel].format = 0;
    outBuf[show_channel].buff_size = size;
    outBuf[show_channel].address_usr = (uint64_t)pBuffer->baseAddress;
    domCtrlShowBuffer(dom[show_channel], &outBuf[show_channel]);
}

int vvisp_dev::buf_handle_start(int isp_id, ISPCORE_BUFIO_ID buf_io)
{
	int result = 0;
	running_flag[isp_id] = 1;
	LOGI("ispid %d %d", isp_id, (int) buf_io);
	mThread[isp_id] = std::thread(
          [=]
          () { 
           this->buf_handle(isp_id, buf_io); 
        }
    );
	return result;
}

void  vvisp_dev::buf_handle_stop(int isp_id)
{
	//without mutex now.
	running_flag[isp_id] = 0;
	//vvdevice_delay(1);  //waitting for buf_handler exit.
}

int vvisp_dev::registerBufferCallback(std::function<int(MediaBuffer_t * buf, int show_channel)> callback, int isp_id, ISPCORE_BUFIO_ID buf_io)
{
	if((isp_id >= MAX_CAM_NUM) ||(buf_io >= ISPCORE_BUFIO_WRITEMAX))
	{
		return -1;
	}

	io_buf[isp_id].ctx[buf_io].callback = callback;
	return 0;
}

void vvisp_dev::buf_handle(int isp_id, ISPCORE_BUFIO_ID buf_io)
{

	buffCtrlEvent_t buffEvent;
	MediaBuffer_t * pBuf;
	LOGI("ispid %d %d", isp_id, (int) buf_io);

	while (running_flag[isp_id])
	{
		int ret;
		ret = m_cam_dev[isp_id].waitForBufferEvent(buf_io, &buffEvent, 100);
		if (ret !=0 ) {
		//LOGE("%s waitForBufferEvent timeout", __func__);
			continue;
		}

		LOGI("ispid: %d", isp_id);
		if(buffEvent.eventID == BUFF_CTRL_CMD_STOP) {
			LOGW("get BUFF_CTRL_CMD_STOP: %d", ret);
			break;
		}


		if(buffEvent.eventID != BUFF_CTRL_CMD_BUFFER_READY) {
			LOGW("get BUFF_CTRL_CMD_READY: %d", ret);
			continue;
		}


		ret = m_cam_dev[isp_id].DQBUF(buf_io, &pBuf);
		if (0 != ret) {
			LOGW("%s DQBUF failed: %d", __func__, ret);
			continue;
		}

		if(io_buf[isp_id].ctx[buf_io].callback != NULL)
		{
			//calling call back functions
			ret = io_buf[isp_id].ctx[buf_io].callback(pBuf, isp_id);
			if(ret != 0)
			{
				LOGW("%s callback failed: %d", __func__, ret);
				ret = m_cam_dev[isp_id].QBUF(buf_io, pBuf);
				if (0 != ret) {
					LOGW("%s QBUF failed: %d", __func__, ret);
				running_flag[isp_id] = 0;
				return;
				}
			}

		}else{
			LOGE("@@Null callback for ISP %d, channel:%d, QBuf..", isp_id, buf_io);
		}

		//return buffer to queue
		ret = m_cam_dev[isp_id].QBUF(buf_io, pBuf);
		if (0 != ret) {
			LOGW("%s QBUF failed: %d", __func__, ret);
			continue;
		}

	}

	return;
}

int vvisp_dev::mp_call_back(MediaBuffer_t * buf, int show_channel){
	int ret = 0;
	LOGW("%s in, process buffer", __func__);
//	mdev.showBuffer(buf, show_channel);
    showBuffer(buf, show_channel);

	return ret;
}

int vvisp_dev::sp1_call_back(MediaBuffer_t * buf, int show_channel){
	int ret = 0;
	LOGW("%s in, process buffer", __func__);

	return ret;
}

int vvisp_dev::sp2_call_back(MediaBuffer_t * buf, int show_channel){
	int ret = 0;
	LOGW("%s in, process buffer", __func__);

	return ret;
}

int vvisp_dev::dummy_call_back(MediaBuffer_t * buf, int show_channel){
	int ret = 0;
	LOGW("%s in, process buffer", __func__);

	return ret;
}

