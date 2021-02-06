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
 * @file dom_ctrl_vidplay.h
 *
 * @brief
 *   Definition of DOM Ctrl Videoplayer.
 *
 *****************************************************************************/
/**
 * @page dom_ctrl_page DOM Ctrl
 * The Display Output Module Videoplayer displays image buffers in an X11 window.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref dom_ctrl_api
 * - @ref dom_ctrl_common
 * - @ref dom_ctrl
 *
 * @defgroup dom_ctrl_vidplay DOM Ctrl Videoplayer
 * @{
 *
 */

#ifndef __DOM_CTRL_VIDPLAY_H__
#define __DOM_CTRL_VIDPLAY_H__

#include <QPointer>
#include <QVideoFrame>


class VideoWidget;

class DomCtrlVidplay : public QObject
{
    Q_OBJECT

public:
    DomCtrlVidplay( QWidget *parent = 0, int posX = 0, int posY = 0, int width = 0, int height = 0 );
    ~DomCtrlVidplay();

    
    void presentFrame( const QVideoFrame *pFrame );
    void show();

//signals:
   
    //void sigPresentFrame( const QVideoFrame *pFrame );
    //void sigShow();
    

private:
    bool updateSize; //!< Set widget size according to first frame's size; used if either width and/or height are/is zero on construction

    QPointer<VideoWidget> pVideoWidget; //!< 0-guarded reference of the underlying video widget.
};

/* @} dom_ctrl_vidplay */

#endif // __DOM_CTRL_VIDPLAY_H__
