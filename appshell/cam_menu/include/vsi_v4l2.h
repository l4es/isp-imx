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

#ifndef CAM_MENU_INCLUDE_VSI_V4L2_H_
#define CAM_MENU_INCLUDE_VSI_V4L2_H_

#define false -1

enum OUTTYPE {
        OUTPUTDRM = 1,
        OUTPUTFB,
        SAVEIMAGE,
};

extern int Width;
extern int Height;

/*****************************************************************************/
/**
 * @brief   Short description.
 *
 * Some detailed description goes here ...
 *
 * @param   param1      Describe the parameter 1.
 *
 * @return              Return the result of the function call.
 * @retval              RET_VAL1
 * @retval              RET_VAL2
 *
 *****************************************************************************/
extern int v4l2_openCamera(void);

extern int v4l2_querycap(void);

extern int v4l2_enum_fmt(void);

extern int v4l2_set_fmt(unsigned int width, unsigned int height);

extern int v4l2_init_mmap(unsigned int bufferCount);

extern int v4l2_start_capture(void);

extern int v4l2_stop_capture(void);

extern int v4l2_closeCamera(void);

extern void v4l2_process_image(unsigned int imageProcess);

extern void display_image(void);

#endif  // CAM_MENU_INCLUDE_VSI_V4L2_H_

