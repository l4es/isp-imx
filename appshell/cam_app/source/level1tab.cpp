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

#include "level1tab.h"

#include "level1/propertiessensorwidget.h"
#include "level1/registerwidget.h"
//#include "level1/calibdatawidget.h"

Level1Tab::Level1Tab( VirtualCamera* camDevice,  QWidget *parent )
  : QTabWidget(parent)
{
    RegisterWidget *registerWidget = new RegisterWidget( camDevice, this, true );
    addTab( registerWidget, tr("CamerIc Registers") );

#if 0
    if (camDevice->isBitstream3D())
    {
        RegisterWidget *registerWidget2 = new RegisterWidget( camDevice, this, false );
        addTab( registerWidget2, tr("CamerIc2 Registers") );
    }
#endif

    PropertiesSensorWidget *sensorWidget = new PropertiesSensorWidget( camDevice, this, true );
    addTab( sensorWidget, tr("Sensor Registers") );

#if 0
    PropertiesSensorWidget *sensorWidget2 = NULL;
    if (camDevice->isBitstream3D())
    {
        sensorWidget2 = new PropertiesSensorWidget( camDevice, this, false );
        addTab( sensorWidget2, tr("Sensor2 Registers") );
    }
#endif
#if 0
    CalibDataWidget *calibDataWidget = new CalibDataWidget( camDevice, this );
    addTab( calibDataWidget, tr("Calibration Data") );

    // connect actions
    connect( parent , SIGNAL( opened() ), calibDataWidget, SLOT( opened() ) );
    connect( parent , SIGNAL( closed() ), calibDataWidget, SLOT( closed() ) );

    connect( parent , SIGNAL( opened()       ), sensorWidget, SLOT( opened()       ) );
    connect( parent , SIGNAL( closed()       ), sensorWidget, SLOT( closed()       ) );
    connect( parent , SIGNAL( connected()    ), sensorWidget, SLOT( connected()    ) );
    connect( parent , SIGNAL( disconnected() ), sensorWidget, SLOT( disconnected() ) );
    connect( parent , SIGNAL( started()      ), sensorWidget, SLOT( started()      ) );
    connect( parent , SIGNAL( stopped()      ), sensorWidget, SLOT( stopped()      ) );
    connect( parent , SIGNAL( resChanged()   ), sensorWidget, SLOT( resChanged()   ) );
#endif
}
