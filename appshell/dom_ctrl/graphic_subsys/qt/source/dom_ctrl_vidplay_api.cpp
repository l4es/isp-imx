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
 * @file dom_ctrl_vidplay_api.cpp
 *
 * @brief
 *   Implementation of DOM Ctrl Videoplayer API.
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
 * @defgroup dom_ctrl_vidplay_api DOM Ctrl Videoplayer API
 * @{
 *
 */

#include <appshell_ebase/types.h>
#include <appshell_ebase/trace.h>

#include <appshell_oslayer/oslayer.h>
#include "dom_ctrl_vidplay_api.h"
#include "dom_ctrl_vidplay.h"

#include "extmemvideobuffer.h"
#include "videowidget.h"

#ifdef FB_ENABLE
#include "video_frame_buffer_display.h"
#endif

#include <QPointer>
#include <QtWidgets/QApplication>

#include <QInternal>

#include <QTime> // for performance evaluation only; see domCtrlVidplayDisplay() below

CREATE_TRACER(DOM_CTRL_QT_INFO , "DOM-CTRL-QT: ", INFO,  0);
CREATE_TRACER(DOM_CTRL_QT_ERROR, "DOM-CTRL-QT: ", ERROR, 1);


/******************************************************************************
 * local macro definitions
 *****************************************************************************/


/******************************************************************************
 * local type definitions
 *****************************************************************************/
typedef struct domCtrlVidplayHandle_s
{
    QPointer<DomCtrlVidplay>    pVidplay;           //!< 0-guarded reference of the underlying video player.

    domCtrlVidplayConfig_t      Config;             //!< Local copy of the incoming config structure; used by @ref domCtrlVidplayCreatePlayer().

    bool                        HaveMainAppThread;  //!< We need this only if we have to create the Qt main application; see @ref pMainApp.
    osThread                    MainAppThread;      //!< We need this only if we have to create the Qt main application; see @ref pMainApp.
    osEvent                     MainAppEvent;       //!< We need this only if we have to create the Qt main application; see @ref pMainApp.
    int                         argc;               //!< We need this only if we have to create the Qt main application; see @ref pMainApp.
    char                        *argv[1];           //!< We need this only if we have to create the Qt main application; see @ref pMainApp.
    QPointer<QApplication>      pMainApp;           //!< If we had to create the Qt main application, it will be stored here for later destruction; 0-guarded.

    QVideoFrame                 *pCurVideoFrame;    //!< Holds last displayed video frame  until either a new frame  was displayed or the display was cleared.
#ifdef FB_ENABLE
    VideoFbConfig_t              FbConfig;
#endif
} domCtrlVidplayContext_t;


/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/
static int domCtrlVidplayMsgPumpThread
(
    void *pArg
);

static RESULT domCtrlVidplayCreatePlayer
(
    domCtrlVidplayContext_t *pDomCtrlVidplayContext
);

/******************************************************************************
 * API functions; see header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * domCtrlVidplayInit()
 *****************************************************************************/
RESULT domCtrlVidplayInit
(
    domCtrlVidplayConfig_t *pConfig
)
{
    RESULT result = RET_SUCCESS;

    domCtrlVidplayContext_t *pDomCtrlVidplayContext;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if (pConfig == NULL)
    {
        return RET_NULL_POINTER;
    }

    // allocate control context
    pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)malloc( sizeof(domCtrlVidplayContext_t) );
    if (pDomCtrlVidplayContext == NULL)
    {
        TRACE( DOM_CTRL_QT_ERROR, "%s: allocating control context failed.\n", __FUNCTION__ );
        return RET_OUTOFMEM;
    }
    memset( pDomCtrlVidplayContext, 0, sizeof(domCtrlVidplayContext_t) );

    // pre initialize control context
    pDomCtrlVidplayContext->HaveMainAppThread = false;
    pDomCtrlVidplayContext->Config            = *pConfig;
    pDomCtrlVidplayContext->pCurVideoFrame    = 0;

    // if there is already a QApplication, we just need to create the player;
    // otherwise we have to create and execute the application from an extra
    // thread; due to Qt internals the player has to be created from within
    // that thread as well
    if (qApp != NULL)
    {
        // create video player
        TRACE( DOM_CTRL_QT_INFO, "%s: creating video player\n", __FUNCTION__ );
        RESULT CreateVidplayResult = domCtrlVidplayCreatePlayer( pDomCtrlVidplayContext );
        if (RET_SUCCESS != CreateVidplayResult)
        {
            memset( pDomCtrlVidplayContext, 0, sizeof(domCtrlVidplayContext_t) );
            free( pDomCtrlVidplayContext );
            return RET_FAILURE;
        }
        pDomCtrlVidplayContext->pVidplay->show();
    }
    else
    {
        // create an extra thread for Qt Main Application; use OSLayer threads & sync objects here
        // as Qt threads & sync objects can't be used since QApplication is not yet created...
        int osStatus;

        TRACE( DOM_CTRL_QT_INFO, "%s: need to create Qt Main Application\n", __FUNCTION__ );

        // create event
        TRACE( DOM_CTRL_QT_INFO, "%s: create sync event\n", __FUNCTION__ );
        osStatus = osEventInit( &pDomCtrlVidplayContext->MainAppEvent, 1, 0 );
        if (OSLAYER_OK != osStatus)
        {
            TRACE( DOM_CTRL_QT_ERROR, "%s: osThreadCreate() failed (OSLAYER_STATUS=%d).\n", __FUNCTION__, osStatus );

            memset( pDomCtrlVidplayContext, 0, sizeof(domCtrlVidplayContext_t) );
            free( pDomCtrlVidplayContext );

            return RET_FAILURE;
        }

        // need to create a thread for this to work
        TRACE( DOM_CTRL_QT_INFO, "%s: create helper thread\n", __FUNCTION__ );
        osStatus = osThreadCreate( &pDomCtrlVidplayContext->MainAppThread, domCtrlVidplayMsgPumpThread, pDomCtrlVidplayContext );
        if (OSLAYER_OK != osStatus)
        {
            TRACE( DOM_CTRL_QT_ERROR, "%s: osThreadCreate() failed (OSLAYER_STATUS=%d).\n", __FUNCTION__, osStatus );

            osEventDestroy( &pDomCtrlVidplayContext->MainAppEvent );

            memset( pDomCtrlVidplayContext, 0, sizeof(domCtrlVidplayContext_t) );
            free( pDomCtrlVidplayContext );

            return RET_FAILURE;
        }

        // wait until thread is started up
        TRACE( DOM_CTRL_QT_INFO, "%s: wait for sync event\n", __FUNCTION__ );
        osStatus = osEventWait( &pDomCtrlVidplayContext->MainAppEvent );
        if (OSLAYER_OK != osStatus)
        {
            TRACE( DOM_CTRL_QT_ERROR, "%s: osEventWait() failed (OSLAYER_STATUS=%d).\n", __FUNCTION__, osStatus );

            //TODO: this may lead to a race condition if the thread terminates prematurely
            if (pDomCtrlVidplayContext->pMainApp)
            {
                TRACE( DOM_CTRL_QT_INFO, "%s: terminating Qt Main Application\n", __FUNCTION__ );
                pDomCtrlVidplayContext->pMainApp->quit(); // this will in turn terminate our MainAppThread as well
            }

            osThreadWait( &pDomCtrlVidplayContext->MainAppThread );
            osThreadClose( &pDomCtrlVidplayContext->MainAppThread );

            osEventDestroy( &pDomCtrlVidplayContext->MainAppEvent );

            memset( pDomCtrlVidplayContext, 0, sizeof(domCtrlVidplayContext_t) );
            free( pDomCtrlVidplayContext );

            return RET_FAILURE;
        }

        // Qt Main Application and message pump running
        TRACE( DOM_CTRL_QT_INFO, "%s: Qt Main Application and message pump running\n", __FUNCTION__ );
        pDomCtrlVidplayContext->HaveMainAppThread = true;
    }
#ifdef FB_ENABLE
    VideoFbInit(&(pDomCtrlVidplayContext->FbConfig));
#endif
    // success, so let's return the control context handle
    pConfig->domCtrlVidplayHandle = (domCtrlVidplayHandle_t) pDomCtrlVidplayContext;

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * domCtrlVidplayShutDown()
 *****************************************************************************/
RESULT domCtrlVidplayShutDown
(
    domCtrlVidplayHandle_t  domCtrlVidplayHandle
)
{
    RESULT result = RET_SUCCESS;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if (domCtrlVidplayHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)domCtrlVidplayHandle;
#ifdef FB_ENABLE
    VideoFbRelease(&(pDomCtrlVidplayContext->FbConfig));
#endif
    // cleanup
    if (pDomCtrlVidplayContext->HaveMainAppThread)
    {
        // quit QApplication; quits and subsequently destroys player as well
        TRACE( DOM_CTRL_QT_INFO, "%s terminating Qt Main Application\n", __FUNCTION__ );
        pDomCtrlVidplayContext->pMainApp->quit(); // this will in turn terminate our MainAppThread as well

        TRACE( DOM_CTRL_QT_INFO, "%s waiting for helper thread done\n", __FUNCTION__ );
        osThreadWait( &pDomCtrlVidplayContext->MainAppThread );
        osThreadClose( &pDomCtrlVidplayContext->MainAppThread );

        osEventDestroy( &pDomCtrlVidplayContext->MainAppEvent );
    }
    else
    {
        // just destroy video player
        delete pDomCtrlVidplayContext->pVidplay;
    }

    // destroy last video frame?
    if (pDomCtrlVidplayContext->pCurVideoFrame)
    {
        delete pDomCtrlVidplayContext->pCurVideoFrame;
    }

    // free context
    free( pDomCtrlVidplayContext );

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * domCtrlVidplayDisplay()
 *****************************************************************************/
RESULT domCtrlVidplayDisplay
(
    domCtrlVidplayHandle_t  domCtrlVidplayHandle,
    PicBufMetaData_t        *pPicBufMetaData
)
{
    RESULT result = RET_SUCCESS;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if ( (domCtrlVidplayHandle == NULL) ||  (pPicBufMetaData == NULL) )
    {
        return RET_NULL_POINTER;
    }

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)domCtrlVidplayHandle;

    // check buffer meta data
    if ( (pPicBufMetaData->Type != PIC_BUF_TYPE_RGB32) || (pPicBufMetaData->Layout != PIC_BUF_LAYOUT_COMBINED) )
    {
        return RET_NOTSUPP;
    }

    // convert picture buffer meta data into Qt's preferred meta data objects
    uchar *pData  = pPicBufMetaData->Data.RGB.combined.pData;
    int   width   = pPicBufMetaData->Data.RGB.combined.PicWidthPixel;
    int   height  = pPicBufMetaData->Data.RGB.combined.PicHeightPixel;
    int   linelen = pPicBufMetaData->Data.RGB.combined.PicWidthBytes;
    int   bufsize = linelen * height;
    QSize size( width, height );
#ifdef FB_ENABLE
    VideoFbrSetRgbDisplay(&(domCtrlVidplayHandle->FbConfig),pData,width,height);
#endif

    // create QVideoFrame from image data via an ExtMemVideobuffer (derived from QAbstractVideoBuffer);
    // QVideoFrame takes care to delete the VideoBuffer on destruction
    ExtMemVideoBuffer *pVideoBuffer = new ExtMemVideoBuffer( pData, bufsize, linelen );
    QVideoFrame *pVideoFrame = new QVideoFrame( pVideoBuffer, size, QVideoFrame::Format_RGB32 );
    if (!pVideoFrame->isValid())
    {
        TRACE( DOM_CTRL_QT_ERROR, "%s: pVideoFrame->isValid() failed.\n", __FUNCTION__ );
        return RET_INVALID_PARM;
    }

    // deal with possible format change
    if (pDomCtrlVidplayContext->pCurVideoFrame)
    {
        if ( (height != pDomCtrlVidplayContext->pCurVideoFrame->height())
          || (width  != pDomCtrlVidplayContext->pCurVideoFrame->width() ) )
        {
            // clear display
            if (pDomCtrlVidplayContext->pVidplay)
            {
                //TODO: we need more protection here as the Vidplay object still may pass away between
                //      the previous check and the actual call;
                //      -> QObject's destroyed() signal may help us here together with an access mutex
                //pDomCtrlVidplayContext->pVidplay->clearDisplay();
            }
        }
    }

    // present frame
    if (pDomCtrlVidplayContext->pVidplay)
    {
        //TODO: we need more protection here as the Vidplay object still may pass away between
        //      the previous check and the actual call;
        //      -> QObject's destroyed() signal may help us here together with an access mutex
        pDomCtrlVidplayContext->pVidplay->presentFrame( pVideoFrame );
    }

////    QTime time;
////    int numFrames = 1000;
////    int i;
////    qApp->processEvents();= QString
////    time.start();
////    for (i=0; i<numFrames; i++)
////    {
////        // present frame
////        if (pDomCtrlVidplayContext->pVidplay)
////        {
////            //TODO: we need more protection here as the Vidplay object still may pass away between
////            //      the previous check and the actual call;
////            //      -> QObject's destroyed() signal may help us here together with an access mutex
////            pDomCtrlVidplayContext->pVidplay->presentFrame( pVideoFrame );
////
////            int delta_ms = time.elapsed();
////            QString text = QString("++++++++  %1 frames in %2 msec -> %3 frames/sec  ++++++++").arg(i).arg(delta_ms).arg((i * 1000) / delta_ms);
////            pDomCtrlVidplayContext->pVidplay->setOverlayText( text );
////        }
////        qApp->processEvents();
////    }
////    int delta_ms = time.elapsed();
////    printf( "++++++++  %d frames in %d msec -> %d frames/sec  ++++++++\n", numFrames, delta_ms, (numFrames * 1000) / delta_ms );

    // destroy last video frame
    if (pDomCtrlVidplayContext->pCurVideoFrame)
    {
        delete pDomCtrlVidplayContext->pCurVideoFrame;
    }

    // remember new video frame
    pDomCtrlVidplayContext->pCurVideoFrame = pVideoFrame;

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * domCtrlVidplayClear()
 *****************************************************************************/
RESULT domCtrlVidplayClear
(
    domCtrlVidplayHandle_t  domCtrlVidplayHandle
)
{
    RESULT result = RET_SUCCESS;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if (domCtrlVidplayHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)domCtrlVidplayHandle;

    // clear display
    if (pDomCtrlVidplayContext->pVidplay)
    {
        //TODO: we need more protection here as the Vidplay object still may pass away between
        //      the previous check the actual call
        //      -> QObject's destroyed() signal may help us here together with an access mutex
        //pDomCtrlVidplayContext->pVidplay->clearDisplay();
    }

    // destroy last video frame?
    if (pDomCtrlVidplayContext->pCurVideoFrame)
    {
        delete pDomCtrlVidplayContext->pCurVideoFrame;
        pDomCtrlVidplayContext->pCurVideoFrame = 0;
    }

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * domCtrlVidplayShow()
 *****************************************************************************/
RESULT domCtrlVidplayShow
(
    domCtrlVidplayHandle_t  domCtrlVidplayHandle,
    bool_t                  show
)
{
    RESULT result = RET_SUCCESS;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if (domCtrlVidplayHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)domCtrlVidplayHandle;

    // clear display
    if (pDomCtrlVidplayContext->pVidplay)
    {
        //TODO: we need more protection here as the Vidplay object still may pass away between
        //      the previous check the actual call
        //      -> QObject's destroyed() signal may help us here together with an access mutex
        if ( BOOL_TRUE == show )
        {
            //pDomCtrlVidplayContext->pVidplay->show();
        }
        else
        {
            //pDomCtrlVidplayContext->pVidplay->hide();
        }
    }

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * domCtrlVidplaySetOverlayText()
 *****************************************************************************/
RESULT domCtrlVidplaySetOverlayText
(
    domCtrlVidplayHandle_t  domCtrlVidplayHandle,
    char                    *szOverlayText
)
{
    RESULT result = RET_SUCCESS;

    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    if (domCtrlVidplayHandle == NULL)
    {
        return RET_NULL_POINTER;
    }

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)domCtrlVidplayHandle;

    // pass string to vidplay object
    QString overlayText( QString::fromLatin1(szOverlayText) );
    if (pDomCtrlVidplayContext->pVidplay)
    {
        //TODO: we need more protection here as the Vidplay object still may pass away between
        //      the previous check the actual call
        //      -> QObject's destroyed() signal may help us here together with an access mutex
        //pDomCtrlVidplayContext->pVidplay->setOverlayText( overlayText );
    }

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return result;
}


/******************************************************************************
 * local functions
 *****************************************************************************/


/******************************************************************************
 * domCtrlVidplayMsgPumpThread()
 *****************************************************************************/
static int domCtrlVidplayMsgPumpThread
(
    void *pArg
)
{
    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    // get context
    domCtrlVidplayContext_t *pDomCtrlVidplayContext = (domCtrlVidplayContext_t*)pArg;

    // create Qt Main Application
    TRACE( DOM_CTRL_QT_INFO, "%s: creating Qt Main Application\n", __FUNCTION__ );
    //QInternal::callFunction( QInternal::SetCurrentThreadToMainThread, 0 ); // stop Qt complaining about Main App not beinbg created in main thread
    pDomCtrlVidplayContext->argc     = 1;
    pDomCtrlVidplayContext->argv[0]  = (char*)"dom_ctrl_vidplay_api";
    pDomCtrlVidplayContext->pMainApp = new QApplication( pDomCtrlVidplayContext->argc, pDomCtrlVidplayContext->argv );

    // create video player
    TRACE( DOM_CTRL_QT_INFO, "%s: creating video player\n", __FUNCTION__ );
    RESULT CreateVidplayResult = domCtrlVidplayCreatePlayer( pDomCtrlVidplayContext );
    if (RET_SUCCESS != CreateVidplayResult)
    {
        // intentionally destroy Qt Main Application first
        TRACE( DOM_CTRL_QT_INFO, "%s: destroying Qt Main Application\n", __FUNCTION__ );
        delete pDomCtrlVidplayContext->pMainApp;

        // then sync with calling thread
        TRACE( DOM_CTRL_QT_INFO, "%s: signalling sync event\n", __FUNCTION__ );
        osEventSignal( &pDomCtrlVidplayContext->MainAppEvent );

        return 1;
    }

    //show windows
    pDomCtrlVidplayContext->pVidplay->show();

    // sync with calling thread
    TRACE( DOM_CTRL_QT_INFO, "%s: signalling sync event\n", __FUNCTION__ );
    osEventSignal( &pDomCtrlVidplayContext->MainAppEvent );

    // execute Qt Main Application's message pump; will return once main app terminates
    TRACE( DOM_CTRL_QT_INFO, "%s: executing Qt Main Application message pump\n", __FUNCTION__ );
    pDomCtrlVidplayContext->pMainApp->exec();

    // destroy video player
    TRACE( DOM_CTRL_QT_INFO, "%s: destroying video player\n", __FUNCTION__ );
    delete pDomCtrlVidplayContext->pVidplay;

    // destroy Qt Main Application
    TRACE( DOM_CTRL_QT_INFO, "%s: destroying Qt Main Application\n", __FUNCTION__ );
    delete pDomCtrlVidplayContext->pMainApp;

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return 0;
}


/******************************************************************************
 * domCtrlVidplayCreatePlayer()
 *****************************************************************************/
static RESULT domCtrlVidplayCreatePlayer
(
    domCtrlVidplayContext_t *pDomCtrlVidplayContext
)
{
    TRACE( DOM_CTRL_QT_INFO, "%s (enter)\n", __FUNCTION__ );

    // create video player
    try
    {
        pDomCtrlVidplayContext->pVidplay = new DomCtrlVidplay( (QWidget*)(pDomCtrlVidplayContext->Config.hParent),
                                                               pDomCtrlVidplayContext->Config.posX, pDomCtrlVidplayContext->Config.posY,
                                                               pDomCtrlVidplayContext->Config.width, pDomCtrlVidplayContext->Config.height );
    }
    catch(...)
    {
        TRACE( DOM_CTRL_QT_ERROR, "%s: creating video player failed.\n", __FUNCTION__ );
        return RET_FAILURE;
    }

    TRACE( DOM_CTRL_QT_INFO, "%s (exit)\n", __FUNCTION__ );

    return RET_SUCCESS;
}

/* @} dom_ctrl_vidplay_api */
