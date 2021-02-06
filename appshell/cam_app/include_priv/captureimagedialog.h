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

#ifndef __CAPTUREIMAGEDIALOG_H__
#define __CAPTUREIMAGEDIALOG_H__

#include <QtWidgets/QDialog>
#include "VirtualCamera.h"

class QLineEdit;
class QComboBox;
class QGroupBox;
class QButtonGroup;

class CaptureImageDialog
  : public QDialog
{
    Q_OBJECT

public:
    CaptureImageDialog(  VirtualCamera* camDevice, QWidget *parent, Qt::WindowFlags flags = 0 );

protected:
    virtual void showEvent( QShowEvent *event );
    virtual void closeEvent( QCloseEvent *event );

public slots:
    void opened();
    void closed();
    void connected();
    void disconnected();
    void started();
    void stopped();

private slots:
    void browse();
    void capture();

private:
    VirtualCamera    *m_camDevice;
    QPoint         m_lastPos;

    QGroupBox     *m_settingsGroupBox;
    QLineEdit     *m_folderLineEdit;
    QComboBox     *m_imageTypeComboBox;
    QComboBox     *m_imageResComboBox;
    QButtonGroup  *m_imageLockButtonGroup;
};

#endif /*__CAPTUREIMAGEDIALOG_H__*/
