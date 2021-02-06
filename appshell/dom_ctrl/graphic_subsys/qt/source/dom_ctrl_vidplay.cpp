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
 * @file dom_ctrl_vidplay.cpp
 *
 * @brief
 *   Implementation of DOM Ctrl DomCtrlVidplay.
 *
 *****************************************************************************/
/**
 * @page dom_ctrl_page DOM Ctrl
 * The Display Output Module DomCtrlVidplay displays image buffers in an X11 window.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref dom_ctrl_api
 * - @ref dom_ctrl_common
 * - @ref dom_ctrl
 *
 * @defgroup dom_ctrl_vidplay DOM Ctrl DomCtrlVidplay
 * @{
 *
 */

#include "dom_ctrl_vidplay.h"
#include "videowidget.h"
#include <QtMultimedia>

#include <QMediaPlayer>
#include <QMovie>

#include <QtWidgets>
#include <qvideosurfaceformat.h>

#define qDebug(...)


DomCtrlVidplay::DomCtrlVidplay( QWidget *parent, int posX, int posY, int width, int height )
    : QObject( parent )
{
    // create video widget
    // this will only succeed if widget is created in main/gui thread; that is,
    // we have to be running in main/GUI thread's context already; for non-widget style
    // objects it would be possible, to change thread affinity afterwards
    pVideoWidget = new VideoWidget( parent );

    // connect our signals to the corresponding slots of the video widget
    bool success;
    qRegisterMetaType<QVideoFrame*>("QVideoFrame*");


#if 0
    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    
    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pVideoWidget);
    layout->addLayout(controlLayout);
    setLayout(layout);
#endif
    // show widget
    //pVideoWidget->show();
}

DomCtrlVidplay::~DomCtrlVidplay()
{
    // trigger deferred delete of video widget
}


void DomCtrlVidplay::presentFrame( const QVideoFrame *pFrame )
{

    // enqueue presentFrame signal & wait for completion (BlockingQueuedConnection)
    qDebug( "signal present frame!\n");
    pVideoWidget->presentFrame(pFrame);
}


void DomCtrlVidplay::show()
{
    // enqueue clearDisplay signal & wait for completion (BlockingQueuedConnection)
    /*emit sigClearDisplay();*/
    pVideoWidget->show();
}

#if 0
void DomCtrlVidplay::clearDisplay()
{
    // enqueue clearDisplay signal & wait for completion (BlockingQueuedConnection)
    /*emit sigClearDisplay();*/
    pVideoWidget->clearDisplay();
}



void DomCtrlVidplay::setOverlayText( QString &overlayText )
{
    // enqueue setOverlayText signal & wait for completion (BlockingQueuedConnection)
    emit sigSetOverlayText( &overlayText );
}
#endif
/* @} dom_ctrl_vidplay */
