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
#include <oslayer/oslayer.h>
#include <cam_device/cam_device_buf_defs.h>

#include "dom_ctrl_vidplay_api.h"
#include <QtWidgets/QApplication> // for qApp

#include <QThread>
#include <QObject>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtMultimedia>

#include "main.h"


RESULT fillRGBW( PicBufMetaData_t *pPicBufMetaData )
{
    // check buffer meta data
    if ( (pPicBufMetaData->Type != PIC_BUF_TYPE_RGB32) || (pPicBufMetaData->Layout != PIC_BUF_LAYOUT_COMBINED) )
    {
        return RET_NOTSUPP;
    }

    uchar *pBuf   = pPicBufMetaData->Data.RGB.combined.pData;
    int   width   = pPicBufMetaData->Data.RGB.combined.PicWidthPixel;
    int   height  = pPicBufMetaData->Data.RGB.combined.PicHeightPixel;
    int   linelen = pPicBufMetaData->Data.RGB.combined.PicWidthBytes;

    uchar *pData = pBuf;
    int x, y;
    for (y=0; y<height; y++)
    {
        uchar Lum = (255*y)/width;
        uchar *pPix = pData;
        for (x=0; x<width; x++)
        {
            uchar R=0,G=0,B=0;

            if      (x < (1*width)/4)
            {
                R = Lum;
            }
            else if (x < (2*width)/4)
            {
                G = Lum;
            }
            else if (x < (3*width)/4)
            {
                B = Lum;
            }
            else
            {
                R = G = B = Lum;
            }

            *pPix++ = R;
            *pPix++ = G;
            *pPix++ = B;
            *pPix++;
        }
        pData += linelen;
    }

    return RET_SUCCESS;
}

/******************************************************************************
 * testVidplayer
 *****************************************************************************/
int testVidplayer( domCtrlVidplayHandle_t hVidPlayer = 0, QWidget *parent = 0 )
{
    RESULT result;
    //domCtrlVidplayHandle_t hVidPlayer;

    printf("qApp = %p\n", (void*)qApp);

    //#### step 1
    if (!hVidPlayer)
    {
        // prepare dom video player configuration
        domCtrlVidplayConfig_t domVidplayConfig;
        domVidplayConfig.hParent = (void*)parent;
        domVidplayConfig.posX    = 0;
        domVidplayConfig.posY    = 0;
        domVidplayConfig.width   = 640;
        domVidplayConfig.height  = 480;
        domVidplayConfig.domCtrlVidplayHandle = NULL;

        // create dom video player
        printf("%s: Creating dom video player\n", __FUNCTION__);
        result = domCtrlVidplayInit( &domVidplayConfig );
        if (result != RET_SUCCESS)
        {
            printf("%s: domCtrlVidplayInit() failed (RESULT=%d)", __FUNCTION__, result);
            return 1;
        }
        // success
        hVidPlayer = domVidplayConfig.domCtrlVidplayHandle;
    }

    // sleep some time
    printf("%s: Sleeping 3 secs\n", __FUNCTION__);
    osSleep(3000);

    //#### step 2
    // prepare sample image to display
    PicBufMetaData_t PicBufMetaData;
    memset( &PicBufMetaData, 0, sizeof(PicBufMetaData_t) );
    PicBufMetaData.Type   = PIC_BUF_TYPE_RGB32;
    PicBufMetaData.Layout = PIC_BUF_LAYOUT_COMBINED;
    PicBufMetaData.Data.RGB.combined.PicWidthPixel  = 640;
    PicBufMetaData.Data.RGB.combined.PicHeightPixel = 480;
    PicBufMetaData.Data.RGB.combined.PicWidthBytes  = 4*PicBufMetaData.Data.RGB.combined.PicWidthPixel;

    PicBufMetaData.Data.RGB.combined.pData        = (uint8_t*)malloc( PicBufMetaData.Data.RGB.combined.PicHeightPixel * PicBufMetaData.Data.RGB.combined.PicWidthBytes );
    if (PicBufMetaData.Data.RGB.combined.pData == NULL)
    {
        printf("%s: malloc(%d) failed (RESULT=%d)", __FUNCTION__, PicBufMetaData.Data.RGB.combined.PicHeightPixel * PicBufMetaData.Data.RGB.combined.PicWidthBytes, result);
    }

    result = fillRGBW( &PicBufMetaData );
    if (result != RET_SUCCESS)
    {
        printf("%s: fillRGBW() failed (RESULT=%d)", __FUNCTION__, result);
    }

    // display image
    printf("%s: Displaying image(s)\n", __FUNCTION__);
    domCtrlVidplaySetOverlayText(hVidPlayer, (char*)"Hello world! See g, j, p, q & y!");
    result = domCtrlVidplayDisplay(hVidPlayer, &PicBufMetaData);
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayDisplay() failed (RESULT=%d)", __FUNCTION__, result);
    }

    // sleep some time
    printf("%s: Sleeping 5 secs\n", __FUNCTION__);
    osSleep(5000);

    //#### step 3
    // clear display area
    printf("%s: Clearing display area\n", __FUNCTION__);
    result = domCtrlVidplayClear(hVidPlayer);
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayClear() failed (RESULT=%d)", __FUNCTION__, result);
    }

    // sleep some time
    printf("%s: Sleeping 3 secs\n", __FUNCTION__);
    osSleep(3000);

    //#### step 4
    // destroy dom video player
    printf("%s: Destroying dom video player\n", __FUNCTION__);
    result = domCtrlVidplayShutDown(hVidPlayer);
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayShutDown() failed (RESULT=%d)", __FUNCTION__, result);
    }
    hVidPlayer = NULL;

    // free image buffer
    free( PicBufMetaData.Data.RGB.combined.pData );

    printf("%s: Done!\n", __FUNCTION__);

    return 0;
}


/******************************************************************************
 * class MyVidplayTest
 *****************************************************************************/
static domCtrlVidplayHandle_t hVidPlayer = 0;

MyVidplayTest::MyVidplayTest( QObject *parent )
    : QThread(parent)
{
}

void MyVidplayTest::create()
{
    RESULT result;

    //#### step 1
    if (!hVidPlayer)
    {
        // prepare dom video player configuration
        domCtrlVidplayConfig_t domVidplayConfig;
        domVidplayConfig.hParent = NULL;
        domVidplayConfig.posX    = 200;
        domVidplayConfig.posY    = 200;
        domVidplayConfig.width   = 640;
        domVidplayConfig.height  = 480;
        domVidplayConfig.domCtrlVidplayHandle = NULL;

        // create dom video player
        printf("%s: Creating dom video player\n", __FUNCTION__);
        result = domCtrlVidplayInit( &domVidplayConfig );
        if (result != RET_SUCCESS)
        {
            printf("%s: domCtrlVidplayInit() failed (RESULT=%d)", __FUNCTION__, result);
            return;
        }
        else
        {
            // success
            hVidPlayer = domVidplayConfig.domCtrlVidplayHandle;
        }
    }
}

void MyVidplayTest::display()
{
    run();
}

void MyVidplayTest::run() //-> display()
{
    RESULT result;

    //#### step 2
    // prepare sample image to display
    PicBufMetaData_t PicBufMetaData;
    memset( &PicBufMetaData, 0, sizeof(PicBufMetaData_t) );

    // display image
    printf("%s: Displaying image\n", __FUNCTION__);
    domCtrlVidplaySetOverlayText(hVidPlayer, (char*)"This text was set using domCtrlVidplaySetOverlayText()!");
    result = domCtrlVidplayDisplay( hVidPlayer, &PicBufMetaData );
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayDisplay() failed (RESULT=%d)", __FUNCTION__, result);
    }
}

void MyVidplayTest::clear()
{
    RESULT result;

    //#### step 3
    // clear display area
    printf("%s: Clearing display area\n", __FUNCTION__);
    result = domCtrlVidplayClear( hVidPlayer );
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayClear() failed (RESULT=%d)", __FUNCTION__, result);
    }
}

void MyVidplayTest::destroy()
{
    RESULT result;

    //#### step 4
    // destroy dom video player
    printf("%s: Destroying dom video player\n", __FUNCTION__);
    result = domCtrlVidplayShutDown( hVidPlayer );
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayShutDown() failed (RESULT=%d)", __FUNCTION__, result);
    }
    hVidPlayer = NULL;
}

/******************************************************************************
 * class MyThread
 *****************************************************************************/
class MyThread : public QThread
{
public:
    MyThread(){m_result = 0;}
    void run();
    int m_result;
};

void MyThread::run()
{
    m_result = testVidplayer();
}


/******************************************************************************
 * class MyThread2
 *****************************************************************************/
class MyThread2 : public QThread
{
public:
    MyThread2( QWidget *parent = 0 );
    void run();
    int m_result;
private:
    domCtrlVidplayHandle_t m_hVideoPlayer;
};

MyThread2::MyThread2( QWidget *parent )
{
    RESULT result;

    m_hVideoPlayer = 0;
    m_result = 0;

    // prepare dom video player configuration
    domCtrlVidplayConfig_t domVidplayConfig;
    domVidplayConfig.hParent = parent;
    domVidplayConfig.posX    = 0;
    domVidplayConfig.posY    = 0;
    domVidplayConfig.width   = 0;
    domVidplayConfig.height  = 0;
    domVidplayConfig.domCtrlVidplayHandle = NULL;

    // create dom video player
    printf("%s: Creating dom video player\n", __FUNCTION__);
    result = domCtrlVidplayInit( &domVidplayConfig );
    if (result != RET_SUCCESS)
    {
        printf("%s: domCtrlVidplayInit() failed (RESULT=%d)", __FUNCTION__, result);
        m_result = 1;
        return;
    }
    // success
    m_hVideoPlayer = domVidplayConfig.domCtrlVidplayHandle;
}

void MyThread2::run()
{
    m_result = testVidplayer(m_hVideoPlayer);
    emit qApp->exit( m_result );
}


/******************************************************************************
 * usage()
 *****************************************************************************/
void usage(void)
{
    printf("Only one param supported:\n");
    printf("    1 : to run directly from main\n");
    printf("    2 : to run indirectly from extra thread\n");
    printf("    3 : to run indirectly from extra thread with existing qApp\n");
    printf("    4 : to run indirectly from extra thread with existing qApp and a parent widget\n");
    printf("    5 : to run GUI controlled from qApp context\n");
}


/******************************************************************************
 * main()
 *****************************************************************************/
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage();
        exit(1);
    }

    int test = atoi(argv[1]);
    switch(test)
    {
        case 1:
            return testVidplayer();

        case 2:
        {
            MyThread doIt;
            doIt.start();
            doIt.wait();
            return doIt.m_result;
        }

        case 3:
        {
            //int argc = 1;
            //char *argv[1] = {(char*)"dom_ctrl_vidplay_testappl"};
            QApplication app(argc, argv);

            printf("Create GUI\n");
            QWidget *mainWidget = new QWidget();
            QAbstractButton *startButton = new QPushButton("Start test");
            QWidget *dummyWidget = new QWidget();
            dummyWidget->setFixedSize( 100, 100 );
            QBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(startButton);
            layout->addWidget(dummyWidget);
            mainWidget->setLayout(layout);
            mainWidget->show();

            printf("Create worker thread\n");
            MyThread2 doIt;
            QObject::connect( startButton, SIGNAL(clicked()), &doIt, SLOT(start()) );

            printf("Executing application\n");
            app.exec();

            printf("Waiting for thread\n");
            doIt.wait();
            printf("Done waiting for thread\n");

            return doIt.m_result;
        }

        case 4:
        {
            //int argc = 1;
            //char *argv[1] = {(char*)"dom_ctrl_vidplay_testappl"};
            QApplication app(argc, argv);

            printf("Create GUI\n");
            QWidget *mainWidget = new QWidget();
            QAbstractButton *startButton = new QPushButton("Start test");
            QWidget *dummyWidget = new QWidget();
            dummyWidget->setFixedSize( 100, 100 );
            QBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(startButton);
            layout->addWidget(dummyWidget);
            mainWidget->setLayout(layout);
            mainWidget->show();

            printf("Create worker thread\n");
            MyThread2 doIt( dummyWidget );
            QObject::connect( startButton, SIGNAL(clicked()), &doIt, SLOT(start()) );

            printf("Executing application\n");
            app.exec();

            printf("Waiting for thread\n");
            doIt.wait();
            printf("Done waiting for thread\n");

            return doIt.m_result;
        }

        case 5:
        {
            //int argc = 1;
            //char *argv[1] = {(char*)"dom_ctrl_vidplay_testappl"};
            QApplication app(argc, argv);

            printf("Create GUI\n");
            QWidget *mainWidget = new QWidget();
            QAbstractButton *createButton = new QPushButton("Create");
            QAbstractButton *displayButton = new QPushButton("Display");
            QAbstractButton *clearButton = new QPushButton("Clear");
            QAbstractButton *destroyButton = new QPushButton("Destroy");
            QBoxLayout *layout = new QHBoxLayout;
            layout->addWidget(createButton);
            layout->addWidget(displayButton);
            layout->addWidget(clearButton);
            layout->addWidget(destroyButton);
            mainWidget->setLayout(layout);
            mainWidget->show();

            printf("Create worker thread\n");
            MyVidplayTest test;
            test.start();
            QObject::connect( createButton,  SIGNAL(clicked()), &test, SLOT(create())  ); //, Qt::QueuedConnection );
            QObject::connect( displayButton, SIGNAL(clicked()), &test, SLOT(start()) ); //, Qt::QueuedConnection );
            QObject::connect( clearButton,   SIGNAL(clicked()), &test, SLOT(clear())   ); //, Qt::QueuedConnection );
            QObject::connect( destroyButton, SIGNAL(clicked()), &test, SLOT(destroy()) ); //, Qt::QueuedConnection );

            printf("Executing application\n");
            app.exec();

            printf("Terminating thread\n");
            test.quit();
            printf("Waiting for thread\n");
            test.wait();
            printf("Done waiting for thread\n");

            return 0;
        }

        default:
            usage();
            exit(2);
    }

    return 0;
}
