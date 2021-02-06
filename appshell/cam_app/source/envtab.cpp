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

#include "envtab.h"


#include <assert.h>

//TODO: Change to API headers
/**< BLC */
#define ISI_BLC_AUTO                        0x00000001      /**< Camera BlackLevelCorrection on */
#define ISI_BLC_OFF                         0x00000002      /**< Camera BlackLevelCorrection off */
/**< AGC */
#define ISI_AGC_AUTO                        0x00000001      /**< Camera AutoGainControl on */
#define ISI_AGC_OFF                         0x00000002      /**< Camera AutoGainControl off */
/**< AWB */
#define ISI_AWB_AUTO                        0x00000001      /**< Camera AutoWhiteBalance on */
#define ISI_AWB_OFF                         0x00000002      /**< Camera AutoWhiteBalance off */
/**< AEC */
#define ISI_AEC_AUTO                        0x00000001      /**< Camera AutoExposureControl on */
#define ISI_AEC_OFF                         0x00000002      /**< Camera AutoExposureControl off */
/**< DPCC */
#define ISI_DPCC_AUTO                       0x00000001      /**< Camera DefectPixelCorrection on */
#define ISI_DPCC_OFF                        0x00000002      /**< Camera DefectPixelCorrection off */
/**< AFPS */
#define ISI_AFPS_NOTSUPP                    0x00000000      /**< Auto FPS mode not supported; or ISI_RES_XXX bitmask of all resolutions being part of any AFPS series */




#include <QComboBox>

#define ERROR_DLG QErrorMessage::qtHandler()->showMessage

EnvironmentTab::EnvironmentTab(  VirtualCamera* camDevice, QWidget *parent)
    : QTabWidget(parent),
      m_camDevice( camDevice )
{
    assert( NULL != m_camDevice );

    //createProjectInfoGroupBox();
    createGeneralInfoGroupBox();
    createSensorSetupGroupBox();
    createFlickerAvoidInfoGroupBox();
    createSensorInfoGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    //mainLayout->addWidget( ProjectInfoGroupBox );
    mainLayout->addWidget( GeneralInfoGroupBox );
    mainLayout->addWidget( SensorSetupGroupBox);
    mainLayout->addWidget( FlickerAvoidInfoGroupBox );
    mainLayout->addWidget( SensorInfoGroupBox);

    setLayout( mainLayout );
}

void EnvironmentTab::createSensorAInfoGroupBox() {
    SensorAInfoGroupBox = new QGroupBox(tr("Sensor A"), this);
    QGridLayout *layout = new QGridLayout;

    QLabel *LblSensorName     = new QLabel( tr("Sensor Name") );
    LblSensorName->setFixedWidth( 175 );
    QLabel *LblSensorStatus    = new QLabel( tr("Sensor Status") );
    LblSensorStatus->setFixedWidth( 175 );
    QLabel *LblSensorID           = new QLabel( tr("Sensor ID/REVISION") );
    LblSensorID->setFixedWidth( 175 );
    QLabel *LblSensorConnect = new QLabel( tr("Connection to CamerIC") );
    LblSensorConnect->setFixedWidth( 175 );

    QLabel *LblSensorCap        = new QLabel( tr("Sensor Capabilities") );
    LblSensorCap->setFixedWidth( 175 );

    QLabel *LblBayerPattern    = new QLabel( tr("Bayer Pattern") );
    LblBayerPattern->setFixedWidth( 175 );
    QLabel *LblBusWidth           = new QLabel( tr("Bus width") );
    LblBusWidth->setFixedWidth( 175 );
    QLabel *LblBlkLvlCtrl           = new QLabel( tr("Black Level Control") );
    LblBlkLvlCtrl->setFixedWidth( 175 );

    QLabel *LblAutoGainCtrl     = new QLabel( tr("Auto Gain Control") );
    LblAutoGainCtrl->setFixedWidth( 175 );
    QLabel *LblAutoExpCtrl      = new QLabel( tr("Auto Exposure Control") );
    LblAutoExpCtrl->setFixedWidth( 175 );
    QLabel *LblAutoWB             = new QLabel( tr("Auto White Balance") );
    LblAutoWB->setFixedWidth( 175 );
    QLabel *LblDefectPC           = new QLabel( tr("Defect Pixel Correction") );
    LblDefectPC->setFixedWidth( 175 );

    QLabel *LblMipiMode           = new QLabel( tr("Mipi Mode") );
    LblMipiMode->setFixedWidth( 175 );

    layout->addWidget( LblSensorName,       2, 0 );
    layout->addWidget( LblSensorStatus,     3, 0 );
    layout->addWidget( LblSensorID,         4, 0 );
    layout->addWidget( LblSensorConnect,    5, 0 );

    layout->addWidget( LblSensorCap,        6, 0 );

    layout->addWidget( LblBayerPattern,     7, 0 );
    layout->addWidget( LblBusWidth,         8, 0 );
    layout->addWidget( LblMipiMode,         9, 0 );
    layout->addWidget( LblBlkLvlCtrl,      10, 0 );

    layout->addWidget( LblAutoGainCtrl,    11, 0 );
    layout->addWidget( LblAutoExpCtrl,     12, 0 );
    layout->addWidget( LblAutoWB,          13, 0 );
    layout->addWidget( LblDefectPC,        14, 0 );

    QString             string;
    string.sprintf( "%s", "N/A");
    LblSensorANameValue = new QLabel( string );
    LblSensorANameValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorAStatusValue = new QLabel( string );
    LblSensorAStatusValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorAIDValue = new QLabel( string );
    LblSensorAIDValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorAConnectValue = new QLabel( string );
    LblSensorAConnectValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSABayerPatternValue = new QLabel( string );
    LblSABayerPatternValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSABusWidthValue = new QLabel( string );
    LblSABusWidthValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSABlkLvlCtrlValue = new QLabel( string );
    LblSABlkLvlCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSAAutoGainCtrlValue = new QLabel( string );
    LblSAAutoGainCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSAAutoExpCtrlValue = new QLabel( string );
    LblSAAutoExpCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSAAutoWBValue = new QLabel( string );
    LblSAAutoWBValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSADefectPCValue = new QLabel( string );
    LblSADefectPCValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSAMipiModeValue = new QLabel( string );
    LblSAMipiModeValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    layout->addWidget( LblSensorANameValue,     2, 1 );
    layout->addWidget( LblSensorAStatusValue,   3, 1 );
    layout->addWidget( LblSensorAIDValue,       4, 1 );
    layout->addWidget( LblSensorAConnectValue,  5, 1 );

    layout->addWidget( LblSABayerPatternValue,  7, 1 );
    layout->addWidget( LblSABusWidthValue,      8, 1 );
    layout->addWidget( LblSAMipiModeValue,      9, 1 );
    layout->addWidget( LblSABlkLvlCtrlValue,   10, 1 );

    layout->addWidget( LblSAAutoGainCtrlValue, 11, 1 );
    layout->addWidget( LblSAAutoExpCtrlValue,  12, 1 );
    layout->addWidget( LblSAAutoWBValue,       13, 1 );
    layout->addWidget( LblSADefectPCValue,     14, 1 );

    layout->setRowStretch( 15, 1 );

    SensorAInfoGroupBox->setLayout(layout);
}



void EnvironmentTab::createSensorBInfoGroupBox() {
    SensorBInfoGroupBox = new QGroupBox(tr("Sensor B"), this);
    QGridLayout *layout = new QGridLayout;

    QLabel *LblSensorName     = new QLabel( tr("Sensor Name") );
    LblSensorName->setFixedWidth( 175 );
    QLabel *LblSensorStatus    = new QLabel( tr("Sensor Status") );
    LblSensorStatus->setFixedWidth( 175 );
    QLabel *LblSensorID           = new QLabel( tr("Sensor ID/REVISION") );
    LblSensorID->setFixedWidth( 175 );
    QLabel *LblSensorConnect = new QLabel( tr("Connection to CamerIC") );
    LblSensorConnect->setFixedWidth( 175 );

    QLabel *LblSensorCap        = new QLabel( tr("Sensor Capabilities") );
    LblSensorCap->setFixedWidth( 175 );

    QLabel *LblBayerPattern    = new QLabel( tr("Bayer Pattern") );
    LblBayerPattern->setFixedWidth( 175 );
    QLabel *LblBusWidth           = new QLabel( tr("Bus width") );
    LblBusWidth->setFixedWidth( 175 );
    QLabel *LblBlkLvlCtrl           = new QLabel( tr("Black Level Control") );
    LblBlkLvlCtrl->setFixedWidth( 175 );

    QLabel *LblAutoGainCtrl     = new QLabel( tr("Auto Gain Control") );
    LblAutoGainCtrl->setFixedWidth( 175 );
    QLabel *LblAutoExpCtrl      = new QLabel( tr("Auto Exposure Control") );
    LblAutoExpCtrl->setFixedWidth( 175 );
    QLabel *LblAutoWB             = new QLabel( tr("Auto White Balance") );
    LblAutoWB->setFixedWidth( 175 );
    QLabel *LblDefectPC           = new QLabel( tr("Defect Pixel Correction") );
    LblDefectPC->setFixedWidth( 175 );

    QLabel *LblMipiMode           = new QLabel( tr("Mipi Mode") );
    LblMipiMode->setFixedWidth( 175 );

    layout->addWidget( LblSensorName,       2, 0 );
    layout->addWidget( LblSensorStatus,     3, 0 );
    layout->addWidget( LblSensorID,         4, 0 );
    layout->addWidget( LblSensorConnect,    5, 0 );

    layout->addWidget( LblSensorCap,        6, 0 );

    layout->addWidget( LblBayerPattern,     7, 0 );
    layout->addWidget( LblBusWidth,         8, 0 );
    layout->addWidget( LblMipiMode,         9, 0 );
    layout->addWidget( LblBlkLvlCtrl,      10, 0 );

    layout->addWidget( LblAutoGainCtrl,    11, 0 );
    layout->addWidget( LblAutoExpCtrl,     12, 0 );
    layout->addWidget( LblAutoWB,          13, 0 );
    layout->addWidget( LblDefectPC,        14, 0 );

    QString             string;
    string.sprintf( "%s", "N/A");
    LblSensorBNameValue = new QLabel( string );
    LblSensorBNameValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorBStatusValue = new QLabel( string );
    LblSensorBStatusValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorBIDValue = new QLabel( string );
    LblSensorBIDValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSensorBConnectValue = new QLabel( string );
    LblSensorBConnectValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBBayerPatternValue = new QLabel( string );
    LblSBBayerPatternValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBBusWidthValue = new QLabel( string );
    LblSBBusWidthValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBBlkLvlCtrlValue = new QLabel( string );
    LblSBBlkLvlCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBAutoGainCtrlValue = new QLabel( string );
    LblSBAutoGainCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);


    LblSBAutoExpCtrlValue = new QLabel( string );
    LblSBAutoExpCtrlValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBAutoWBValue = new QLabel( string );
    LblSBAutoWBValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBDefectPCValue = new QLabel( string );
    LblSBDefectPCValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    LblSBMipiModeValue = new QLabel( string );
    LblSBMipiModeValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    layout->addWidget( LblSensorBNameValue,     2, 1 );
    layout->addWidget( LblSensorBStatusValue,   3, 1 );
    layout->addWidget( LblSensorBIDValue,       4, 1 );
    layout->addWidget( LblSensorBConnectValue,  5, 1 );

    layout->addWidget( LblSBBayerPatternValue,  7, 1 );
    layout->addWidget( LblSBBusWidthValue,      8, 1 );
    layout->addWidget( LblSBMipiModeValue,      9, 1 );
    layout->addWidget( LblSBBlkLvlCtrlValue,   10, 1 );

    layout->addWidget( LblSBAutoGainCtrlValue, 11, 1 );
    layout->addWidget( LblSBAutoExpCtrlValue,  12, 1 );
    layout->addWidget( LblSBAutoWBValue,       13, 1 );
    layout->addWidget( LblSBDefectPCValue,     14, 1 );

    layout->setRowStretch( 15, 1 );

    SensorBInfoGroupBox->setLayout(layout);
}

void EnvironmentTab::clearSensorInfo() {
    QString   string( tr( "N/A") );

    LblSensorANameValue->setText(string);
    LblSensorAStatusValue->setText(string);
    LblSensorAIDValue->setText(string);
    LblSensorAConnectValue->setText(string);

    LblSensorBNameValue->setText(string);
    LblSensorBStatusValue->setText(string);
    LblSensorBIDValue->setText(string);
    LblSensorBConnectValue->setText(string);

    LblSensorDriverValue->setText(string);
    LblCalibDataValue->setText(string);


    testpatternCheckBox->setEnabled( false );
   
    flickerPeriodComboBox->setEnabled( false );
    ResolutionComboBox->setEnabled( false );
    AfpsCheckBox->setEnabled( false );

 
    testpatternCheckBox->blockSignals( true );
    testpatternCheckBox->setChecked( false );
    testpatternCheckBox->blockSignals( false );

    ResolutionComboBox->clear();
    ResolutionComboBox->addItem("<load sensor driver first>", QVariant::fromValue<uint32_t>( 0 ) );
    AfpsCheckBox->setChecked( false );
}

void EnvironmentTab::clearSensorConfig() {
    QString   string( tr( "N/A") );

    LblSABayerPatternValue->setText(string);
    LblSABusWidthValue->setText(string);
    LblSABlkLvlCtrlValue->setText(string);
    LblSAAutoGainCtrlValue->setText(string);
    LblSAAutoExpCtrlValue->setText(string);
    LblSAAutoWBValue->setText(string);
    LblSADefectPCValue->setText(string);
    LblSAMipiModeValue->setText(string);

    LblSBBayerPatternValue->setText(string);
    LblSBBusWidthValue->setText(string);
    LblSBBlkLvlCtrlValue->setText(string);
    LblSBAutoGainCtrlValue->setText(string);
    LblSBAutoExpCtrlValue->setText(string);
    LblSBAutoWBValue->setText(string);
    LblSBDefectPCValue->setText(string);
    LblSBMipiModeValue->setText(string);

    testpatternCheckBox->setEnabled( false );

    ResolutionComboBox->setEnabled( false );
    AfpsCheckBox->setEnabled( false );

  
    testpatternCheckBox->blockSignals( true );
    ResolutionComboBox->blockSignals( true );
    AfpsCheckBox->blockSignals( true );
    testpatternCheckBox->setChecked( false );
    ResolutionComboBox->clear();
    ResolutionComboBox->addItem("<load sensor driver first>", QVariant::fromValue<uint32_t>( 0 ) );
    AfpsCheckBox->setChecked( false );
    testpatternCheckBox->blockSignals( false );
    ResolutionComboBox->blockSignals( false );
    AfpsCheckBox->blockSignals( false );
}

void EnvironmentTab::updateSensorInfo() {
    std::string name;
    int ret = m_camDevice->post<int, std::string>(ISPCORE_MODULE_SENSOR_NAME_GET, 0, 0, SENSOR_NAME_GET, name);
    if( 0 != ret ) {
        return;
    }
    //m_camDevice->sensor().nameGet(name);
    if ( !name.empty() )
    {
        LblSensorANameValue->setText( name.c_str() );
    }

    // status
    LblSensorAStatusValue->setText( tr( "Ready" ) );

    // revision
    uint32_t inId = 0;
    uint32_t revId = 0;
    ret = m_camDevice->post<uint32_t, uint32_t>(ISPCORE_MODULE_SENSOR_REVISION_GET,
                        SENSOR_SENSOR_ID_PARAMS, inId, SENSOR_SENSOR_ID_PARAMS, revId);

    if ( 0 != revId ) {
        LblSensorAIDValue->setText( QString( "0x" ) + QString( "%1" ).arg( revId, 1, 16 ).toUpper() );
    }

    bool sensorAConnected;
    ret = m_camDevice->post<int, bool>(ISPCORE_MODULE_SENSOR_IS_CONNECTED, 0, 0, SENSOR_CONNECTION_PARAMS, sensorAConnected);

    if ( true == sensorAConnected ) {
        LblSensorAConnectValue->setText( tr( "Connected" ) );
    }

#if 0    // the second sensor
    bool sensorBConnected = false;
    if ( true == sensorBConnected ) {
        // name
        if ( !name.empty() ) {
            LblSensorBNameValue->setText( name.c_str() );
        }

        // status
        LblSensorBStatusValue->setText( tr( "Ready" ) );

        // revision
        uint32_t revId = 0;
        {
            Json::Value jRequest, jResponse;
            jRequest[SENSOR_SENSOR_ID_PARAMS] = revId;
            ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REVISION_GET, jRequest, jResponse);
            assert( 0 == ret );
            revId = jResponse[SENSOR_SENSOR_ID_PARAMS].asUInt();
        }
        if ( 0 != revId )
        {
            LblSensorBIDValue->setText( QString( "0x" ) + QString( "%1" ).arg( revId, 1, 16 ).toUpper() );
        }

        // connection
        LblSensorBConnectValue->setText( tr( "Connected" ) );
    }
    else
    {
        LblSensorBConnectValue->setText( tr( "Not connected" ) );
    }

    //LblSensorDriverValue->setText( m_camDevice->sensorDriverFile() );
    //LblCalibDataValue->setText( m_camDevice->calibrationDataFile() );

    sensorBCheckBox->setEnabled( sensorBConnected );
#endif

    testpatternCheckBox->setEnabled( true );

    flickerPeriodComboBox->setEnabled( false );

    ResolutionComboBox->setEnabled( true );

    sensorBCheckBox->blockSignals( true );
    testpatternCheckBox->blockSignals( true );
    {
        bool isTestPattern;
        ret = m_camDevice->post<int, bool>(ISPCORE_MODULE_SENSOR_IS_TEST_PATTERN, 0, 0, SENSOR_TEST_PATTERN_PARAMS, isTestPattern);
        testpatternCheckBox->setChecked( isTestPattern );
    }
    sensorBCheckBox->blockSignals( false );
    testpatternCheckBox->blockSignals( false );

    IsiSensorConfig_t sensorConfig;
    memset( &sensorConfig, 0, sizeof( sensorConfig ) );
    {
#ifndef SUPPORT_PASS_JSON
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CONFIG_PARAMS_BASE64] = "";
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CFG_GET, jRequest, jResponse);
        if ( 0 == ret ) {
            readValueFromBase64(jResponse, SENSOR_CONFIG_PARAMS_BASE64, &sensorConfig);
        }
#endif
    }

#ifndef SUPPORT_PASS_JSON
    IsiSensorCaps_t sensorCaps;
    memset( &sensorCaps, 0, sizeof( sensorCaps ) );
    {
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CAPS_PARAMS_BASE64] = "";
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CAPS, jRequest, jResponse);
        if ( 0 == ret )
            readValueFromBase64(jResponse, SENSOR_CAPS_PARAMS_BASE64, &sensorConfig);
    }
    ResolutionComboBox->blockSignals( true );
    {
        // add all supported resolutions
        ResolutionComboBox->clear();
        for (uint32_t Res=1; Res!=0; Res<<=1) {
            if ( (sensorCaps.Resolution & Res) ==  Res ) {
                QString ResName;
                char *szResName = NULL;

                //TODO: change IsiGetResolutionName to IOCTL cmd
                //if ( RET_SUCCESS == IsiGetResolutionName(Res, &szResName) ) {
                if ( 0 ) {
                    ResName.append( szResName );
                } else {
                    ResName.append( QString("unknown resolution 0x%1").arg(Res, 8, 16) );
                }

                if ( (sensorCaps.AfpsResolutions & Res) ==  Res ) {
                    ResolutionComboBox->addItem( QIcon(":/images/afps.png"), ResName, Res );
                } else {
                    ResolutionComboBox->addItem( QIcon(":/images/noafps.png"), ResName, Res );
                }
            }
        }

        // select sensor default resolution
        int idx = ResolutionComboBox->findData( QVariant::fromValue<uint32_t>(sensorConfig.Resolution) );
        if (idx >= 0) {
            ResolutionComboBox->setCurrentIndex( idx );
        }
    }
    ResolutionComboBox->blockSignals( false );

#endif
    AfpsCheckBox->blockSignals( true );
    {
        //TODO: Chagne to IOCTL cmds
        //bool AfpsSupport = (sensorCaps.AfpsResolutions != ISI_AFPS_NOTSUPP);
        bool AfpsSupport = false;
        AfpsCheckBox->setEnabled( AfpsSupport );
        if (!AfpsSupport) {
            AfpsCheckBox->setChecked( false );
        }
    }
    AfpsCheckBox->blockSignals( false );
}


void EnvironmentTab::updateSensorConfig() {
    uint32_t ret;
    IsiSensorConfig_t sensorConfig;
    memset( &sensorConfig, 0, sizeof( IsiSensorConfig_t ) );
    {
#ifndef SUPPORT_PASS_JSON
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CONFIG_PARAMS_BASE64] = "";
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CFG_GET, jRequest, jResponse);
        if ( 0 == ret )
            readValueFromBase64(jResponse, SENSOR_CONFIG_PARAMS_BASE64, &sensorConfig);
#endif
    }


    QString blcText  = ( ISI_BLC_AUTO  == sensorConfig.BLC  ) ? tr( "ON" ) : tr ("OFF");
    QString agcText  = ( ISI_AGC_AUTO  == sensorConfig.AGC  ) ? tr( "ON" ) : tr ("OFF");
    QString awbText  = ( ISI_AWB_AUTO  == sensorConfig.AWB  ) ? tr( "AWB ON" ) : tr ("AWB OFF");
    QString aecText  = ( ISI_AEC_AUTO  == sensorConfig.AEC  ) ? tr( "ON" ) : tr ("OFF");
    QString dpccText = ( ISI_DPCC_AUTO == sensorConfig.DPCC ) ? tr( "DPC ENABLE" ) : tr ("DPC DISABLE");


    LblSABlkLvlCtrlValue->setText(blcText);
    LblSAAutoGainCtrlValue->setText(agcText);
    LblSAAutoExpCtrlValue->setText(aecText);
    LblSAAutoWBValue->setText(awbText);
    LblSADefectPCValue->setText(dpccText);

//TODO: change to IOCTL functions
#if 0
    LblSABayerPatternValue->setText( QString( isiCapDescription<CamerIcIspBayerPattern_t>( sensorConfig.BPat ) ) );
    LblSABusWidthValue->setText( QString( isiCapDescription<CamerIcIspInputSelection_t>( sensorConfig.BusWidth ) ) );
    LblSAMipiModeValue->setText( QString( isiCapDescription<MipiDataType_t>( sensorConfig.MipiMode ) ) );
#else
    LblSABayerPatternValue->setText( QString( "BAYERPATTERN" ) );
    LblSABusWidthValue->setText( QString( "BUS WIDTH") );
    LblSAMipiModeValue->setText( QString( "MIPI MODE" ) );
#endif

    bool sensorConnected;

    ret = m_camDevice->post<int, bool>(ISPCORE_MODULE_SENSOR_IS_CONNECTED, 0, 0, SENSOR_CONNECTION_PARAMS, sensorConnected);

    if ( true == sensorConnected ) {
        LblSBBlkLvlCtrlValue->setText(blcText);
        LblSBAutoGainCtrlValue->setText(agcText);
        LblSBAutoExpCtrlValue->setText(aecText);
        LblSBAutoWBValue->setText(awbText);
        LblSBDefectPCValue->setText(dpccText);

//TODO: change to IOCTL functions
#if 0
        LblSBBayerPatternValue->setText( QString( isiCapDescription<CamerIcIspBayerPattern_t>( sensorConfig.BPat ) ) );
        LblSBBusWidthValue->setText( QString( isiCapDescription<CamerIcIspInputSelection_t>( sensorConfig.BusWidth ) ) );
        LblSBMipiModeValue->setText( QString( isiCapDescription<MipiDataType_t>( sensorConfig.MipiMode ) ) );
#else
    LblSABayerPatternValue->setText( QString( "BAYERPATTERN" ) );
    LblSABusWidthValue->setText( QString( "BUS WIDTH") );
    LblSAMipiModeValue->setText( QString( "MIPI MODE" ) );
#endif
    }


    ResolutionComboBox->blockSignals( true );
    {
        // select current resolution
        int idx = ResolutionComboBox->findData( QVariant::fromValue<uint32_t>(sensorConfig.Resolution) );
        if (idx >= 0) {
            ResolutionComboBox->setCurrentIndex( idx );
        }
    }
    ResolutionComboBox->blockSignals( false );
}

void EnvironmentTab::resChanged() {
}

void EnvironmentTab::createSensorSetupGroupBox() {
    SensorSetupGroupBox = new QGroupBox(tr("Sensor Setup"));

    QLabel *LblSensorDriver    = new QLabel( tr("Sensor Driver") );
    LblSensorDriver->setFixedWidth( 150 );

    LblSensorDriverValue = new QLabel( tr("N/A") );
    LblSensorDriverValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *LblCalibData    = new QLabel( tr("Calibration Data") );
    LblCalibData->setFixedWidth( 150 );

    LblCalibDataValue = new QLabel( tr("N/A") );
    LblCalibDataValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QPushButton* sensorDrvButton  = new QPushButton(tr("Load Sensor Driver"));
    sensorDrvButton->setFixedWidth( 150 );

    sensorBCheckBox     = new QCheckBox( tr("Enable 3D (Sensor A && B)") );
    testpatternCheckBox = new QCheckBox( tr("Enable Testpattern") );

    ResolutionComboBox = new QComboBox;
    ResolutionComboBox->setEditable( false );

    AfpsCheckBox = new QCheckBox( tr("Enable AFPS") );

    QGridLayout *layout = new QGridLayout;

    layout->addWidget( sensorDrvButton,        0, 0 );
    layout->addWidget( LblSensorDriver,        1, 0 );
    layout->addWidget( LblSensorDriverValue,   1, 1, 1, 4 );
    layout->addWidget( LblCalibData,           2, 0 );
    layout->addWidget( LblCalibDataValue,      2, 1, 1, 4 );

    layout->addWidget( AfpsCheckBox,           3, 0 );
    layout->addWidget( ResolutionComboBox,     3, 1, 1, 3 );

    layout->addWidget( testpatternCheckBox,    4, 0 );
    layout->addWidget( sensorBCheckBox,        5, 0 );

    SensorSetupGroupBox->setLayout( layout );

    connect(sensorDrvButton, SIGNAL(clicked()), this, SIGNAL(sensorSelectButtonClicked()));

    //connect(sensorBCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(enable3DToggled(bool)));

    //connect(testpatternCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(enableTestpatternToggled(bool)));

    connect(ResolutionComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(resolutionChanged()));
    connect(AfpsCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(enableAfpsToggled(bool)));

    testpatternCheckBox->setEnabled( false );
    ResolutionComboBox->setEnabled( false );
    AfpsCheckBox->setEnabled( false );
}

void EnvironmentTab::createSensorInfoGroupBox() {
    SensorInfoGroupBox = new QGroupBox(tr("Sensor Information"));

    QGridLayout *layout = new QGridLayout;

    createSensorAInfoGroupBox();
    createSensorBInfoGroupBox();

    layout->addWidget( SensorAInfoGroupBox, 2, 0 );
    layout->addWidget( SensorBInfoGroupBox, 2, 1 );

    SensorInfoGroupBox->setLayout( layout );

    clearSensorInfo();
}

void EnvironmentTab::createFlickerAvoidInfoGroupBox() {
    FlickerAvoidInfoGroupBox = new QGroupBox( tr("Flicker Avoidance") );
    QGridLayout *layout = new QGridLayout;

    QLabel *LblFrickerPeriod = new QLabel( tr("Flicker Period"));
    LblFrickerPeriod->setFixedWidth( 150 );
    layout->addWidget( LblFrickerPeriod, 1, 0 );

    flickerPeriodComboBox = new QComboBox;
    flickerPeriodComboBox->setEditable( false );
    //flickerPeriodComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    flickerPeriodComboBox->addItem("disabled"  , QVariant::fromValue<uint32_t>( CAM_ENGINE_FLICKER_OFF   ) );
    flickerPeriodComboBox->addItem("100HZ", QVariant::fromValue<uint32_t>( CAM_ENGINE_FLICKER_100HZ ) );
    flickerPeriodComboBox->addItem("120HZ", QVariant::fromValue<uint32_t>( CAM_ENGINE_FLICKER_120HZ ) );
    flickerPeriodComboBox->setEnabled( false );

    flickerPeriodComboBox->setCurrentIndex( 1 );
    layout->addWidget( flickerPeriodComboBox, 1, 1 );

    connect( flickerPeriodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(flickerPeriodChanged()));

    FlickerAvoidInfoGroupBox->setLayout(layout);
}

void EnvironmentTab::createGeneralInfoGroupBox() {
    GeneralInfoGroupBox = new QGroupBox( tr("General Information") );
    QGridLayout *layout = new QGridLayout;

    QLabel *LblSwVersion    = new QLabel( tr("Software Version") );
    LblSwVersion->setFixedWidth( 150 );
    QLabel *LblBitStreamId  = new QLabel( tr("Bitstream-ID") );
    LblBitStreamId->setFixedWidth( 150 );
    QLabel *LblCamerIcId    = new QLabel( tr("CamerIc-ID") );
    LblCamerIcId->setFixedWidth( 150 );

    layout->addWidget( LblSwVersion,   1, 0 );
    layout->addWidget( LblBitStreamId, 2, 0 );
    layout->addWidget( LblCamerIcId,   3, 0 );

    // print software version
    QLabel *LblSwVersionValue = new QLabel( QString( "1.0.1" ) );//m_camDevice->softwareVersion().c_str() ) );
    LblSwVersionValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    uint32_t bitStreamId, camericID;
    int ret = m_camDevice->post<int, uint32_t>(ISPCORE_MODULE_DEVICE_BITSTREAMID, 0, 0, DEVICE_BITSTREAM_ID_PARAMS, bitStreamId);
    ret = m_camDevice->post<int, uint32_t>(ISPCORE_MODULE_DEVICE_CAMERIC_ID, 0, 0, DEVICE_CAMER_ID_PARAMS, camericID);

    // prepare bitstream id string
    QLabel *LblBitStreamIdValue = new QLabel( QString( "0x" ) +
            QString( "%1" ).arg( bitStreamId, 8, 16, QLatin1Char( '0' ) ).toUpper() );

    LblBitStreamIdValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // print CamerIc version
    QLabel *LblCamerIcIdValue = new QLabel( QString( "0x" ) +
            QString( "%1" ).arg( camericID, 8, 16, QLatin1Char( '0' ) ).toUpper() );

    LblCamerIcIdValue->setFrameStyle( QFrame::Panel | QFrame::Sunken );

    layout->addWidget( LblSwVersionValue,   1, 1 );
    layout->addWidget( LblBitStreamIdValue, 2, 1 );
    layout->addWidget( LblCamerIcIdValue,   3, 1 );

    GeneralInfoGroupBox->setLayout(layout);
}


void EnvironmentTab::createProjectInfoGroupBox() {
    ProjectInfoGroupBox = new QGroupBox( tr("Project Information") );
    QGridLayout *layout = new QGridLayout;

    QLabel *LblProjectFile      = new QLabel( tr("Project") );
    LblProjectFile->setFixedWidth( 150 );
    layout->addWidget( LblProjectFile, 1, 0 );

    QLabel *LblProjectFileValue = new QLabel( tr(" ") );
    LblProjectFileValue->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    layout->addWidget( LblProjectFileValue,   1, 1 );

    ProjectInfoGroupBox->setLayout(layout);
}

CamEngineFlickerPeriod_t EnvironmentTab::flickerPeriod() const {
    return (CamEngineFlickerPeriod_t)flickerPeriodComboBox->itemData( flickerPeriodComboBox->currentIndex() ).toUInt();
}

uint32_t EnvironmentTab::selectedResolution() const {
    return (uint32_t)ResolutionComboBox->itemData( ResolutionComboBox->currentIndex() ).toUInt();
}

void EnvironmentTab::opened() {
    updateSensorInfo();
    updateSensorConfig();
}

void EnvironmentTab::closed() {
    clearSensorInfo();
    clearSensorConfig();
}
