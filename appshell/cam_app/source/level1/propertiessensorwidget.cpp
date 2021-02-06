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

#include "level1/propertiessensorwidget.h"

#include "level1/isiregmodel.h"

#include <limits>
#include <algorithm>

#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>
#include <string.h>
#include <assert.h>

#define TOSTRING(x) #x

#define ADDCOMBOBOXITEM( comboBox, caps, flag )                         \
        if ( (flag & caps) ==  flag )                                   \
        {                                                               \
            comboBox->addItem( TOSTRING(__##flag##__), (uint32_t)flag );\
        }

#define SELECTCOMBOBOXITEM( comboBox, caps )                            \
    {                                                                   \
        int idx = comboBox->findData( caps );                           \
        if ( -1 != idx )                                                \
        {                                                               \
            comboBox->setCurrentIndex( idx );                           \
        }                                                               \
    }

#define GETCOMBOBOXITEMDATA( comboBox, caps )                           \
    caps = comboBox->itemData( comboBox->currentIndex() ).toUInt()

PropertiesSensorWidget::PropertiesSensorWidget( VirtualCamera* camDevice, QWidget* parent, bool main_notsub )
  : QWidget( parent ),
    m_camDevice( camDevice ),
    m_regModel( new IsiRegModel( NULL, this ) ),
    m_main_notsub( main_notsub ) 
{
    assert( NULL != m_camDevice );
    assert( NULL != m_regModel );

    // create gui elements defined in the Ui::PropertiesSensorWidget class
    setupUi( this );

    registerView->setSelectionBehavior( QAbstractItemView::SelectRows );
    registerView->setSelectionMode( QAbstractItemView::SingleSelection );
    registerView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    registerView->setModel( m_regModel );

    registerRWText->setToolTip( tr( "Flags:"
            "<table>"
            "<tr><td><b>0x00</b></td><td>eTableEnd</td></tr>"
            "<tr><td><b>0x01</b></td><td>eReadable</td></tr>"
            "<tr><td><b>0x02</b></td><td>eWritable</td></tr>"
            "<tr><td><b>0x04</b></td><td>eVolatile</td></tr>"
            "<tr><td><b>0x08</b></td><td>eDelay</td></tr>"
            "<tr><td><b>0x10</b></td><td>eReserved</td></tr>"
            "<tr><td><b>0x20</b></td><td>eNoDefault</td></tr>"
            "<tr><td><b>0x40</b></td><td>eTwoBytes</td></tr>"
            "<tr><td><b>0x80</b></td><td>eFourBytes</td></tr>"
            "</table>" ) );

    // connect actions
    connect( (QObject*)buswidthComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)modeComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)fieldselectionComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)ycseqComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)conv422ComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)bpatComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)hpolComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)vpolComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)edgeComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)blsComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)gammaComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)cconvComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)resolutionComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)dwnszComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)blcComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)agcComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)awbComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)aecComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)dpccComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)smiamodeComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );
    connect( (QObject*)mipimodeComboBox , SIGNAL( currentIndexChanged( int ) ),
            this, SLOT( updateSensorConfig() ) );

    connect( (QObject*)testPatternCheckBox , SIGNAL( toggled( bool ) ),
            this, SLOT( updateTestpattern( bool ) ) );
    connect( (QObject*)d3CheckBox , SIGNAL( toggled( bool ) ),
            this, SLOT( update3D( bool ) ) );

    connect( (QObject*)dumpButton , SIGNAL( clicked() ),
            this, SLOT( dumpRegister() ) );
    connect( (QObject*)registerReadButton , SIGNAL( clicked() ),
            this, SLOT( readRegister() ) );
    connect( (QObject*)registerWriteButton , SIGNAL( clicked() ),
            this, SLOT( writeRegister() ) );
    connect( (QObject*)registerAddrLineEdit , SIGNAL( returnPressed() ),
            this, SLOT( readRegister() ) );
    connect( (QObject*)registerValueLineEdit , SIGNAL( returnPressed() ),
            this, SLOT( writeRegister() ) );

    connect( (QObject*)registerView , SIGNAL( clicked( const QModelIndex & ) ),
            this, SLOT( selectRegister( const QModelIndex & ) ) );
}

void PropertiesSensorWidget::opened()
{
    refreshSensorCaps();
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();
    refreshRegisterAccess();
    refreshRegisterTable();

    updateStatus();
}

void PropertiesSensorWidget::closed()
{
    refreshSensorCaps();
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();
    refreshRegisterAccess();
    refreshRegisterTable();

    updateStatus();
}

void PropertiesSensorWidget::connected()
{
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();

    updateStatus();
}

void PropertiesSensorWidget::disconnected()
{
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();

    updateStatus();
}

void PropertiesSensorWidget::started()
{
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();

    updateStatus();
}

void PropertiesSensorWidget::stopped()
{
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();

    updateStatus();
}

void PropertiesSensorWidget::resChanged()
{
    refreshSensorConfig( true );
}

void PropertiesSensorWidget::updateSensorConfig()
{
    IsiSensorConfig_t sensorConfig;
    memset( &sensorConfig, 0, sizeof( IsiSensorConfig_t ) );
    {
#ifndef SUPPORT_PASS_JSON
        int ret;
        Json::Value jRequest, jResponse;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CFG_GET, jRequest, jResponse);
        if (ret == 0)
            readValueFromBase64(jResponse, SENSOR_CONFIG_PARAMS_BASE64, &sensorConfig);
#endif
    }

    GETCOMBOBOXITEMDATA( buswidthComboBox, sensorConfig.BusWidth );
    GETCOMBOBOXITEMDATA( modeComboBox, sensorConfig.Mode );
    GETCOMBOBOXITEMDATA( fieldselectionComboBox, sensorConfig.FieldSelection );
    GETCOMBOBOXITEMDATA( ycseqComboBox, sensorConfig.YCSequence );
    GETCOMBOBOXITEMDATA( conv422ComboBox, sensorConfig.Conv422 );
    GETCOMBOBOXITEMDATA( bpatComboBox, sensorConfig.BPat );
    GETCOMBOBOXITEMDATA( hpolComboBox, sensorConfig.HPol );
    GETCOMBOBOXITEMDATA( vpolComboBox, sensorConfig.VPol );
    GETCOMBOBOXITEMDATA( edgeComboBox, sensorConfig.Edge );
    GETCOMBOBOXITEMDATA( blsComboBox, sensorConfig.Bls );
    GETCOMBOBOXITEMDATA( gammaComboBox, sensorConfig.Gamma );
    GETCOMBOBOXITEMDATA( cconvComboBox, sensorConfig.CConv );
    GETCOMBOBOXITEMDATA( resolutionComboBox, sensorConfig.Resolution );
    GETCOMBOBOXITEMDATA( dwnszComboBox, sensorConfig.DwnSz );
    GETCOMBOBOXITEMDATA( blcComboBox, sensorConfig.BLC );
    GETCOMBOBOXITEMDATA( agcComboBox, sensorConfig.AGC );
    GETCOMBOBOXITEMDATA( awbComboBox, sensorConfig.AWB );
    GETCOMBOBOXITEMDATA( aecComboBox, sensorConfig.AEC );
    GETCOMBOBOXITEMDATA( dpccComboBox, sensorConfig.DPCC );
    GETCOMBOBOXITEMDATA( smiamodeComboBox, sensorConfig.SmiaMode );
    GETCOMBOBOXITEMDATA( mipimodeComboBox, sensorConfig.MipiMode );

    connected(); // discard changes, was: m_camDevice->setSensorConfig( sensorConfig );
}

void PropertiesSensorWidget::update3D( bool checked )
{

}

void PropertiesSensorWidget::updateTestpattern( bool checked )
{
    connected(); // discard changes, was: m_camDevice->enableTestpattern( checked );
}

void PropertiesSensorWidget::dumpRegister()
{
    std::string fileName = dumpLineEdit->text().toLatin1().constData();
    bool ret;
    {
        Json::Value jRequest, jResponse;
        jRequest[SENSOR_REG_DUMP_FILE_NAME] = fileName;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REG_DUMP2FILE, jRequest, jResponse);
    }
    if ( 0 !=  ret) 
    {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Dumping sensor registers failed!\n" ) );
    }
}

void PropertiesSensorWidget::readRegister()
{
    IsiRegDescription_t description;
    bool ok = true;
    uint32_t addr  = registerAddrLineEdit->text().remove( "0x" ).toUInt( &ok, 16 );
    if (!ok)
    {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Malformed register address!\n" ) );
        return;
    }

    uint32_t value = 0U;

    bool ret;
    {
        Json::Value jRequest, jResponse;
        jRequest[SENSOR_ADDRESS_PARAMS] = addr;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REG_DESCRIPTION, jRequest, jResponse);
        if (ret == 0) {
            description.Addr = jResponse[SENSOR_DESCRIPTION_PARAMS][SENSOR_ADDRESS_PARAMS].asUInt();
            description.DefaultValue = jResponse[SENSOR_DESCRIPTION_PARAMS][SENSOR_DEFAULT_VALUE_PARAMS].asUInt();
            description.Flags = jResponse[SENSOR_DESCRIPTION_PARAMS][SENSOR_FLAGS_PARAMS].asUInt();
            std::string strName = jResponse[SENSOR_DESCRIPTION_PARAMS][SENSOR_NAME_PARAMS].asCString();
            description.pName = new char[strName.size() + 1];
            strcpy((char*)description.pName,  strName.c_str());
        }
    }

    if ( true != ret) {
        QMessageBox::warning( this, tr( "Characterization Application" ),
                tr( "Unknown register!\n" ) );

        registerRWText->setText( tr( "N/A" ) );
        registerNameText->setText( tr( "N/A" ) );
        registerDefaultText->setText( tr( "N/A" ) );
    }
    else
    {
        registerRWText->setText( QString( "0b" ) +
                QString( "%1" ).arg( description.Flags, 8, 2, QLatin1Char( '0' ) ).toUpper() );
        registerNameText->setText( description.pName );
        registerDefaultText->setText( QString( "0x" ) +
                QString( "%1" ).arg( description.DefaultValue, 4, 16, QLatin1Char( '0' ) ).toUpper() );
    }

    {
        Json::Value jRequest, jResponse;
        jRequest[SENSOR_ADDRESS_PARAMS] = addr;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REG_GET, jRequest, jResponse);
        if (ret == 0)
            value = jResponse[SENSOR_VALUE_PARAMS].asUInt();
    }

    if ( true != ret) {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Reading sensor register failed!\n" ) );

        registerValueLineEdit->setText( "0x0000" );
    }
    else
    {
        registerValueLineEdit->setText( QString( "0x" ) +
                QString( "%1" ).arg( value, 4, 16, QLatin1Char( '0' ) ).toUpper() );
    }
}

void PropertiesSensorWidget::writeRegister()
{
    bool ok = true;
    uint32_t addr  = registerAddrLineEdit->text().remove( "0x" ).toUInt( &ok, 16 );
    if (!ok)
    {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Malformed register address!\n" ) );
        return;
    }
    uint32_t value = registerValueLineEdit->text().remove( "0x" ).toUInt( &ok, 16 );;
    if (!ok)
    {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Malformed register value!\n" ) );
        return;
    }

    bool ret;
    {
        Json::Value jRequest, jResponse;
        jRequest[SENSOR_ADDRESS_PARAMS] = addr;
        jResponse[SENSOR_VALUE_PARAMS] = value;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REG_SET, jRequest, jResponse);
    }

    if ( 0 !=  ret) {
        QMessageBox::critical( this, tr( "Characterization Application" ),
                tr( "Writing sensor register failed!\n" ) );
    }
}

void PropertiesSensorWidget::selectRegister( const QModelIndex &index )
{
    QVariant data = index.data( Qt::UserRole ) ;
    registerAddrLineEdit->setText( QString( "0x" ) +
            QString( "%1" ).arg( data.toUInt(), 4, 16, QLatin1Char( '0' ) ).toUpper() );
    readRegister();
}

void PropertiesSensorWidget::updateStatus()
{
    bool enable = false;
#if 0
    switch (  m_camDevice->state() )
    {
        case CamEngineItf::State::Running:
            statusLabel->setText( tr( "Running" ) );
            enable = false;
            break;

        case CamEngineItf::State::Paused:
            statusLabel->setText( tr( "Paused" ) );
            enable = false;
            break;

        case CamEngineItf::State::Idle:
            statusLabel->setText( tr( "Idle" ) );
            enable = true;
            break;

        default:
            statusLabel->setText( tr( "N/A" ) );
            enable = false;
    }
#endif
    capGroupBox->setEnabled( enable );
    testPatternCheckBox->setEnabled( enable );
    //d3CheckBox->setEnabled( enable ? m_camDevice->isBitstream3D() : false );
}

void PropertiesSensorWidget::showEvent( QShowEvent *event )
{
    refreshSensorCaps();
    refreshSensorConfig();

    refresh3D();
    refreshTestpattern();

    updateStatus();

    QWidget::showEvent( event );
}

void PropertiesSensorWidget::refreshSensorCaps()
{
    std::string name ;
    int ret = 0;
    {
        Json::Value jRequest, jResponse;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_NAME_GET, jRequest, jResponse);
        if (ret == 0)
            name = jResponse[SENSOR_NAME_GET].asString();
    }
    if ( !name.empty() )
    {
        nameLabel->setText( name.c_str() );
    }
    else
    {
        nameLabel->setText( tr( "N/A" ) );
    }

    uint32_t revId  = 0;
    {
        Json::Value jRequest, jResponse;

        jRequest[SENSOR_SENSOR_ID_PARAMS] = revId;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_REVISION_GET, jRequest, jResponse);
        if (ret == 0)
            revId = jResponse[SENSOR_SENSOR_ID_PARAMS].asUInt();
    }
    if ( 0 != revId )
    {
        idLabel->setText( QString( "0x" ) + QString( "%1" ).arg( revId, 1, 16 ).toUpper() );
    }
    else
    {
        idLabel->setText( tr( "N/A" ) );
    }
    // connection
     bool sensorConnected;
     {
        Json::Value jRequest, jResponse;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_IS_CONNECTED, jRequest, jResponse);
        if (ret == 0)
            sensorConnected = jResponse[SENSOR_CONNECTION_PARAMS].asBool();
    }

    if (sensorConnected) {
        connectionLabel->setText( tr( "Connected" ) );
    } else {
        connectionLabel->setText( tr( "N/A" ) );
    }
#ifndef SUPPORT_PASS_JSON
    return; 
#endif
    IsiSensorCaps_t sensorCaps;
    memset( &sensorCaps, 0, sizeof( IsiSensorCaps_t ) );
    {
        Json::Value jRequest, jResponse;
        jResponse[SENSOR_CAPS_PARAMS_BASE64] = "";
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CAPS, jRequest, jResponse);
        if (ret == 0) {
            Json::Value base64bin = jResponse[SENSOR_CAPS_PARAMS_BASE64];
            std::string cfg_parms = base64bin.asString();
            std::string decodedString = base64_decode(cfg_parms);
            std::copy(decodedString.begin(), decodedString.end(),
                        (unsigned char *)(&sensorCaps));
        }
    }

    buswidthComboBox->blockSignals( true );
    modeComboBox->blockSignals( true );
    fieldselectionComboBox->blockSignals( true );
    ycseqComboBox->blockSignals( true );
    conv422ComboBox->blockSignals( true );
    bpatComboBox->blockSignals( true );
    hpolComboBox->blockSignals( true );
    vpolComboBox->blockSignals( true );
    edgeComboBox->blockSignals( true );
    blsComboBox->blockSignals( true );
    gammaComboBox->blockSignals( true );
    cconvComboBox->blockSignals( true );
    resolutionComboBox->blockSignals( true );
    dwnszComboBox->blockSignals( true );
    blcComboBox->blockSignals( true );
    agcComboBox->blockSignals( true );
    awbComboBox->blockSignals( true );
    aecComboBox->blockSignals( true );
    dpccComboBox->blockSignals( true );
    smiamodeComboBox->blockSignals( true );
    mipimodeComboBox->blockSignals( true );

    buswidthComboBox->clear();
    ADDCOMBOBOXITEM( buswidthComboBox, sensorCaps.BusWidth, ISI_BUSWIDTH_8BIT_ZZ );
    ADDCOMBOBOXITEM( buswidthComboBox, sensorCaps.BusWidth, ISI_BUSWIDTH_8BIT_EX );
    ADDCOMBOBOXITEM( buswidthComboBox, sensorCaps.BusWidth, ISI_BUSWIDTH_10BIT_EX );
    ADDCOMBOBOXITEM( buswidthComboBox, sensorCaps.BusWidth, ISI_BUSWIDTH_10BIT_ZZ );
    ADDCOMBOBOXITEM( buswidthComboBox, sensorCaps.BusWidth, ISI_BUSWIDTH_12BIT );

    modeComboBox->clear();
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_BT601 );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_BT656 );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_BAYER );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_DATA );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_PICT );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_RGB565 );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_SMIA );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_MIPI );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_BAY_BT656 );
    ADDCOMBOBOXITEM( modeComboBox, sensorCaps.Mode, ISI_MODE_RAW_BT656 );

    fieldselectionComboBox->clear();
    ADDCOMBOBOXITEM( fieldselectionComboBox, sensorCaps.FieldSelection, ISI_FIELDSEL_BOTH );
    ADDCOMBOBOXITEM( fieldselectionComboBox, sensorCaps.FieldSelection, ISI_FIELDSEL_EVEN );
    ADDCOMBOBOXITEM( fieldselectionComboBox, sensorCaps.FieldSelection, ISI_FIELDSEL_ODD );

    ycseqComboBox->clear();
    ADDCOMBOBOXITEM( ycseqComboBox, sensorCaps.YCSequence, ISI_YCSEQ_YCBYCR );
    ADDCOMBOBOXITEM( ycseqComboBox, sensorCaps.YCSequence, ISI_YCSEQ_YCRYCB );
    ADDCOMBOBOXITEM( ycseqComboBox, sensorCaps.YCSequence, ISI_YCSEQ_CBYCRY );
    ADDCOMBOBOXITEM( ycseqComboBox, sensorCaps.YCSequence, ISI_YCSEQ_CRYCBY );

    conv422ComboBox->clear();
    ADDCOMBOBOXITEM( conv422ComboBox, sensorCaps.Conv422, ISI_CONV422_COSITED );
    ADDCOMBOBOXITEM( conv422ComboBox, sensorCaps.Conv422, ISI_CONV422_INTER );
    ADDCOMBOBOXITEM( conv422ComboBox, sensorCaps.Conv422, ISI_CONV422_NOCOSITED );

    bpatComboBox->clear();
    ADDCOMBOBOXITEM( bpatComboBox, sensorCaps.BPat, ISI_BPAT_RGRGGBGB );
    ADDCOMBOBOXITEM( bpatComboBox, sensorCaps.BPat, ISI_BPAT_GRGRBGBG );
    ADDCOMBOBOXITEM( bpatComboBox, sensorCaps.BPat, ISI_BPAT_GBGBRGRG );
    ADDCOMBOBOXITEM( bpatComboBox, sensorCaps.BPat, ISI_BPAT_BGBGGRGR );

    hpolComboBox->clear();
    ADDCOMBOBOXITEM( hpolComboBox, sensorCaps.HPol, ISI_HPOL_SYNCPOS );
    ADDCOMBOBOXITEM( hpolComboBox, sensorCaps.HPol, ISI_HPOL_SYNCNEG );
    ADDCOMBOBOXITEM( hpolComboBox, sensorCaps.HPol, ISI_HPOL_REFPOS );
    ADDCOMBOBOXITEM( hpolComboBox, sensorCaps.HPol, ISI_HPOL_REFNEG );

    vpolComboBox->clear();
    ADDCOMBOBOXITEM( vpolComboBox, sensorCaps.VPol, ISI_VPOL_POS );
    ADDCOMBOBOXITEM( vpolComboBox, sensorCaps.VPol, ISI_VPOL_NEG );

    edgeComboBox->clear();
    ADDCOMBOBOXITEM( edgeComboBox, sensorCaps.Edge, ISI_EDGE_RISING );
    ADDCOMBOBOXITEM( edgeComboBox, sensorCaps.Edge, ISI_EDGE_FALLING );

    blsComboBox->clear();
    ADDCOMBOBOXITEM( blsComboBox, sensorCaps.Bls, ISI_BLS_OFF );
    ADDCOMBOBOXITEM( blsComboBox, sensorCaps.Bls, ISI_BLS_TWO_LINES );
    ADDCOMBOBOXITEM( blsComboBox, sensorCaps.Bls, ISI_BLS_FOUR_LINES );
    ADDCOMBOBOXITEM( blsComboBox, sensorCaps.Bls, ISI_BLS_SIX_LINES );

    gammaComboBox->clear();
    ADDCOMBOBOXITEM( gammaComboBox, sensorCaps.Gamma, ISI_GAMMA_ON );
    ADDCOMBOBOXITEM( gammaComboBox, sensorCaps.Gamma, ISI_GAMMA_OFF );

    cconvComboBox->clear();
    ADDCOMBOBOXITEM( cconvComboBox, sensorCaps.CConv, ISI_CCONV_ON );
    ADDCOMBOBOXITEM( cconvComboBox, sensorCaps.CConv, ISI_CCONV_OFF );

    resolutionComboBox->clear();
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_VGA );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_2592_1944 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_3264_2448 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_4416_3312 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV720P5 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV720P10 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV720P15 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV720P30 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV720P60 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P5);
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P6);
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P10);
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P12);
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P15 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P24 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P25 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P30 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P50 );
    ADDCOMBOBOXITEM( resolutionComboBox, sensorCaps.Resolution, ISI_RES_TV1080P60 );

    dwnszComboBox->clear();
    ADDCOMBOBOXITEM( dwnszComboBox, sensorCaps.DwnSz, ISI_DWNSZ_SUBSMPL );
    ADDCOMBOBOXITEM( dwnszComboBox, sensorCaps.DwnSz, ISI_DWNSZ_SCAL_BAY );
    ADDCOMBOBOXITEM( dwnszComboBox, sensorCaps.DwnSz, ISI_DWNSZ_SCAL_COS );

    blcComboBox->clear();
    ADDCOMBOBOXITEM( blcComboBox, sensorCaps.BLC, ISI_BLC_AUTO );
    ADDCOMBOBOXITEM( blcComboBox, sensorCaps.BLC, ISI_BLC_OFF );

    agcComboBox->clear();
    ADDCOMBOBOXITEM( agcComboBox, sensorCaps.AGC, ISI_AGC_AUTO );
    ADDCOMBOBOXITEM( agcComboBox, sensorCaps.AGC, ISI_AGC_OFF );

    awbComboBox->clear();
    ADDCOMBOBOXITEM( awbComboBox, sensorCaps.AWB, ISI_AWB_AUTO );
    ADDCOMBOBOXITEM( awbComboBox, sensorCaps.AWB, ISI_AWB_OFF );

    aecComboBox->clear();
    ADDCOMBOBOXITEM( aecComboBox, sensorCaps.AEC, ISI_AEC_AUTO );
    ADDCOMBOBOXITEM( aecComboBox, sensorCaps.AEC, ISI_AEC_OFF );

    dpccComboBox->clear();
    ADDCOMBOBOXITEM( dpccComboBox, sensorCaps.DPCC, ISI_DPCC_AUTO );
    ADDCOMBOBOXITEM( dpccComboBox, sensorCaps.DPCC, ISI_DPCC_OFF );

    smiamodeComboBox->clear();
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_COMPRESSED );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_8_TO_10_DECOMP );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_12 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_10 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_8 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_7 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RAW_6 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RGB_888 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RGB_565 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_RGB_444 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_YUV_420 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_MODE_YUV_422 );
    ADDCOMBOBOXITEM( smiamodeComboBox, sensorCaps.SmiaMode, ISI_SMIA_OFF );

    mipimodeComboBox->clear();
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV420_8 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV420_10 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_LEGACY_YUV420_8 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV420_CSPS_8 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV420_CSPS_10 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV422_8 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_YUV422_10 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RGB444 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RGB555 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RGB565 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RGB666 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RGB888 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RAW_6 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RAW_7 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RAW_8 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RAW_10 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_MODE_RAW_12 );
    ADDCOMBOBOXITEM( mipimodeComboBox, sensorCaps.MipiMode, ISI_MIPI_OFF );

    buswidthComboBox->blockSignals( false );
    modeComboBox->blockSignals( false );
    fieldselectionComboBox->blockSignals( false );
    ycseqComboBox->blockSignals( false );
    conv422ComboBox->blockSignals( false );
    bpatComboBox->blockSignals( false );
    hpolComboBox->blockSignals( false );
    vpolComboBox->blockSignals( false );
    edgeComboBox->blockSignals( false );
    blsComboBox->blockSignals( false );
    gammaComboBox->blockSignals( false );
    cconvComboBox->blockSignals( false );
    resolutionComboBox->blockSignals( false );
    dwnszComboBox->blockSignals( false );
    blcComboBox->blockSignals( false );
    agcComboBox->blockSignals( false );
    awbComboBox->blockSignals( false );
    aecComboBox->blockSignals( false );
    dpccComboBox->blockSignals( false );
    smiamodeComboBox->blockSignals( false );
    mipimodeComboBox->blockSignals( false );
}

void PropertiesSensorWidget::refreshSensorConfig( bool resOnly )
{
    IsiSensorConfig_t sensorConfig;
    memset( &sensorConfig, 0, sizeof( IsiSensorConfig_t ) );
    int ret = 0;
    {

#ifndef SUPPORT_PASS_JSON
         Json::Value jRequest, jResponse;
         ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_CFG_GET, jRequest, jResponse);
         if (ret == 0) {
            std::string cfg_parms = jResponse[SENSOR_CONFIG_PARAMS_BASE64].asString();  

            std::string decodedString = base64_decode(cfg_parms);
            std::copy(decodedString.begin(), decodedString.end(), (unsigned char *)(&sensorConfig));
         }
#endif    
     }

    if (!resOnly)
    {
        buswidthComboBox->blockSignals( true );
        modeComboBox->blockSignals( true );
        fieldselectionComboBox->blockSignals( true );
        ycseqComboBox->blockSignals( true );
        conv422ComboBox->blockSignals( true );
        bpatComboBox->blockSignals( true );
        hpolComboBox->blockSignals( true );
        vpolComboBox->blockSignals( true );
        edgeComboBox->blockSignals( true );
        blsComboBox->blockSignals( true );
        gammaComboBox->blockSignals( true );
        cconvComboBox->blockSignals( true );
        dwnszComboBox->blockSignals( true );
        blcComboBox->blockSignals( true );
        agcComboBox->blockSignals( true );
        awbComboBox->blockSignals( true );
        aecComboBox->blockSignals( true );
        dpccComboBox->blockSignals( true );
        smiamodeComboBox->blockSignals( true );
        mipimodeComboBox->blockSignals( true );

        SELECTCOMBOBOXITEM( buswidthComboBox, sensorConfig.BusWidth );
        SELECTCOMBOBOXITEM( modeComboBox, sensorConfig.Mode );
        SELECTCOMBOBOXITEM( fieldselectionComboBox, sensorConfig.FieldSelection );
        SELECTCOMBOBOXITEM( ycseqComboBox, sensorConfig.YCSequence );
        SELECTCOMBOBOXITEM( conv422ComboBox, sensorConfig.Conv422 );
        SELECTCOMBOBOXITEM( bpatComboBox, sensorConfig.BPat );
        SELECTCOMBOBOXITEM( hpolComboBox, sensorConfig.HPol );
        SELECTCOMBOBOXITEM( vpolComboBox, sensorConfig.VPol );
        SELECTCOMBOBOXITEM( edgeComboBox, sensorConfig.Edge );
        SELECTCOMBOBOXITEM( blsComboBox, sensorConfig.Bls );
        SELECTCOMBOBOXITEM( gammaComboBox, sensorConfig.Gamma );
        SELECTCOMBOBOXITEM( cconvComboBox, sensorConfig.CConv );
        SELECTCOMBOBOXITEM( dwnszComboBox, sensorConfig.DwnSz );
        SELECTCOMBOBOXITEM( blcComboBox, sensorConfig.BLC );
        SELECTCOMBOBOXITEM( agcComboBox, sensorConfig.AGC );
        SELECTCOMBOBOXITEM( awbComboBox, sensorConfig.AWB );
        SELECTCOMBOBOXITEM( aecComboBox, sensorConfig.AEC );
        SELECTCOMBOBOXITEM( dpccComboBox, sensorConfig.DPCC );
        SELECTCOMBOBOXITEM( smiamodeComboBox, sensorConfig.SmiaMode );
        SELECTCOMBOBOXITEM( mipimodeComboBox, sensorConfig.MipiMode );

        buswidthComboBox->blockSignals( false );
        modeComboBox->blockSignals( false );
        fieldselectionComboBox->blockSignals( false );
        ycseqComboBox->blockSignals( false );
        conv422ComboBox->blockSignals( false );
        bpatComboBox->blockSignals( false );
        hpolComboBox->blockSignals( false );
        vpolComboBox->blockSignals( false );
        edgeComboBox->blockSignals( false );
        blsComboBox->blockSignals( false );
        gammaComboBox->blockSignals( false );
        cconvComboBox->blockSignals( false );
        dwnszComboBox->blockSignals( false );
        blcComboBox->blockSignals( false );
        agcComboBox->blockSignals( false );
        awbComboBox->blockSignals( false );
        aecComboBox->blockSignals( false );
        dpccComboBox->blockSignals( false );
        smiamodeComboBox->blockSignals( false );
        mipimodeComboBox->blockSignals( false );
    }

    resolutionComboBox->blockSignals( true );
    SELECTCOMBOBOXITEM( resolutionComboBox, sensorConfig.Resolution );
    resolutionComboBox->blockSignals( false );
}

void PropertiesSensorWidget::refresh3D()
{
    //d3CheckBox->setChecked(  m_camDevice->is3DEnabled() );
}

void PropertiesSensorWidget::refreshTestpattern()
{
    Json::Value jRequest, jResponse;
    int ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_IS_TEST_PATTERN, jRequest, jResponse);
    if (ret == 0) {
        bool isTestPattern = jResponse[SENSOR_TEST_PATTERN_PARAMS].asBool();
        testPatternCheckBox->setChecked(  isTestPattern );
    }
}

void PropertiesSensorWidget::refreshRegisterAccess()
{
     bool sensorConnected;
     {
        Json::Value jRequest, jResponse;
        int ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_IS_CONNECTED, jRequest, jResponse);
        if (ret == 0)
            sensorConnected = jResponse[SENSOR_CONNECTION_PARAMS].asBool();
    }

    if ( true == sensorConnected) {
        QString fileAppendix = QString( "" ) + QString( "_register_dump.txt" );
        QString fileName = QDir( QDir::currentPath() ).filePath( fileAppendix );
        std::string name;
        int ret = 0;
        {
            Json::Value jRequest, jResponse;
            ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_NAME_GET, jRequest, jResponse);
            if (ret == 0)
                name = jResponse[SENSOR_NAME_GET].asString();
        }
        if ( name.empty() )
        {
            fileName = QDir( QDir::currentPath() ).filePath( QString( name.c_str() ) + fileAppendix );
        }
        dumpLineEdit->setText( fileName );
    }
    else
    {
        dumpLineEdit->setText( tr( "N/A" ) );
        registerAddrLineEdit->setText( "0x0000" );
        registerValueLineEdit->setText( "0x0000" );
        registerRWText->setText( tr( "N/A" ) );
        registerNameText->setText( tr( "N/A" ) );
        registerDefaultText->setText( tr( "N/A" ) );
    }
}

IsiRegDescription_t regTable;
void PropertiesSensorWidget::refreshRegisterTable()
{
     int ret = 0;
     bool sensorConnected;
     {
        Json::Value jRequest, jResponse;
        ret = m_camDevice->ioctl(ISPCORE_MODULE_SENSOR_IS_CONNECTED, jRequest, jResponse);
        if (ret == 0)
            sensorConnected = jResponse[SENSOR_CONNECTION_PARAMS].asBool();
    }

    if ( true == sensorConnected) //m_camDevice->sensor().isConnected() 
    {
#if 0
        delete m_regModel;
        m_camDevice->sensor().registerTableGet(&regTable);
        m_regModel = new IsiRegModel(&regTable, this );
        assert( m_regModel != NULL );
        registerView->setModel( m_regModel );
#endif
    }
    else
    {
        delete m_regModel;
        m_regModel = new IsiRegModel( NULL, this );
        assert( m_regModel != NULL );
        registerView->setModel( m_regModel );
    }

}
