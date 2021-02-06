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

#ifndef __VVBASE_H__
#define __VVBASE_H__
#include <exception>
#include <string>


#ifndef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#endif

/******************************************************************************
 * vvctx_t
 *****************************************************************************/
struct vvctx_t
{
	const char * cfg_json_file;
	const char * case_json_file;

    /* Driver based test. */

};

struct vvcfg_t
{
    /* Driver based test. */
    int cam_dev_id;
    unsigned int phy_address_base;
    unsigned int offset_used;
    unsigned int align_mask;
    unsigned int total_pool_size;
    int stream_duration;
};



struct LogicError : std::exception {
    LogicError(int32_t error, std::string message)
            : error(error), message(message) {}

    const char *what() const throw() override { return message.c_str(); }

    int32_t error;
    std::string message;
};


#endif //__VVBASE_H__