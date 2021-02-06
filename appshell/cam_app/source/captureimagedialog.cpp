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

#include "captureimagedialog.h"

#include "mainwindow.h"

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>

#include "base64.hpp"
#include <string.h>
#include <assert.h>


CaptureImageDialog::CaptureImageDialog(  VirtualCamera* camDevice, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags ),
    m_camDevice( camDevice ),
    m_lastPos()
{
    assert( NULL != camDevice );

    setWindowTitle( tr( "Capture Snapshot" ) );

    QPushButton* browseButton  = new QPushButton(tr("Browse..."), this);
    QPushButton* captureButton = new QPushButton(tr("Capture"), this);
    //QPushButton* cancelButton  = new QPushButton(tr("Cancel"), this);

    QLabel* directoryLabel = new QLabel(tr("Folder"), this);
    QLabel* imageTypeLabel = new QLabel(tr("Type"), this);
    QLabel* imageResLabel = new QLabel(tr("Resolution"), this);
    QLabel* imageLockLabel = new QLabel(tr("Lock"), this);
    //QLabel* numFramesLabel = new QLabel(tr("NumFrames:"), this);

    m_folderLineEdit = new QLineEdit( this );
    // m_folderLineEdit->setText( QDir::currentPath() );
    m_folderLineEdit->setText( QDir::homePath() + "/Pictures" );

    m_imageTypeComboBox = new QComboBox( this );
    m_imageTypeComboBox->addItem( tr( "Portable Pixmap (PPM)" ), (uint32_t)CAMDEV_RGB );
    m_imageTypeComboBox->addItem( tr( "RAW8 (PGM)" ), (uint32_t)CAMDEV_RAW8 );
    m_imageTypeComboBox->addItem( tr( "RAW12 (PGM)" ), (uint32_t)CAMDEV_RAW12 );
    //m_imageTypeComboBox->addItem( tr( "JPEG" ), (uint32_t)VirtualCamera::JPEG );

    m_imageResComboBox = new QComboBox( this );

    m_imageLockButtonGroup = new QButtonGroup( this );
    QCheckBox *aeCheckbox = new QCheckBox( tr("AE"), this);
    QCheckBox *afCheckbox = new QCheckBox( tr("AF"), this);
    QCheckBox *awbCheckbox = new QCheckBox( tr("AWB"), this);

    m_imageLockButtonGroup->setExclusive( false );
    m_imageLockButtonGroup->addButton( aeCheckbox, CAM_ENGINE_LOCK_AEC );
    m_imageLockButtonGroup->addButton( afCheckbox, CAM_ENGINE_LOCK_AF );
    m_imageLockButtonGroup->addButton( awbCheckbox, CAM_ENGINE_LOCK_AWB );

    aeCheckbox->setChecked( false );
    afCheckbox->setChecked( false );
    awbCheckbox->setChecked( false );

    QHBoxLayout *imageLockLayout = new QHBoxLayout;
    imageLockLayout->addWidget( aeCheckbox );
    imageLockLayout->addWidget( afCheckbox );
    imageLockLayout->addWidget( awbCheckbox );

    /*
    doAverageCheckBox = new QCheckBox(tr("Average"));
    doAverageCheckBox->setChecked(false);
    doAverageCheckBox->setEnabled(false);

    // spin box
    numFramesSpinBox = new QDoubleSpinBox;
    numFramesSpinBox->setRange( 1, 99 );
    numFramesSpinBox->setSingleStep( 1 );
    numFramesSpinBox->setDecimals( 0 );
    numFramesSpinBox->setValue( 1 );
*/

    QGridLayout* gridlayout = new QGridLayout();

    gridlayout->addWidget(directoryLabel,    0, 0);
    gridlayout->addWidget(m_folderLineEdit, 0, 1);
    gridlayout->addWidget(browseButton,      0, 2);
    gridlayout->addWidget(imageTypeLabel,    1, 0);
    gridlayout->addWidget(m_imageTypeComboBox, 1, 1);
    gridlayout->addWidget(imageResLabel,    2, 0);
    gridlayout->addWidget(m_imageResComboBox, 2, 1);
    //layout->addWidget(doAverageCheckBox, 1, 2);
    //layout->addWidget(numFramesLabel,    2, 0);
    //layout->addWidget(numFramesSpinBox,  2, 1);
    gridlayout->addWidget(imageLockLabel,    3, 0);
    gridlayout->addLayout(imageLockLayout, 3, 1);
    gridlayout->addWidget(captureButton,     3, 2);

    m_settingsGroupBox = new QGroupBox( this );
    m_settingsGroupBox->setLayout( gridlayout );

    QDialogButtonBox* buttonBox = new QDialogButtonBox ( this );
    buttonBox->setStandardButtons( QDialogButtonBox::Close );

    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget( m_settingsGroupBox );
    verticalLayout->addStretch( 1 );
    verticalLayout->addWidget( buttonBox );
    setLayout( verticalLayout );

    resize( 400, 150 );

    // connect actions
    connect( (QObject*)buttonBox->button( QDialogButtonBox::Close ) , SIGNAL( clicked() ),
          this, SLOT( close() ) );

    connect( browseButton, SIGNAL(clicked()), this, SLOT(browse()) );
    connect( captureButton, SIGNAL(clicked()), this, SLOT(capture()) );

    m_settingsGroupBox->setEnabled( false );
}


void CaptureImageDialog::opened() { }

void CaptureImageDialog::closed() {
    m_settingsGroupBox->setEnabled( false );
}

void CaptureImageDialog::connected() {
#ifndef SUPPORT_PASS_JSON
    return;
#endif
    m_imageResComboBox->clear();
    IsiSensorCaps_t sensorCaps;
    memset( &sensorCaps, 0, sizeof( IsiSensorCaps_t ) );
    {
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CAPS_PARAMS_BASE64] = "";
        uint32_t ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CAPS, jRequest, jResponse);
        if ( 0 == ret ) {
            Json::Value base64bin =  jResponse[SENSOR_CAPS_PARAMS_BASE64]; 
            std::string cfg_parms = base64bin[BASE64_BIN].asString();  
            std::string decodeBuffer = base64_decode(cfg_parms);
            std::copy(decodeBuffer.begin(), decodeBuffer.end(),
                            (unsigned char *)(&sensorCaps));
        } else {
            return;
        }
    }

    for (uint32_t Res=1; Res!=0; Res<<=1) {
        if ( (sensorCaps.Resolution & Res) ==  Res ) {
            QString ResName;
            int ret;
            std::string res_name;
            ret = m_camDevice->post<uint32_t, std::string>(ISPCORE_MODULE_SENSOR_GET_RESOLUTION_NAME, 
                                SENSOR_SENSOR_GET_RESOLUTION_NAME, Res,
                                SENSOR_SENSOR_GET_RESOLUTION_NAME, res_name);
            if ( 0 == ret ) {
                ResName.append( res_name.c_str() );
            } else {
                ResName.append( QString("unknown resolution 0x%1").arg(Res, 8, 16) );
            }
            m_imageResComboBox->addItem( ResName, Res );
        }
    }
    bool available = false;
    //m_camDevice->afAvailableGet( available );

    m_imageLockButtonGroup->button( CAM_ENGINE_LOCK_AEC )->setEnabled( true );
    //m_imageLockButtonGroup->button( CAM_ENGINE_LOCK_AF )->setEnabled( available );
    m_imageLockButtonGroup->button( CAM_ENGINE_LOCK_AF )->setEnabled( false );
    m_imageLockButtonGroup->button( CAM_ENGINE_LOCK_AWB )->setEnabled( true );
}

void CaptureImageDialog::disconnected() {
    m_settingsGroupBox->setEnabled( false );
}

void CaptureImageDialog::started() {
    m_settingsGroupBox->setEnabled( true );
}

void CaptureImageDialog::stopped() {
    m_settingsGroupBox->setEnabled( false );
}

void CaptureImageDialog::showEvent( QShowEvent *event ) {
    if ( true != m_lastPos.isNull() ) {
        move( m_lastPos );
    }
    QDialog::showEvent( event );
}

void CaptureImageDialog::closeEvent( QCloseEvent *event ) {
    // remember last position
    m_lastPos = pos();

    QDialog::closeEvent( event );
}

void CaptureImageDialog::browse() {
    QString dir = QFileDialog::getExistingDirectory(this,
            tr("Select Directory"),
            QDir::currentPath(),
            QFileDialog::DontResolveSymlinks );

    if ( NULL != dir ) {
        m_folderLineEdit->setText( dir );
    }
}

void CaptureImageDialog::capture() {
    QString path = m_folderLineEdit->text();
    int snapshotType = 
        m_imageTypeComboBox->itemData( m_imageTypeComboBox->currentIndex() ).toUInt();
    uint32_t resolution = (uint32_t)m_imageResComboBox->itemData( m_imageResComboBox->currentIndex() ).toUInt();

    uint32_t locks = (uint32_t)CAM_ENGINE_LOCK_NO;
    QList<QAbstractButton *> buttonList = m_imageLockButtonGroup->buttons();
    QAbstractButton *button;
    foreach (button, buttonList) {
        if ( true == button->isChecked() ) {
            locks |= (uint32_t)m_imageLockButtonGroup->id( button );
        }
    }

    QDateTime datetime = QDateTime::currentDateTime();
    QString   SnapshotNameOfTime = datetime.toString(tr("yyyyMMdd")) + "_" + datetime.toString(tr("hhmmss"));
    switch (snapshotType) {
    case CAMDEV_RGB:
        path = path + "/RGB_" + SnapshotNameOfTime;
        break;
    case CAMDEV_RAW8:
        path = path + "/RAW8_" + SnapshotNameOfTime;
        break;
    case CAMDEV_RAW12:
        path = path + "/RAW12_" + SnapshotNameOfTime;
        break;
    case CAMDEV_JPEG:
        path = path + "/JPEG_" + SnapshotNameOfTime;
        break;
    default:
        break;
    }

    ((MainWindow*)parent())->capture( path, snapshotType, resolution, locks );
}
