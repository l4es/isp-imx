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

#include "level2/cprocwidget.h"


#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <string.h>

CProcWidget::CProcWidget( VirtualCamera *camDevice, QWidget *parent )
    : QWidget    ( parent ),
      m_camDevice( camDevice )
{
    createCProcGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_cprocGroupBox );
    mainLayout->addStretch( 1 );
    setLayout( mainLayout );

    connect( parent, SIGNAL(UpdateCamEngineStatus()), SLOT(updateStatus()) );

    m_enableCheckBox->setChecked( false );

    updateStatus();
}


void CProcWidget::createCProcGroupBox()
{
    m_cprocGroupBox = new QGroupBox( tr("Color Processing") );

    QVBoxLayout *mainLayout = new QVBoxLayout;

    m_enableCheckBox = new QCheckBox( tr("Enable") );

    createChromaOutGroupBox();
    createLumaInGroupBox();
    createLumaOutGroupBox();
    createContrastGroupBox();

    connect( m_enableCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( toggleEnable( bool ) ) );

    mainLayout->addWidget( m_enableCheckBox );
    mainLayout->addWidget( m_chromaOutGroupBox );
    mainLayout->addWidget( m_lumaInGroupBox );
    mainLayout->addWidget( m_lumaOutGroupBox );
    mainLayout->addWidget( m_contrastGroupBox );

    m_cprocGroupBox->setLayout( mainLayout );

}

void CProcWidget::createChromaOutGroupBox()
{
    m_chromaOutGroupBox = new QGroupBox( tr("Chrominance clipping range at output") );
    QVBoxLayout *CCROLayout = new QVBoxLayout();

    m_chromaOutComboBox = new QComboBox();
    m_chromaOutComboBox->addItem( tr("BT.601 (16..240)") ,
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_CHROM_RANGE_OUT_BT601 ) );
    m_chromaOutComboBox->addItem( tr("FUll Range (0..255)"),
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_CHROM_RANGE_OUT_FULL_RANGE ));

    connect( m_chromaOutComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeRange() ) );

    CCROLayout->addWidget( m_chromaOutComboBox );

    m_chromaOutGroupBox->setLayout( CCROLayout );
}

void CProcWidget::createLumaInGroupBox()
{
    m_lumaInGroupBox = new QGroupBox( tr("Luminance input range (offset processing)") );
    QVBoxLayout *LIRLayout = new QVBoxLayout();

    m_lumaInComboBox = new QComboBox();
    m_lumaInComboBox->addItem( tr("BT.601 (64..940)"),
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_LUM_RANGE_IN_BT601 ));
    m_lumaInComboBox->addItem( tr("FUll Range (0..1023)"),
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_LUM_RANGE_IN_FULL_RANGE ));

    connect( m_lumaInComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeRange() ) );

    LIRLayout->addWidget( m_lumaInComboBox );

    m_lumaInGroupBox->setLayout( LIRLayout );
}

void CProcWidget::createLumaOutGroupBox()
{
    m_lumaOutGroupBox = new QGroupBox( tr("Luminance clipping range at output") );
    QVBoxLayout *LCROLayout = new QVBoxLayout();

    m_lumaOutComboBox = new QComboBox();
    m_lumaOutComboBox->addItem( tr("BT.601 (16..235)"),
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_LUM_RANGE_OUT_BT601 ));
    m_lumaOutComboBox->addItem( tr("FUll Range (0..255)"),
            QVariant::fromValue<uint32_t>( CAMERIC_CPROC_LUM_RANGE_OUT_FULL_RANGE ));

    connect( m_lumaOutComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeRange() ) );

    LCROLayout->addWidget( m_lumaOutComboBox );

    m_lumaOutGroupBox->setLayout( LCROLayout );
}


void CProcWidget::createContrastGroupBox()
{
    m_contrastGroupBox = new QGroupBox();
    QFormLayout *layout = new QFormLayout();

    m_contrastSpinBox = new QDoubleSpinBox();
    m_contrastSpinBox->setMinimumSize(QSize(120, 22));
    m_contrastSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_contrastSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_contrastSpinBox->setDecimals( 7 );
    m_contrastSpinBox->setRange(0.0f, 1.9921875f);
    m_contrastSpinBox->setSingleStep( 0.0078125f );   // 2/256

    m_contrastSlider = new QSlider();
    m_contrastSlider->setOrientation(Qt::Horizontal);
    m_contrastSlider->setTracking( false );
    m_contrastSpinBox->setRange( 0, 255 );//1.9921875f/0.0078125f+0.5f

    QGridLayout *contrastLayout = new QGridLayout();
    contrastLayout->addWidget( m_contrastSlider, 0, 0, 1, 2 );
    contrastLayout->addWidget( m_contrastSpinBox, 0, 2, 1, 1 );

    m_brightnessSpinBox = new QSpinBox();
    m_brightnessSpinBox->setMinimumSize(QSize(120, 22));
    m_brightnessSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_brightnessSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_brightnessSpinBox->setRange(-128, 127);
    m_brightnessSpinBox->setSingleStep(1);

    m_brightnessSlider = new QSlider();
    m_brightnessSlider->setOrientation(Qt::Horizontal);
    m_brightnessSlider->setTracking( false );
    m_brightnessSlider->setRange(-128, 127);

    QGridLayout *brightnessLayout = new QGridLayout();
    brightnessLayout->addWidget( m_brightnessSlider, 0, 0, 1, 2 );
    brightnessLayout->addWidget( m_brightnessSpinBox, 0, 2, 1, 1 );

    m_saturationSpinBox = new QDoubleSpinBox();
    m_saturationSpinBox->setMinimumSize(QSize(120, 22));
    m_saturationSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_saturationSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_saturationSpinBox->setDecimals( 7 );
    m_saturationSpinBox->setRange(0.0f , 1.9921875f);
    m_saturationSpinBox->setSingleStep(0.0078125f);   // 2/256

    m_saturationSlider = new QSlider();
    m_saturationSlider->setOrientation(Qt::Horizontal);
    m_saturationSlider->setTracking( false );
    m_saturationSlider->setRange( 0, 255 );//1.9921875f/0.0078125f+0.5f

    QGridLayout *saturationLayout = new QGridLayout();
    saturationLayout->addWidget( m_saturationSlider, 0, 0, 1, 2 );
    saturationLayout->addWidget( m_saturationSpinBox, 0, 2, 1, 1 );

    m_hueSpinBox = new QDoubleSpinBox();
    m_hueSpinBox->setMinimumSize(QSize(120, 22));
    m_hueSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_hueSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_hueSpinBox->setDecimals( 6 );
    m_hueSpinBox->setRange(-90.0f , 89.296875f);
    m_hueSpinBox->setSingleStep(0.703125f);           // 90/128

    m_hueSlider = new QSlider();
    m_hueSlider->setOrientation(Qt::Horizontal);
    m_hueSlider->setTracking( false );
    m_hueSlider->setRange( 0, 255 );//(89.296875f+90f)/0.703125f+0.5f

    QGridLayout *hueLayout = new QGridLayout();
    hueLayout->addWidget( m_hueSlider, 0, 0, 1, 2 );
    hueLayout->addWidget( m_hueSpinBox, 0, 2, 1, 1 );

    connect( m_contrastSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setContrast( double ) ) );
    connect( m_contrastSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeContrast( int ) ) );
    connect( m_brightnessSpinBox, SIGNAL( valueChanged( int    ) ), this, SLOT( setBrightness( int ) ) );
    connect( m_brightnessSlider,  SIGNAL( valueChanged( int    ) ), this, SLOT( setBrightness( int ) ) );
    connect( m_saturationSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( setSaturation( double ) ) );
    connect( m_saturationSlider,  SIGNAL( valueChanged( int    ) ), this, SLOT( changeSaturation( int ) ) );
    connect( m_hueSpinBox,        SIGNAL( valueChanged( double ) ), this, SLOT( setHue( double ) ) );
    connect( m_hueSlider,         SIGNAL( valueChanged( int    ) ), this, SLOT( changeHue( int ) ) );

    layout->addRow( tr("Contrast"), contrastLayout );
    layout->addRow( tr("Brightness"), brightnessLayout );
    layout->addRow( tr("Saturation"), saturationLayout );
    layout->addRow( tr("Hue"), hueLayout );

    m_contrastGroupBox->setLayout( layout );
}


void CProcWidget::toggleEnable( bool checked )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    if ( true == checked )
    {
        CamEngineCprocConfig_t config;
        memset( &config, 0, sizeof( CamEngineCprocConfig_t ) );

        config.ChromaOut   = (CamerIcCprocChrominaceRangeOut_t)m_chromaOutComboBox->itemData(
                m_chromaOutComboBox->currentIndex() ).toUInt();
        config.LumaOut     = (CamerIcCprocLuminanceRangeOut_t)m_lumaOutComboBox->itemData(
                m_lumaOutComboBox->currentIndex() ).toUInt();
        config.LumaIn      = (CamerIcCprocLuminanceRangeIn_t)m_lumaInComboBox->itemData(
                m_lumaInComboBox->currentIndex() ).toUInt();

        config.contrast    = (float)m_contrastSpinBox->value();
        config.brightness  = (int8_t)m_brightnessSpinBox->value();
        config.saturation  = (float)m_saturationSpinBox->value();
        config.hue         = (float)m_hueSpinBox->value();

        //m_camDevice->cProcEnable( &config );
    }
    else
    {
        //m_camDevice->cProcDisable();
    }
}


void CProcWidget::changeRange()
{
    if ( true == m_enableCheckBox->isChecked() )
    {
        //disable
        toggleEnable( false );
        //enable
        toggleEnable( true );
    }
}


void CProcWidget::setContrast( double value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_contrastSlider->blockSignals( true );
    m_contrastSpinBox->blockSignals( true );

    //m_camDevice->cProcSetContrast( (float)value );

    int sliderValue = (int)((value - m_contrastSpinBox->minimum()) / m_contrastSpinBox->singleStep() + 0.5f);
    m_contrastSlider->setValue( sliderValue );
    m_contrastSpinBox->setValue( value );

    m_contrastSlider->blockSignals( false );
    m_contrastSpinBox->blockSignals( false );
}


void CProcWidget::changeContrast( int value )
{
    double newContrast = m_contrastSpinBox->minimum() + (double)value * m_contrastSpinBox->singleStep();
    setContrast( newContrast );
}


void CProcWidget::setBrightness( int value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_brightnessSlider->blockSignals( true );
    m_brightnessSpinBox->blockSignals( true );

    //m_camDevice->cProcSetBrightness( (int8_t)value );

    m_brightnessSlider->setValue( value );
    m_brightnessSpinBox->setValue( value );

    m_brightnessSlider->blockSignals( false );
    m_brightnessSpinBox->blockSignals( false );
}


void CProcWidget::setSaturation( double value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_saturationSlider->blockSignals( true );
    m_saturationSpinBox->blockSignals( true );

    //m_camDevice->cProcSetSaturation( (float)value );

    int sliderValue = (int)((value - m_saturationSpinBox->minimum()) / m_saturationSpinBox->singleStep() + 0.5f);
    m_saturationSlider->setValue( sliderValue );
    m_saturationSpinBox->setValue( value );

    m_saturationSlider->blockSignals( false );
    m_saturationSpinBox->blockSignals( false );
}


void CProcWidget::changeSaturation( int value )
{
    double newSaturation = m_saturationSpinBox->minimum() + (double)value * m_saturationSpinBox->singleStep();
    setSaturation( newSaturation );
}


void CProcWidget::setHue( double value )
{
    if ( NULL == m_camDevice )
    {
        return;
    }

    m_hueSlider->blockSignals( true );
    m_hueSpinBox->blockSignals( true );

    //m_camDevice->cProcSetHue( (float)value );

    int sliderValue = (int)((value - m_hueSpinBox->minimum()) / m_hueSpinBox->singleStep() + 0.5f);
    m_hueSlider->setValue( sliderValue );
    m_hueSpinBox->setValue( value );

    m_hueSlider->blockSignals( false );
    m_hueSpinBox->blockSignals( false );
}


void CProcWidget::changeHue( int value )
{
    double newHue = m_hueSpinBox->minimum() + (double)value * m_hueSpinBox->singleStep();
    setHue( newHue );
}


void CProcWidget::updateStatus()
{
#if 0
    if ( CamEngineItf::State::Invalid == m_camDevice->state() )
    {
        setEnabled( false );
        return;
    }
#endif
    setEnabled( true );

    bool running = false;
    CamEngineCprocConfig_t config;
    memset( &config, 0, sizeof( CamEngineCprocConfig_t ) );

    config.contrast      = 1.0f;
    config.brightness    = 0.0f;
    config.saturation    = 1.0f;
    config.hue           = 0.0f;

    //m_camDevice->cProcStatus( running, config );

    m_enableCheckBox->blockSignals( true );
    m_chromaOutComboBox->blockSignals( true );
    m_lumaInComboBox->blockSignals( true );
    m_lumaOutComboBox->blockSignals( true );

    m_enableCheckBox->setChecked( running );

    int chromaOutIdx = m_chromaOutComboBox->findData( config.ChromaOut );
    if ( -1 != chromaOutIdx )
    {
        m_chromaOutComboBox->setCurrentIndex( chromaOutIdx );
    }
    int lumaInIdx = m_lumaInComboBox->findData( config.LumaIn );
    if ( -1 != lumaInIdx )
    {
        m_lumaInComboBox->setCurrentIndex( lumaInIdx );
    }
    int lumaOutIdx = m_lumaOutComboBox->findData( config.LumaOut );
    if ( -1 != lumaOutIdx )
    {
        m_lumaOutComboBox->setCurrentIndex( lumaOutIdx );
    }

    m_enableCheckBox->blockSignals( false );
    m_chromaOutComboBox->blockSignals( false );
    m_lumaInComboBox->blockSignals( false );
    m_lumaOutComboBox->blockSignals( false );

    setContrast( config.contrast );
    setBrightness( config.brightness );
    setSaturation( config.saturation );
    setHue( config.hue );
}

