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

#ifndef __PYTOOL_HPP__
#define __PYTOOL_HPP__
#include "Python.h"

int py_init(void);
void py_deinit(void);

unsigned int nwl_csi_i2c_write(int bus_id, unsigned short slave_addr, unsigned int address, unsigned int data);
unsigned int nwl_csi_i2c_read(int bus_id, unsigned short slave_addr, unsigned int address);
unsigned int cam_app_la(int aa);
#endif //__PYTOOL_HPP__

