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
 * @file cam_calibdb.c
 *
 * @brief
 *   Implementation of the CamEngine.
 *
 *****************************************************************************/
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>

#include <common/utl_fixfloat.h>

/******************************************************************************
 * local macro definitions
 *****************************************************************************/
CREATE_TRACER( CAM_CALIBDB_INFO , "CAM-CALIBDB: ", INFO   , 0 );
CREATE_TRACER( CAM_CALIBDB_WARN , "CAM-CALIBDB: ", WARNING, 1 );
CREATE_TRACER( CAM_CALIBDB_ERROR, "CAM-CALIBDB: ", ERROR  , 1 );
CREATE_TRACER( CAM_CALIBDB_DEBUG, ""             , INFO   , 0 );


