/******************************************************************************\
|* Copyright 2010, Dream Chip Technologies GmbH. used with permission by      *|
|* VeriSilicon.                                                               *|
|* Copyright (c) <2020> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

/* VeriSilicon 2020 */

/**
 * @bufsync_ctrl_cb.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>

#include <common/return_codes.h>
#include <common/picture_buffer.h>

#include <bufferpool/media_buffer.h>
#include <bufferpool/media_buffer_pool.h>
#include <bufferpool/media_buffer_queue_ex.h>

#include "bufsync_ctrl_cb.h"
#include "bufsync_ctrl.h"



/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( BUFSYNC_CTRL_CB_INFO , "BUFSYNC-CTRL-CB: ", INFO      , 1 );
CREATE_TRACER( BUFSYNC_CTRL_CB_WARN , "BUFSYNC-CTRL-CB: ", WARNING   , 1 );
CREATE_TRACER( BUFSYNC_CTRL_CB_ERROR, "BUFSYNC-CTRL-CB: ", ERROR     , 1 );

CREATE_TRACER( BUFSYNC_CTRL_CB_DEBUG, "", INFO, 0 );

