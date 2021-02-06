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

#ifndef __MAIN_H__
#define __MAIN_H__


#include <QObject>
#include <QThread>
#include <QEvent>
#include "dom_ctrl_vidplay_api.h"

/******************************************************************************
 * class MyVidplayTest
 *****************************************************************************/
class MyVidplayTest : public QThread
{
    Q_OBJECT

public:
    MyVidplayTest( QObject *parent = 0 );
    void run();
private slots:
    void create();
    void display();
    void clear();
    void destroy();
};

#endif // __MAIN_H__