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


#ifndef __LEVEL_1_TAB_H__
#define __LEVEL_1_TAB_H__

#include <QtWidgets/QTabWidget>
#include <VirtualCamera.h>



class Level1Tab : public QTabWidget
{
    Q_OBJECT

    public:
        Level1Tab( VirtualCamera* camDevice, QWidget *parent = 0 );
};

#endif /* __LEVEL_1_TAB_H__ */
