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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>

#include <QMutex>
#include <thread>

#include <appshell_dom_ctrl/dom_ctrl_api.h>

#include "VirtualCamera.h"

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QComboBox;

class EnvironmentTab;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
    ~MainWindow();
    void AfpsResChangeCb();

    osThread Thread;

protected:
    virtual void closeEvent( QCloseEvent *event );

signals:
    void opened();
    void closed();
    void connected();
    void disconnected();
    void started();
    void stopped();
    void buffer(isp_metadata* meta);
    void resChanged();

private slots:
    //void enable3D( bool enable = true );
    void enableTestpattern( bool enable = true );
    void togglePictureOrientation( QAction *action );
    void changeResolution();
    void setFlickerPeriod();
    void enableAfps( bool enable = false );

    void loadSensor( QString fileName = QString::null );
    void loadCalibData();
    void connectCamera();
    void disconnectCamera();

    void start();
    void pause();
    void stop();

    void live();
    void fileFinished( int result );

public:
    void capture( QString &fileName, int type, uint32_t resolution, uint32_t locks );
    VirtualCamera *camDevice;
    struct isp_metadata* mMetadata;

private:
    void createActions();
    void createToolBars();

private:
   
    QString            sensorDriverFile;

    QMutex          snapshotMutex;

    // gui elements
    QToolBar *cmdToolBar;
    QMenu     *inputMenu;

    // command actions
    QActionGroup    *inputActGroup;
    QAction    *liveInputAct;
    QAction *wdr5MP;
    QAction *wdr2MP;
    QAction *dpcc5MP;
    QAction *dpcc2MP;
    QAction *dpf5MP;
    QAction *dpf2MP;
    QAction *canon21MP;
    QAction *face2MP;
    QAction *fileInputAct;
    QAction *startAct;
    QAction *pauseAct;
    QAction *stopAct;
    QAction *captureAct;

    // image actions
    QActionGroup *picOrientActGrp;
    QAction *origAct;
    QAction *hflipAct;
    QAction *vflipAct;
    QAction *rotateLeftAct;
    QAction *rotateRightAct;

    QTabWidget *tabWidget;
    EnvironmentTab *envTab;

public:
    EnvironmentTab *getEnvTab(){ return envTab; };

private:
    domCtrlHandle_t dom;

    void onFrameAvailable(BufIdentity* buf, int port);
};

#endif  /* __MAINWINDOW_H__ */
