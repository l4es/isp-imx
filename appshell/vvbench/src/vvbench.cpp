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
#include <stdio.h>
#include <json/json.h>

#define LOGTAG "MAIN"
#include "vlog.h"

#include <unistd.h>
#include "vvbparser.hpp"

int vvbench_log_level = -1;

int main( int argc, char *argv[] )
{
	Json::Value cfg_json_value;
	Json::Value case_json_value;

	struct vvctx_t vvctx;
	struct vvcfg_t vvcfg;

	int ret = 0;

	LOGW("@@vvbench started@@");
	vvbench_version();

	if (argc > 3) {
	usage();
		exit(1);
	}

	if (argc > 1) {
		vvctx.cfg_json_file = argv[1];
		if(vvctx.cfg_json_file != NULL)
		{
			LOGW("USE configure file: %s", vvctx.cfg_json_file);
		}
	}
	if (argc > 2) {
		vvctx.case_json_file = argv[2];
		if(vvctx.case_json_file != NULL)
		{
			LOGW("USE case list file: %s", vvctx.case_json_file);
		}
	}

	if(argc == 1)
	{
		vvctx.cfg_json_file = "vvbcfg/vvbench_settings.json";
		vvctx.case_json_file =  "vvbcfg/vvbench_list.json";
		LOGW("USE default configure file: %s", vvctx.cfg_json_file);
		LOGW("USE default case list file: %s", vvctx.case_json_file);
	}

	LOGW("Parse configuration settings...");

	ret = vvb_config_parser(vvctx.cfg_json_file, &vvcfg);
	if(ret != 0)
	{
		LOGE("Parser config file error, exit");
		exit(1);
	}

	ret = vvb_list_execute(vvctx.case_json_file, &vvcfg);
	if(ret != 0)
	{
		LOGE("execute list error, exit");
		exit(1);
	}


	return 0;
}

void usage(void)
{
    LOGE(" vvbench command:");
    LOGE("    vvbench: execute vvbench by default config files");
    LOGE("    vvbench new_config_file.json");
	LOGE("    vvbench new_config_file.json case_name.json");
}

void vvbench_version(void)
{
    LOGW("--------------------------------------");
    LOGW("ISP Pipeline CFG: %s", ISP_VERSION);
	LOGW("VVBench Version:%s", VVBENCH_VERSION);
    LOGW("Build time: %s, %s", __DATE__, __TIME__);
    LOGW("--------------------------------------");
}


