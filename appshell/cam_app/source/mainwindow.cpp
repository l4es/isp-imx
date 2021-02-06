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

#include "mainwindow.h"

#include <string.h>
#include "captureimagedialog.h"
#include "imageviewer.h"
#include "envtab.h"
#include "level1tab.h"
#include "level2tab.h"
#include "level3tab.h"
//#include "level4tab.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <assert.h>



//TODO: move to cam_device api

/*****************************************************************************/
/**
 * @brief   Enumeration type to specify the picture orientation on the self
 *          path
 *
 *****************************************************************************/
//! self picture operating modes
typedef enum  CamerIcMiOrientation_e
{
    CAMERIC_MI_ORIENTATION_INVALID          = 0,    /**< lower border (only for an internal evaluation) */
    CAMERIC_MI_ORIENTATION_ORIGINAL         = 1,    /**< no rotation, no horizontal or vertical flipping */
    CAMERIC_MI_ORIENTATION_VERTICAL_FLIP    = 2,    /**< vertical   flipping (no additional rotation) */
    CAMERIC_MI_ORIENTATION_HORIZONTAL_FLIP  = 3,    /**< horizontal flipping (no additional rotation) */
    CAMERIC_MI_ORIENTATION_ROTATE90         = 4,    /**< rotation  90 degrees ccw (no additional flipping) */
    CAMERIC_MI_ORIENTATION_ROTATE180        = 5,    /**< rotation 180 degrees ccw (equal to horizontal plus vertical flipping) */
    CAMERIC_MI_ORIENTATION_ROTATE270        = 6,    /**< rotation 270 degrees ccw (no additional flipping) */

#if 0
    eMrvMifSp_Rot_090_V_Flip   = 6, //!< rotation  90 degrees ccw plus vertical flipping
    eMrvMifSp_Rot_270_V_Flip   = 7  //!< rotation 270 degrees ccw plus vertical flipping
#endif

    CAMERIC_MI_ORIENTATION_MAX                      /**< upper border (only for an internal evaluation) */
} CamerIcMiOrientation_t;



#define ERROR_DLG QErrorMessage::qtHandler()->showMessage

void MainWindow_AfpsResChangeCb(const void *ctx ) {
    MainWindow *mainwindow = (MainWindow*)ctx;

    if (mainwindow) {
        mainwindow->AfpsResChangeCb();
    }
}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags ),
    camDevice      ( NULL )
{
    camDevice = VirtualCamera::createObject();

    printf("%s open sensor from camDevice\n", __func__);
    camDevice->open(1);

    // initialize user interface

    // capture image dialog
    CaptureImageDialog* captureImageDialog = new CaptureImageDialog( camDevice, this );

    // tabs
    tabWidget = new QTabWidget( this );

    envTab = new EnvironmentTab( camDevice, this );
    tabWidget->addTab( envTab, tr("Environment") );
    tabWidget->addTab( new Level1Tab( camDevice, this ), tr("Level1 (DRC)") );
    tabWidget->addTab( new Level2Tab( camDevice, this ), tr("Level2 (MF)" ) );
    tabWidget->addTab( new Level3Tab( camDevice, this ), tr("Level3 (AF)") );

    setCentralWidget( tabWidget );

    createActions();
    createToolBars();

    // connect signals
    connect( captureAct, SIGNAL( triggered() ), captureImageDialog, SLOT( show() ) );
    connect( envTab, SIGNAL(sensorSelectButtonClicked()), this, SLOT(loadSensor()) );
    connect( envTab, SIGNAL(flickerPeriodChanged()), this, SLOT(setFlickerPeriod()) );

    connect( envTab, SIGNAL(enableTestpatternToggled(bool)), this, SLOT(enableTestpattern(bool)) );
    connect( envTab, SIGNAL(resolutionChanged()), this, SLOT(changeResolution()) );
    connect( envTab, SIGNAL(enableAfpsToggled(bool)), this, SLOT(enableAfps(bool)) );

    connect( this, SIGNAL( opened()        ), envTab, SLOT( opened()        ) );
    connect( this, SIGNAL( closed()        ), envTab, SLOT( closed()        ) );
    connect( this, SIGNAL( resChanged()    ), envTab, SLOT( resChanged()    ) );

    connect( this, SIGNAL( opened() ), captureImageDialog, SLOT( opened() ) );
    connect( this, SIGNAL( closed() ), captureImageDialog, SLOT( closed() ) );
    connect( this, SIGNAL( connected() ), captureImageDialog, SLOT( connected() ) );
    connect( this, SIGNAL( disconnected() ), captureImageDialog, SLOT( disconnected() ) );
    connect( this, SIGNAL( started() ), captureImageDialog, SLOT( started() ) );
    connect( this, SIGNAL( stopped() ), captureImageDialog, SLOT( stopped() ) );

    // title & size
    setWindowTitle( tr("CamerIc Demonstration Application") );
    /*setMinimumSize( 1280, 720 ); */
    /*resize( 1280, 720 );         */
    setMinimumSize( 700, 850 );
    resize( 700, 900 );

    statusBar()->showMessage( tr(" "), 2000 );

    domCtrlConfig_t ctrlConfig;
    memset( &ctrlConfig, 0, sizeof( domCtrlConfig_t ) );
    ctrlConfig.MaxPendingCommands   = 10;
    ctrlConfig.MaxBuffers           = 1;
    ctrlConfig.domCbCompletion      = NULL;
    ctrlConfig.pUserContext         = (void *)this;
    ctrlConfig.hParent              = NULL;
    ctrlConfig.width                = 0;
    ctrlConfig.height               = 0;
    ctrlConfig.ImgPresent           = DOMCTRL_IMAGE_PRESENTATION_3D_VERTICAL;
    ctrlConfig.domCtrlHandle        = NULL;

    domCtrlInit( &ctrlConfig );
    dom = ctrlConfig.domCtrlHandle;
    domCtrlStart(dom);

    mMetadata = new isp_metadata;
    setAttribute(Qt::WA_DeleteOnClose);
}

MainWindow::~MainWindow()
{
    stop();
    if (dom) {
        domCtrlStop(dom);
        domCtrlShutDown(dom);
    }

    delete mMetadata;
    if ( NULL != camDevice )
    {
        camDevice->close(0);
        delete camDevice;
    }
}

void MainWindow::AfpsResChangeCb()
{
    emit resChanged();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    disconnectCamera();
    QMainWindow::closeEvent( event );
}

void MainWindow::enableTestpattern( bool enable )
{
    disconnectCamera();
     printf("%s ISPCORE_MODULE_SENSOR_TESTPATTERN_EN_SET cmd\n", __func__);
    int ret = camDevice->post<bool>(ISPCORE_MODULE_SENSOR_TESTPATTERN_EN_SET, SENSOR_ENABLE_PARAMS, enable);
}

void MainWindow::togglePictureOrientation( QAction *action )
{
}

void MainWindow::changeResolution() {

    IsiSensorConfig_t sensorConfig;
    memset( &sensorConfig, 0, sizeof( IsiSensorConfig_t ) );
    {
        printf("%s ISPCORE_MODULE_SENSOR_CFG_GET cmd\n", __func__);
#ifndef SUPPORT_PASS_JSON
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CONFIG_PARAMS_BASE64] = "";
        int ret = camDevice->ioctl(ISPCORE_MODULE_SENSOR_CFG_GET, jRequest, jResponse);
        if (ret == 0)
            readValueFromBase64(jResponse, SENSOR_CONFIG_PARAMS_BASE64, &sensorConfig);
#endif
    }
    printf("%s ISPCORE_MODULE_DEVICE_RESOLUTION_SET_SENSOR_ISP cmd\n", __func__);
    uint32_t newResolution = envTab->selectedResolution();
    if ( newResolution != sensorConfig.Resolution ) {
        //temp solution for ISPCORE_MODULE_DEVICE_RESOLUTION_SET_SENSOR_ISP,
        //future change to resolutionSetsIsp + ISPCORE_MODULE_SENSOR_RESOLUTION_SET
        int ret = camDevice->post<int>(ISPCORE_MODULE_DEVICE_RESOLUTION_SET_SENSOR_ISP, DEVICE_RESOLUTION_PARAMS, newResolution);
        emit resChanged();
    }
}

void MainWindow::setFlickerPeriod() {

}

void MainWindow::enableAfps( bool enable ) {

}

void MainWindow::loadSensor( QString fileName ) {
    if ( NULL == fileName) {
        // determine sensor driver directory
        QDir driverDir( QDir::currentPath() ); // default, may be changed below
        if ( !QCoreApplication::applicationDirPath().isEmpty() ) {
            QDir appDir( QCoreApplication::applicationDirPath() );
            if ( appDir.exists() ) {
                QDir driverSubDir( appDir.path() + tr("/sensor-driver") );
                if ( driverSubDir.exists() ) {
                    driverDir.setPath( driverSubDir.absolutePath() );
                } else {
                    driverDir.setPath( appDir.absolutePath() );
                }
            }
        }

        // let user select driver to load; switch to sensor driver dir determined above
        fileName = QFileDialog::getOpenFileName( this,
                tr( "Select Sensor Driver" ),
                driverDir.path(),
                tr( "Driver Files (*.drv)" ) );
    }

    if ( NULL != fileName ) {
#if 0
        disconnectCamera();
#endif
        printf("%s ISPCORE_MODULE_SENSOR_OPEN cmd\n", __func__);
        Json::Value jRequest, jResponse;
        jRequest[SENSOR_SENSOR_DRIVER_PARAMS] = fileName.toLatin1().constData();
        jRequest[SENSOR_SENSOR_DEVICEID] = 0x10000000;
        jRequest[SENSOR_SENSOR_I2C_NUMBER] = 1;
        int ret = camDevice->ioctl(ISPCORE_MODULE_SENSOR_OPEN, jRequest, jResponse);
        emit opened();

        // connect
        connectCamera();

        // update gui
        liveInputAct->setChecked(true);
        sensorDriverFile = fileName;
    }
}

void MainWindow::loadCalibData()
{
    QString fileName = QFileDialog::getOpenFileName( this,
            tr( "Select Calibration Data" ),
            "/home.local/marvin/test_appl/sensor-driver",//QDir::currentPath(),
            tr( "Calibration Data Files (*.xml)" ) );

    if ( NULL != fileName ) {
        disconnectCamera();
        printf("%s ISPCORE_MODULE_SENSOR_CALIB_INSTALL cmd\n", __func__);
        int ret = camDevice->post<const char*>(ISPCORE_MODULE_SENSOR_CALIB_INSTALL, SENSOR_CALIBRATION_DATA_PARAMS, fileName.toLatin1().constData());
        if ( ret != 0 ) {
            QMessageBox::critical( this, tr( "Calibration Data Error" ),
                    tr( "Cannot load calibration data!\n" ) );
        }
    }
}

void MainWindow::onFrameAvailable(BufIdentity* buf, int port)
{
    //camDevice->updateMetadata(mMetadata, port);
    domCtrlShowBuffer(dom, buf);
    //Todo: Broadcast medata to all Tabs.
    emit buffer(mMetadata);
}

void MainWindow::connectCamera() {
    printf("%s registerBufferCallback from camDevice\n", __func__);
    camDevice->registerBufferCallback(std::bind(&MainWindow::onFrameAvailable, this,
                std::placeholders::_1, std::placeholders::_2));
    printf("%s setFormat from camDevice\n", __func__);
    camDevice->setFormat(0, 1920, 1080, CAMERA_PIX_FMT_YUV422SP);
    printf("%s connectCamera from camDevice\n", __func__);
    int ret = camDevice->connectCamera();

	if ( 0 == ret) {

        emit connected();

        // update gui
        origAct->setChecked( true );
        picOrientActGrp->setEnabled( false );
    } else {
        QMessageBox::critical( this, tr( "Sensor Error" ),
                tr( "Could not connect sensor!\n" ) );

        emit disconnected();
        // update gui
        origAct->setChecked( true );
        picOrientActGrp->setEnabled( false );
    }
}

void MainWindow::disconnectCamera() {
    printf("%s disconnectCamera from camDevice\n", __func__);
    camDevice->disconnectCamera();
    printf("%s removeBufferCallback from camDevice\n", __func__);
    camDevice->removeBufferCallback(); 
    emit disconnected();
    // update gui
    origAct->setChecked( true );
    picOrientActGrp->setEnabled( false );
}

void MainWindow::start() {
    printf("%s start from camDevice\n", __func__);
    if ( 0 == camDevice->start() ) {
        emit started();
        // update gui
        picOrientActGrp->setEnabled( true );
    }
}

void MainWindow::pause() { 
    printf("%s stop from camDevice\n", __func__);
    if ( 0 == camDevice->stop() ) {
        emit stopped();

        // update gui
        origAct->setChecked( true );
        picOrientActGrp->setEnabled( false );
    }
}

void MainWindow::stop() {
    printf("%s stop from camDevice\n", __func__);
    if ( 0 == camDevice->stop() ) {
        emit stopped();

        // update gui
        origAct->setChecked( true );
        picOrientActGrp->setEnabled( false );
    }
}

void MainWindow::capture( QString &fileName, int type, uint32_t resolution, uint32_t locks ) {
    if ( true != snapshotMutex.tryLock() ) {
        QMessageBox::information( this, tr( "Capture Error" ),
                            tr( "Already capturing snapshot!\n\nPlease wait for current snapshot to finish.\n" ) );
        return;
    }

    int ret;
    {
        Json::Value jRequest, jResponse;
     
        jRequest[DEVICE_FILE_PARAMS] = fileName.toLatin1().constData();
        jRequest[DEVICE_SNAPSHOT_TYPE_PARAMS] = (CAMDEV_SnapshotType)type;
        jRequest[DEVICE_RESOLUTION_PARAMS] = resolution;
        jRequest[DEVICE_LOCK_TYPE_PARAMS] = (CamEngineLockType_t)locks;

        printf("%s ISPCORE_MODULE_DEVICE_CAPTURESENSOR cmd\n", __func__);
        ret = camDevice->ioctl(ISPCORE_MODULE_DEVICE_CAPTURESENSOR, jRequest, jResponse);
        //camDevice->captureSensor( fileName.toLatin1().constData(), (CAMDEV_SnapshotType)type, resolution, (CamEngineLockType_t)locks )
    }

    if ( 0 != ret) {
        snapshotMutex.unlock();
        QMessageBox::critical( this, tr( "Capture Error" ),
                            tr( "Could not capture image!\n" ) );
        return;
    }
    snapshotMutex.unlock();

    ImageViewer *imageView = new ImageViewer( this );
    imageView->show();
    imageView->open( QFileInfo( fileName ).dir() );
}

void MainWindow::live() {
    loadSensor( sensorDriverFile );
    inputMenu->setIcon( QIcon(":/images/sensor.png") );
}


void MainWindow::fileFinished( int result ) {
    if ( QDialog::Accepted == result ) {
        inputMenu->setIcon( style()->standardIcon( QStyle::SP_FileIcon ) );
    } else {
        fileInputAct->setChecked( false );
    }
}

void MainWindow::createActions() {
/****************************************
** input
****************************************/
    inputActGroup = new  QActionGroup( this );

    liveInputAct = new  QAction( QIcon(":/images/sensor.png"), tr("&Sensor"), this );
    liveInputAct->setStatusTip( tr("Streaming from camera sensor") );
    connect( liveInputAct, SIGNAL(triggered()), this, SLOT(live()) );
    liveInputAct->setCheckable(true);

/****************************************
** WDR
****************************************/
    wdr5MP = new QAction(style()->standardIcon(QStyle::SP_FileIcon),tr("WDR5MP"), this);
    wdr5MP->setStatusTip(tr("WDR5MP"));
  
    wdr5MP->setCheckable(true);

    wdr2MP = new QAction(style()->standardIcon(QStyle::SP_FileIcon),tr("WDR2MP"), this);
    wdr2MP->setStatusTip(tr("WDR2MP"));

    wdr2MP->setCheckable(true);

/****************************************
** DPCC
****************************************/
    dpcc5MP = new QAction(style()->standardIcon(QStyle::SP_FileIcon),tr("DPCC5MP"), this);
    dpcc5MP->setStatusTip(tr("DPCC5MP"));

    dpcc5MP->setCheckable(true);

    dpcc2MP = new QAction(style()->standardIcon(QStyle::SP_FileIcon),tr("DPCC2MP"), this);
    dpcc2MP->setStatusTip(tr("DPCC2MP"));

    dpcc2MP->setCheckable(true);

/****************************************
** DPF
****************************************/
    dpf5MP = new QAction(style()->standardIcon(QStyle::SP_FileIcon),tr("DPF5MP"), this);
    dpf5MP->setStatusTip(tr("DPF5MP"));

    dpf5MP->setCheckable(true);

    dpf2MP = new QAction( style()->standardIcon(QStyle::SP_FileIcon),tr("DPF2MP"), this);
    dpf2MP->setStatusTip(tr("DPF2MP"));
    
    dpf2MP->setCheckable(true);

    canon21MP = new QAction( style()->standardIcon(QStyle::SP_FileIcon), tr("CANON21MP"), this);
    canon21MP->setStatusTip( tr("CANON21MP") );

    canon21MP->setCheckable(true);

/****************************************
** FACE (detection, recognition, ... )
****************************************/
    face2MP = new QAction( style()->standardIcon(QStyle::SP_FileIcon),tr("FACE2MP"), this);
    face2MP->setStatusTip(tr("FACE2MP"));
    //connect(face2MP, SIGNAL(triggered()), this, SLOT(face2MPAction()));
    face2MP->setCheckable(true);

/****************************************
** userdefined raw image
****************************************/
    fileInputAct = new  QAction( style()->standardIcon(QStyle::SP_FileIcon), tr("&RAW-File"), this );
    fileInputAct->setStatusTip( tr("RAW") );
    //connect( fileInputAct, SIGNAL(triggered()), this, SLOT(file()) );
    fileInputAct->setCheckable(true);

    inputActGroup->addAction( liveInputAct );
    inputActGroup->addAction( wdr5MP );
    inputActGroup->addAction( wdr2MP );
    inputActGroup->addAction( dpcc5MP );
    inputActGroup->addAction( dpcc2MP );
    inputActGroup->addAction( dpf5MP );
    inputActGroup->addAction( dpf2MP );
    inputActGroup->addAction( canon21MP );
    inputActGroup->addAction( face2MP );
    inputActGroup->addAction( fileInputAct );
    inputActGroup->setEnabled( true );
    inputActGroup->setExclusive( true );

/****************************************
** camera control
****************************************/
    startAct = new QAction( style()->standardIcon(QStyle::SP_MediaPlay), tr("&Start"), this );
    startAct->setStatusTip( tr("Start streaming") );
    connect( startAct, SIGNAL(triggered()), this, SLOT(start()) );

    pauseAct = new QAction( style()->standardIcon(QStyle::SP_MediaPause), tr("&Pause"), this );
    pauseAct->setStatusTip( tr("Pause streaming") );
    connect( pauseAct, SIGNAL(triggered()), this, SLOT(pause()) );

    stopAct = new QAction( style()->standardIcon(QStyle::SP_MediaStop), tr("&Stop"), this );
    stopAct->setStatusTip( tr("Stop streaming") );
    connect( stopAct, SIGNAL(triggered()), this, SLOT(stop()) );

    captureAct = new  QAction( QIcon(":/images/screenshot.png"), tr("&Capture"), this );
    captureAct->setStatusTip( tr("Capture image") );

/****************************************
** image tool bar
****************************************/
    picOrientActGrp = new  QActionGroup( this );

    origAct = new  QAction( QIcon(":/images/original.png"), tr("&Original"), this );
    origAct->setStatusTip( tr("original") );
    origAct->setCheckable( true );
    origAct->setProperty( "PictureOrientation" , qVariantFromValue<uint32_t>( CAMERIC_MI_ORIENTATION_ORIGINAL ) );
    //connect( origAct, SIGNAL(triggered()), this, SLOT( original() ) );

    hflipAct = new  QAction( QIcon(":/images/hflip.png"), tr("&Horizontal flip"), this );
    hflipAct->setStatusTip( tr("horizontal flip") );
    hflipAct->setCheckable( true );
    hflipAct->setProperty( "PictureOrientation" , qVariantFromValue<uint32_t>( CAMERIC_MI_ORIENTATION_HORIZONTAL_FLIP ) );
    //connect( hflipAct, SIGNAL(triggered()), this, SLOT( hflip() ) );

    vflipAct = new  QAction( QIcon(":/images/vflip.png"), tr("&Vertical flip"), this );
    vflipAct->setStatusTip( tr("vertical flip") );
    vflipAct->setCheckable( true );
    vflipAct->setProperty( "PictureOrientation" , qVariantFromValue<uint32_t>( CAMERIC_MI_ORIENTATION_VERTICAL_FLIP ) );
    //connect( vflipAct, SIGNAL(triggered()), this, SLOT( vflip() ) );

    rotateLeftAct= new  QAction( QIcon(":/images/rotate-left.png"), tr("&Rotate +90°"), this );
    rotateLeftAct->setStatusTip( tr("rotate +90°") );
    rotateLeftAct->setCheckable( true );
    rotateLeftAct->setProperty( "PictureOrientation" , qVariantFromValue<uint32_t>( CAMERIC_MI_ORIENTATION_ROTATE90 ) );
    //connect( rotateLeftAct, SIGNAL(triggered()), this, SLOT( rotateLeft() ) );

    rotateRightAct = new  QAction( QIcon(":/images/rotate-right.png"), tr("&Rotate -90°"), this );
    rotateRightAct->setStatusTip( tr("rotate -90°") );
    rotateRightAct->setCheckable( true );
    rotateRightAct->setProperty( "PictureOrientation" , qVariantFromValue<uint32_t>( CAMERIC_MI_ORIENTATION_ROTATE270 ) );
    //connect( rotateRightAct, SIGNAL(triggered()), this, SLOT( rotateRight() ) );

    picOrientActGrp->addAction( origAct );
    picOrientActGrp->addAction( hflipAct );
    picOrientActGrp->addAction( vflipAct );
    picOrientActGrp->addAction( rotateLeftAct );
    picOrientActGrp->addAction( rotateRightAct );
    picOrientActGrp->setEnabled( false );
    picOrientActGrp->setExclusive( true );
    origAct->setChecked( true );

    connect( picOrientActGrp, SIGNAL(triggered(QAction *)), this, SLOT( togglePictureOrientation(QAction *) ) );
}

void MainWindow::createToolBars() {
    cmdToolBar = addToolBar( tr("Command") );

    inputMenu= new QMenu("Input Selection", this);
    inputMenu->setIcon( QIcon(":/images/sensor.png") );
    inputMenu->addAction( liveInputAct );
    liveInputAct->setChecked(true);
    inputMenu->addSeparator();
    inputMenu->addAction( wdr5MP );
    inputMenu->addAction( wdr2MP );
    inputMenu->addAction( dpcc5MP );
    inputMenu->addAction( dpcc2MP );
    inputMenu->addAction( dpf5MP );
    inputMenu->addAction( dpf2MP );
    //inputMenu->addSeparator();
    inputMenu->addAction( canon21MP );

    inputMenu->addSeparator();
    inputMenu->addAction( fileInputAct );

    cmdToolBar->addAction( inputMenu->menuAction() );
    cmdToolBar->addSeparator( );
    cmdToolBar->addAction( startAct );
    cmdToolBar->addAction( pauseAct );
    cmdToolBar->addAction( stopAct );
    cmdToolBar->addAction( captureAct );

    cmdToolBar->addSeparator( );
    cmdToolBar->addAction( origAct );
    cmdToolBar->addAction( hflipAct );
    cmdToolBar->addAction( vflipAct );
    cmdToolBar->addAction( rotateLeftAct );
    cmdToolBar->addAction( rotateRightAct );
}
