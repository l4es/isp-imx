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

#include <QtGui>
#include "level2tab.h"
#include <QtGui>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QtMultimedia>
#include "cac/cac-plot.h"
#include <math.h>
#include <QList>
#include <limits>

#include "level2/cprocwidget.h"
#include "level2/imgeffectswidget.h"
#include "level2/wdrwidget.h"
#include <assert.h>

static const QSize resultSize(600, 600);

Level2Tab::Level2Tab( VirtualCamera *camDevice, QWidget *parent )
    : QTabWidget(parent),
      m_camDevice( camDevice )
{
    QTabWidget *tabWidget = new QTabWidget;

    tabWidget->addTab( new EcTab( m_camDevice, this ), tr("EC") );
    tabWidget->addTab( new BlsTab( m_camDevice, this ), tr("BLS") );
    tabWidget->addTab( new WbTab( m_camDevice, this ), tr("WB") );
#ifdef ISP_WDR_V2
    //tabWidget->addTab( new WdrWidget( m_camDevice, this ), tr("WDR2") );
#endif

    tabWidget->addTab( new WdrWidget( m_camDevice, this ), tr("HDR") );
    //tabWidget->addTab( new WdrWidget( m_camDevice, this ), tr("WDR") );
    tabWidget->addTab( new DpfTab( m_camDevice, this ), tr("DPF") );
    tabWidget->addTab( new DpccTab( m_camDevice, this ), tr("DPCC") );
    tabWidget->addTab( new CProcWidget( m_camDevice, this ), tr("Color Processing") );
    tabWidget->addTab( new ImgEffectsWidget( m_camDevice, this ), tr("Image Effects") );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
    connect( parent, SIGNAL( opened() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( closed() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( connected() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( disconnected() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( started() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( stopped() ), this, SIGNAL( UpdateCamEngineStatus() ) );
}

EcTab::EcTab
(
    VirtualCamera *camDevice,
    QWidget *parent
)
: QWidget( parent ),
m_camDevice( camDevice )
{
    EcBlockGroupBox = new QGroupBox( tr("Exposure Control") );

    createEcBlockGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( EcBlockGroupBox );
    setLayout( mainLayout );

    updateStatus();
}

void EcTab::createEcBlockGroupBox() {
    QGridLayout* layout = new QGridLayout;

    gainGroupBox  = new QGroupBox( tr("Gain") );
    itimeGroupBox = new QGroupBox( tr("Integration Time") );
    expGroupBox = new QGroupBox( tr("Exposure") );

    EcRefreshButton = new QPushButton(tr("&Refresh"));
    EcRefreshButton->setFixedWidth( 120 );

    createGainGroupBox();
    createItimeGroupBox();
    createExpGroupBox();

    layout->addWidget( EcRefreshButton, 0, 0, Qt::AlignRight );
    layout->addWidget( gainGroupBox   , 1, 0 );
    layout->addWidget( itimeGroupBox  , 2, 0 );
    layout->addWidget( expGroupBox    , 3, 0 );
    layout->setRowStretch( 4, 1 );

    EcBlockGroupBox->setLayout( layout );

    connect( parent()       , SIGNAL( UpdateCamEngineStatus() ), SLOT( updateStatus() ) );
    connect( EcRefreshButton, SIGNAL( clicked()               ), SLOT( updateStatus() ) );
}

void EcTab::createGainGroupBox()
{
    gainGridLayout = new QGridLayout();

    gainHorizontalSlider = new QSlider( gainGroupBox );
    QSizePolicy sizePolicy1( QSizePolicy::Expanding, QSizePolicy::Fixed );
    sizePolicy1.setHorizontalStretch( 1 );
    sizePolicy1.setVerticalStretch( 0 );
    sizePolicy1.setHeightForWidth( gainHorizontalSlider->sizePolicy().hasHeightForWidth() );
    gainHorizontalSlider->setSizePolicy( sizePolicy1 );
    gainHorizontalSlider->setOrientation( Qt::Horizontal );
    gainHorizontalSlider->setTracking( false );

    gainGridLayout->addWidget( gainHorizontalSlider, 0, 0, 1, 2 );

    gainSpinBox = new QDoubleSpinBox( gainGroupBox );
    QSizePolicy sizePolicy2( QSizePolicy::Minimum, QSizePolicy::Fixed );
    sizePolicy2.setHorizontalStretch( 0 );
    sizePolicy2.setVerticalStretch( 0 );
    sizePolicy2.setHeightForWidth( gainSpinBox->sizePolicy().hasHeightForWidth() );
    gainSpinBox->setSizePolicy( sizePolicy2 );
    gainSpinBox->setMinimumSize( QSize(100, 22) );
    gainSpinBox->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    gainGridLayout->addWidget( gainSpinBox, 0, 2, 1, 1 );

    gainMinText = new QLabel( tr("min"), gainGroupBox );
    QSizePolicy sizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    sizePolicy.setHeightForWidth( gainMinText->sizePolicy().hasHeightForWidth() );
    gainMinText->setSizePolicy( sizePolicy );
    gainMinText->setMinimumSize( QSize(150, 0) );

    gainGridLayout->addWidget( gainMinText, 1, 0, 1, 1 );

    gainMaxText = new QLabel( tr("max"), gainGroupBox );
    sizePolicy.setHeightForWidth( gainMaxText->sizePolicy().hasHeightForWidth() );
    gainMaxText->setSizePolicy( sizePolicy );
    gainMaxText->setMinimumSize( QSize(150, 0) );
    gainMaxText->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    gainGridLayout->addWidget( gainMaxText, 1, 1, 1, 1 );

    gainGroupBox->setLayout( gainGridLayout );

    connect( gainSpinBox         , SIGNAL( valueChanged( double ) ),  this, SLOT( updateGain( double ) ) );
    connect( gainHorizontalSlider, SIGNAL( valueChanged( int ) )   ,  this, SLOT( changeGain( int )    ) );
}

void EcTab::createItimeGroupBox()
{
    itimeGridLayout = new QGridLayout();

    itimeHorizontalSlider = new QSlider( itimeGroupBox );
    QSizePolicy sizePolicy1( QSizePolicy::Expanding, QSizePolicy::Fixed );
    sizePolicy1.setHorizontalStretch( 1 );
    sizePolicy1.setVerticalStretch( 0 );
    sizePolicy1.setHeightForWidth( itimeHorizontalSlider->sizePolicy().hasHeightForWidth() );
    itimeHorizontalSlider->setSizePolicy( sizePolicy1 );
    itimeHorizontalSlider->setOrientation( Qt::Horizontal );
    itimeHorizontalSlider->setTracking( false );

    itimeGridLayout->addWidget( itimeHorizontalSlider, 0, 0, 1, 2 );

    itimeSpinBox = new QDoubleSpinBox( itimeGroupBox );
    QSizePolicy sizePolicy2( QSizePolicy::Minimum, QSizePolicy::Fixed );
    sizePolicy2.setHorizontalStretch( 0 );
    sizePolicy2.setVerticalStretch( 0 );
    sizePolicy2.setHeightForWidth( itimeSpinBox->sizePolicy().hasHeightForWidth() );
    itimeSpinBox->setSizePolicy( sizePolicy2 );
    itimeSpinBox->setMinimumSize( QSize(100, 22) );
    itimeSpinBox->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    itimeGridLayout->addWidget( itimeSpinBox, 0, 2, 1, 1 );

    itimeMinText = new QLabel( tr("min"), itimeGroupBox );
    QSizePolicy sizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    sizePolicy.setHeightForWidth( itimeMinText->sizePolicy().hasHeightForWidth() );
    itimeMinText->setSizePolicy( sizePolicy );
    itimeMinText->setMinimumSize( QSize(150, 0) );

    itimeGridLayout->addWidget( itimeMinText, 1, 0, 1, 1 );

    itimeMaxText = new QLabel( tr("max"), itimeGroupBox );
    sizePolicy.setHeightForWidth( itimeMaxText->sizePolicy().hasHeightForWidth() );
    itimeMaxText->setSizePolicy( sizePolicy );
    itimeMaxText->setMinimumSize( QSize(150, 0) );
    itimeMaxText->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    itimeGridLayout->addWidget( itimeMaxText, 1, 1, 1, 1 );

    itimeGroupBox->setLayout( itimeGridLayout );

    connect( itimeSpinBox         , SIGNAL( valueChanged( double ) ), this, SLOT( updateIntegrationTime( double ) ) );
    connect( itimeHorizontalSlider, SIGNAL( valueChanged( int ) )   , this, SLOT( changeIntegrationTime( int ) ) );
}

void EcTab::createExpGroupBox()
{
    expGridLayout = new QGridLayout();

    expHorizontalSlider = new QSlider( expGroupBox );
    QSizePolicy sizePolicy1( QSizePolicy::Expanding, QSizePolicy::Fixed );
    sizePolicy1.setHorizontalStretch( 1 );
    sizePolicy1.setVerticalStretch( 0 );
    sizePolicy1.setHeightForWidth( expHorizontalSlider->sizePolicy().hasHeightForWidth() );
    expHorizontalSlider->setSizePolicy( sizePolicy1 );
    expHorizontalSlider->setOrientation( Qt::Horizontal );
    expHorizontalSlider->setTracking( false );
    expHorizontalSlider->setEnabled( false );

    expGridLayout->addWidget( expHorizontalSlider, 0, 0, 1, 2 );

    expSpinBox = new QDoubleSpinBox( expGroupBox );
    QSizePolicy sizePolicy2( QSizePolicy::Minimum, QSizePolicy::Fixed );
    sizePolicy2.setHorizontalStretch( 0 );
    sizePolicy2.setVerticalStretch( 0 );
    sizePolicy2.setHeightForWidth( expSpinBox->sizePolicy().hasHeightForWidth() );
    expSpinBox->setSizePolicy( sizePolicy2 );
    expSpinBox->setMinimumSize( QSize(100, 22) );
    expSpinBox->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );
    expSpinBox->setEnabled( false );

    expGridLayout->addWidget( expSpinBox, 0, 2, 1, 1 );

    expMinText = new QLabel( tr("min"), expGroupBox );
    QSizePolicy sizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    sizePolicy.setHeightForWidth( expMinText->sizePolicy().hasHeightForWidth() );
    expMinText->setSizePolicy( sizePolicy );
    expMinText->setMinimumSize( QSize(150, 0) );

    expGridLayout->addWidget( expMinText, 1, 0, 1, 1 );

    expMaxText = new QLabel( tr("max"), expGroupBox );
    sizePolicy.setHeightForWidth( expMaxText->sizePolicy().hasHeightForWidth() );
    expMaxText->setSizePolicy( sizePolicy );
    expMaxText->setMinimumSize( QSize(150, 0) );
    expMaxText->setAlignment( Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter );

    expGridLayout->addWidget( expMaxText, 1, 1, 1, 1 );

    expGroupBox->setLayout( expGridLayout );

    connect( expSpinBox         , SIGNAL( valueChanged( double ) ), this, SLOT( updateExposure( double ) ) );
    connect( expHorizontalSlider, SIGNAL( valueChanged( int ) )   , this, SLOT( changeExposure( int ) ) );
}

void EcTab::updateStatus()
{
    refreshGain();
    refreshIntegrationTime();
    refreshExposure();
}

bool EcTab::getGain( float &gain )
{
    //return m_camDevice->getGain( gain );
    return true;
}

bool EcTab::getIntegrationTime( float &time )
{
    //return m_camDevice->getIntegrationTime( time );
    return true;
}

bool EcTab::getExposure( float &exp )
{
    bool result = true;
    float gain;
    float time;

    result &= getGain( gain );
    result &= getIntegrationTime( time );

    if (result)
    {
        exp = gain * time;
    }

    return result;
}

bool EcTab::getGainLimits( float &minGain, float &maxGain, float &stepGain )
{
    //return m_camDevice->getGainLimits( minGain, maxGain, stepGain );
    return true;
}

bool EcTab::getIntegrationTimeLimits( float &minTime, float &maxTime, float &stepTime )
{
    //return m_camDevice->getIntegrationTimeLimits( minTime, maxTime, stepTime );
    return true;
}

bool EcTab::getExposureLimits( float &minExp, float &maxExp, float &stepExp )
{
    bool success = true;

    float minGain;
    float maxGain;
    float stepGain;

    float minTime;
    float maxTime;
    float stepTime;

    success &= getGainLimits( minGain, maxGain, stepGain );
    success &= getIntegrationTimeLimits( minTime, maxTime, stepTime );

    if (success)
    {
        minExp = minGain * minTime;
        maxExp = maxGain * maxTime;
        stepExp = 0.001f; // arbitrary small value
    }

    return success;
}



bool EcTab::setGain( float newGain, float &setGain )
{
    //return m_camDevice->setGain( newGain, setGain );
    return true;
}



bool EcTab::setIntegrationTime( float newTime, float &setTime )
{
    //return m_camDevice->setIntegrationTime( newTime, setTime );
    return true;
}



bool EcTab::setExposure( float newExp, float &setExp )
{
    //return m_camDevice->setExposure( newExp, setExp );
    return true;
}



void EcTab::updateGain( double value )
{
//    if ( CamEngine::State::Invalid == m_camDevice.state() )
//    {
//        return;
//    }

    float newGain = (float)value;
    newGain = (newGain < m_minGain) ? m_minGain : newGain;
    newGain = (newGain > m_maxGain) ? m_maxGain : newGain;

    float setGain = -1.0f;
    if ( true != this->setGain( newGain, setGain ) )
    {
        //QMessageBox::warning( this, tr( "Exposure Control" ), tr( "Couldn't set gain!" ) );
    }

    refreshGain( setGain );
    updateStatus();
}



void EcTab::updateIntegrationTime( double value )
{
//    if ( CamEngine::State::Invalid == m_camDevice.state() )
//    {
//        return;
//    }

    float newItime = (float)(value/1000.0);
    newItime = (newItime < m_minItime) ? m_minItime : newItime;
    newItime = (newItime > m_maxItime) ? m_maxItime : newItime;

    float setItime = -1.0f;
    if ( true != setIntegrationTime( newItime, setItime ) )
    {
        //QMessageBox::warning( this, tr( "Exposure Control" ), tr( "Couldn't set integration time!" ) );
    }

    refreshIntegrationTime( setItime );
    updateStatus();
}



void EcTab::updateExposure( double value )
{
//    if ( CamEngine::State::Invalid == m_camDevice.state() )
//    {
//        return;
//    }

    float newExp = (float)value;
    newExp = (newExp < m_minExp) ? m_minExp : newExp;
    newExp = (newExp > m_maxExp) ? m_maxExp : newExp;

    float setExp = -1.0f;
    if ( true != setExposure( newExp, setExp ) )
    {
        //QMessageBox::warning( this, tr( "Exposure Control" ), tr( "Couldn't set exposure!" ) );
    }

    refreshExposure( setExp );
    updateStatus();
}



void EcTab::changeGain( int value )
{
    double newGain = (double)m_minGain + (double)(value * m_stepGain);
    updateGain( newGain );
}



void EcTab::changeIntegrationTime( int value )
{
    double newItime = (double)m_minItime + (double)(value * m_stepItime);
    updateIntegrationTime( newItime * 1000.0 );
}



void EcTab::changeExposure( int value )
{
    double newExp = (double)m_minExp + (double)(value * m_stepExp);
    updateExposure( newExp );
}



void EcTab::refreshGain( float value )
{
    bool enabled = true; //!aeCheckBox->isChecked();

    if ( ( true == enabled ) && ( 0.0f >= value ) )
    {
        enabled &= getGain( value );
    }

    m_minGain  = 0.0f;
    m_maxGain  = 1.0f;
    m_stepGain = 0.1f;

    if ( true == enabled )
    {
        enabled &= getGainLimits( m_minGain, m_maxGain, m_stepGain );
    }

    gainHorizontalSlider->blockSignals( true );
    gainSpinBox->blockSignals( true );

    // text
    gainMinText->setText( QString().setNum( m_minGain, 'f', 2 ).append(tr(" x")) );
    gainMaxText->setText( QString().setNum( m_maxGain, 'f', 2 ).append(tr(" x")) );

    // slider
    int sliderSteps = std::max( (m_stepGain > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(m_maxGain - m_minGain) / m_stepGain + 0.5f) : 1, 1 );
    int sliderValue = std::max( (m_stepGain > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(value - m_minGain) / m_stepGain + 0.5f) : 0, 0 );
    gainHorizontalSlider->setRange( 0, sliderSteps );
    gainHorizontalSlider->setSingleStep( 1 );
    gainHorizontalSlider->setPageStep( 40 );
    gainHorizontalSlider->setValue( sliderValue );

    // spin box
    gainSpinBox->setRange( m_minGain, m_maxGain );
    gainSpinBox->setSingleStep( m_stepGain );
    gainSpinBox->setDecimals( 4 );
    gainSpinBox->setValue( value );

    gainHorizontalSlider->blockSignals( false );
    gainSpinBox->blockSignals( false );

    gainGroupBox->setEnabled( enabled );
// these children will be enabled/disabled automatically by Qt
//    gainHorizontalSlider->setEnabled( enabled );
//    gainSpinBox->setEnabled( enabled );
//    gainMinText->setEnabled( enabled );
//    gainMaxText->setEnabled( enabled );
    if (!enabled)
    {
        gainMinText->setText( tr("min") );
        gainMaxText->setText( tr("max") );
    }
}



void EcTab::refreshIntegrationTime( float value )
{
    bool enabled = true; //!aeCheckBox->isChecked();

    if ( ( true == enabled ) && ( 0.0f >= value ) )
    {
        enabled &= getIntegrationTime( value );
    }

    m_minItime  = 0.0f;
    m_maxItime  = 1.0f;
    m_stepItime = 0.1f;

    if ( true == enabled )
    {
        enabled &= getIntegrationTimeLimits( m_minItime, m_maxItime, m_stepItime );
    }

    itimeHorizontalSlider->blockSignals( true );
    itimeSpinBox->blockSignals( true );

    // text
    itimeMinText->setText( QString().setNum( m_minItime * 1000.0f, 'f', 2 ).append(tr(" ms")) );
    itimeMaxText->setText( QString().setNum( m_maxItime * 1000.0f, 'f', 2 ).append(tr(" ms")) );

    //slider
    int sliderSteps = std::max( (m_stepItime > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(m_maxItime - m_minItime) / m_stepItime + 0.5f) : 1, 1 );
    int sliderValue = std::max( (m_stepItime > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(value - m_minItime) / m_stepItime + 0.5f) : 0, 0 );
    itimeHorizontalSlider->setRange( 0, sliderSteps );
    itimeHorizontalSlider->setSingleStep( 1 );
    itimeHorizontalSlider->setPageStep( 40 );
    itimeHorizontalSlider->setValue( sliderValue );

    //spin box
    itimeSpinBox->setRange( m_minItime * 1000.0f, m_maxItime * 1000.0f );
    itimeSpinBox->setSingleStep( m_stepItime * 1000.0f );
    itimeSpinBox->setDecimals( 3 );
    itimeSpinBox->setValue( value * 1000.0f );

    itimeHorizontalSlider->blockSignals( false );
    itimeSpinBox->blockSignals( false );

    itimeGroupBox->setEnabled( enabled );
// these children will be enabled/disabled automatically by Qt
//    itimeHorizontalSlider->setEnabled( enabled );
//    itimeSpinBox->setEnabled( enabled );
//    itimeMinText->setEnabled( enabled );
//    itimeMaxText->setEnabled( enabled );
    if (!enabled)
    {
        itimeMinText->setText( tr("min") );
        itimeMaxText->setText( tr("max") );
    }
}

void EcTab::refreshExposure( float value )
{
    bool enabled = true; //!aeCheckBox->isChecked();

    if ( ( true == enabled ) && ( 0.0f >= value ) )
    {
        enabled &= getExposure( value );
    }

    m_minExp  = 0.0f;
    m_maxExp  = 1.0f;
    m_stepExp = 0.1f;

    if ( true == enabled )
    {
        enabled &= getExposureLimits( m_minExp, m_maxExp, m_stepExp );
    }

    expHorizontalSlider->blockSignals( true );
    expSpinBox->blockSignals( true );

    // text
    expMinText->setText( QString().setNum( m_minExp, 'f', 2 ).append(tr(" units")) );
    expMaxText->setText( QString().setNum( m_maxExp, 'f', 2 ).append(tr(" units")) );

    // slider
    int sliderSteps = std::max( (m_stepExp > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(m_maxExp - m_minExp) / m_stepExp + 0.5f) : 1, 1 );
    int sliderValue = std::max( (m_stepExp > std::numeric_limits<float>::epsilon()) ?
            (int)((float)(value - m_minExp) / m_stepExp + 0.5f) : 0, 0 );
    expHorizontalSlider->setRange( 0, sliderSteps );
    expHorizontalSlider->setSingleStep( 1 );
    expHorizontalSlider->setPageStep( 40 );
    expHorizontalSlider->setValue( sliderValue );

    // spin box
    expSpinBox->setRange( m_minExp, m_maxExp );
    expSpinBox->setSingleStep( m_stepExp );
    expSpinBox->setDecimals( 4 );
    expSpinBox->setValue( value );

    expHorizontalSlider->blockSignals( false );
    expSpinBox->blockSignals( false );

    expGroupBox->setEnabled( enabled );
// these children will be enabled/disabled automatically by Qt
//    expHorizontalSlider->setEnabled( enabled );
//    expSpinBox->setEnabled( enabled );
//    expMinText->setEnabled( enabled );
//    expMaxText->setEnabled( enabled );
    if (!enabled)
    {
        expMinText->setText( tr("min") );
        expMaxText->setText( tr("max") );
    }
}



/******************************************************************************
 * EC Block END
 *****************************************************************************/



/******************************************************************************
 * BLS Block START
 *****************************************************************************/
BlsTab::BlsTab(VirtualCamera *camDevice, QWidget *parent)
    : QWidget(parent),
      m_camDevice( camDevice )
{
    BlsBlockGroupBox = new QGroupBox( tr("Black Level Subtraction") );

    createBlsBlockGroupBox();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(BlsBlockGroupBox );
    setLayout(mainLayout);

    updateStatus();
}



void BlsTab::createBlsBlockGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsSubtractionGroupBox = new QGroupBox(tr("Black Level Subtraction"));
    BlsMeasuringGroupBox = new QGroupBox(tr("Black Level Measuring (not supported with sensor) "));

    createBlsSubtractionGroupBox();
    createBlsMeasuringGroupBox();
    layout->addWidget(BlsSubtractionGroupBox, 0, 0 );
    layout->addWidget(BlsMeasuringGroupBox,   1, 0 );

    layout->setRowStretch( 2, 1 );

    BlsBlockGroupBox->setLayout(layout);

    connect( parent(), SIGNAL(UpdateCamEngineStatus()), SLOT(updateStatus()) );
}



void BlsTab::createBlsSubtractionGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsValueRedLabel    = new QLabel(tr("Red"));
    BlsValueRedLabel->setFixedWidth( 150 );

    BlsValueGreenRLabel = new QLabel(tr("GreenR"));
    BlsValueGreenRLabel->setFixedWidth( 150 );

    BlsValueGreenBLabel = new QLabel(tr("GreenB"));
    BlsValueGreenBLabel->setFixedWidth( 150 );

    BlsValueBlueLabel   = new QLabel(tr("Blue"));
    BlsValueBlueLabel->setFixedWidth( 150 );

    BlsValueRedText     = new QSpinBox;
    BlsValueRedText->setFixedWidth( 150 );
    BlsValueRedText->setRange( 0, 8191 );

    BlsValueGreenRText  = new QSpinBox;
    BlsValueGreenRText->setFixedWidth( 150 );
    BlsValueGreenRText->setRange( 0, 8191 );

    BlsValueGreenBText  = new QSpinBox;
    BlsValueGreenBText->setFixedWidth( 150 );
    BlsValueGreenBText->setRange( 0, 8191 );

    BlsValueBlueText    = new QSpinBox;
    BlsValueBlueText->setFixedWidth( 150 );
    BlsValueBlueText->setRange( 0, 8191 );

    BlsRefreshButton = new QPushButton(tr("&Refresh"));
    BlsRefreshButton->setFixedWidth( 120 );

    BlsLoadFromFileButton = new QPushButton(tr("&Load from file"));
    BlsLoadFromFileButton->setFixedWidth( 120 );

    BlsSaveToFileButton = new QPushButton(tr("&Save to file"));
    BlsSaveToFileButton->setFixedWidth( 120 );

    layout->addWidget(BlsValueRedLabel      , 0, 0);
    layout->addWidget(BlsValueGreenRLabel   , 1, 0);
    layout->addWidget(BlsValueGreenBLabel   , 2, 0);
    layout->addWidget(BlsValueBlueLabel     , 3, 0);

    layout->addWidget(BlsValueRedText       , 0, 1);
    layout->addWidget(BlsValueGreenRText    , 1, 1);
    layout->addWidget(BlsValueGreenBText    , 2, 1);
    layout->addWidget(BlsValueBlueText      , 3, 1);

    layout->addWidget(BlsRefreshButton      , 0, 3);
    //layout->addWidget(BlsLoadFromFileButton , 2, 3);
    //layout->addWidget(BlsSaveToFileButton   , 3, 3);

    layout->setColumnStretch( 2, 1 );

    BlsSubtractionGroupBox->setLayout( layout );

    connect( BlsValueRedText   , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueGreenRText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueGreenBText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueBlueText  , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );

    connect( BlsRefreshButton,      SIGNAL( clicked() ), this, SLOT( updateStatus() ) );
    connect( BlsLoadFromFileButton, SIGNAL( clicked() ), this, SLOT( BlsLoadFromFileButtonClicked() ) );
    connect( BlsSaveToFileButton  , SIGNAL( clicked() ), this, SLOT( BlsSaveToFileButtonClicked() ) );
}



void BlsTab::BlsSetValues()
{
    uint16_t Red;
    uint16_t GreenR;
    uint16_t GreenB;
    uint16_t Blue;

    int value;

    /* check red value */
    value = BlsValueRedText->value();
    if ( value < 0 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for red pixel has to be > 0"));
        return;
    }
    else if ( value > 8191 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenR pixel has to be < 8191"));
        return;
    }
    Red = (uint16_t)value;

    /* check greenR value */
    value = BlsValueGreenRText->value();
    if ( value < 0 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenR pixel has to be > 0"));
        return;
    }
    else if ( value > 8191 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenR pixel has to be < 8191"));
        return;
    }
    GreenR = (uint16_t)value;

    /* check greenR value */
    value = BlsValueGreenBText->value();
    if ( value < 0 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenB pixel has to be > 0"));
        return;
    }
    else if ( value > 8191 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenR pixel has to be < 8191"));
        return;
    }
    GreenB = (uint16_t)value;

    /* check blue value */
    value = BlsValueBlueText->value();
    if ( value < 0 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for blue pixel has to be > 0"));
        return;
    }
    else if ( value > 8191 )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("black level for greenR pixel has to be < 8191"));
        return;
    }
    Blue = (uint16_t)value;

#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            m_camDevice->VideoDisplayBlsSet( Red, GreenR, GreenB, Blue );
        }
    }
#endif
}



void BlsTab::BlsLoadFromFileButtonClicked()
{
    printf("Bls Load From File Button Clicked\n");
}



void BlsTab::BlsSaveToFileButtonClicked()
{
    printf("Bls Save To File Button Clicked\n");
}



void BlsTab::createBlsMeasuringGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsWindowGroupBox = new QGroupBox;
    BlsResultsGroupBox = new QGroupBox(tr("Results"));
    BlsMeasuringButtonGroupBox = new QGroupBox;

    createBlsWindowGroupBox();
    createBlsResultsGroupBox();
    createBlsMeasuringButtonGroupBox();

    layout->addWidget(BlsWindowGroupBox,0,0);
    layout->addWidget(BlsResultsGroupBox,1,0);
    layout->addWidget(BlsMeasuringButtonGroupBox,2,0);

    BlsMeasuringGroupBox->setLayout(layout);
    BlsMeasuringGroupBox->setEnabled(false);
}



void BlsTab::createBlsWindowGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsWindow1GroupBox = new QGroupBox(tr("Window 1"));
    BlsWindow2GroupBox = new QGroupBox(tr("Window 2"));

    createBlsWindow1GroupBox();
    createBlsWindow2GroupBox();

    layout->addWidget(BlsWindow1GroupBox,0,0);
    layout->addWidget(BlsWindow2GroupBox,0,1);

    BlsWindowGroupBox->setLayout(layout);
}

void BlsTab::createBlsWindow1GroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsWindow1GroupBox->setCheckable(true);

    BlsWindow1XLabel = new QLabel(tr("x"));
    BlsWindow1YLabel = new QLabel(tr("y"));
    BlsWindow1widthLabel = new QLabel(tr("width"));
    BlsWindow1heightLabel = new QLabel(tr("height"));

    BlsWindow1XText = new QLineEdit;
    BlsWindow1YText = new QLineEdit;
    BlsWindow1WidthText = new QLineEdit;
    BlsWindow1HeightText = new QLineEdit;

    layout->addWidget(BlsWindow1XLabel,1,0);
    layout->addWidget(BlsWindow1YLabel,1,2);
    layout->addWidget(BlsWindow1widthLabel,2,0);
    layout->addWidget(BlsWindow1heightLabel,2,2);
    layout->addWidget(BlsWindow1XText,1,1);
    layout->addWidget(BlsWindow1YText,1,3);
    layout->addWidget(BlsWindow1WidthText,2,1);
    layout->addWidget(BlsWindow1HeightText,2,3);

    BlsWindow1GroupBox->setLayout(layout);
}

void BlsTab::createBlsWindow2GroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsWindow2GroupBox->setCheckable(true);

    BlsWindow2XLabel = new QLabel(tr("x"));
    BlsWindow2YLabel = new QLabel(tr("y"));
    BlsWindow2widthLabel = new QLabel(tr("width"));
    BlsWindow2heightLabel = new QLabel(tr("height"));

    BlsWindow2XText = new QLineEdit;
    BlsWindow2YText = new QLineEdit;
    BlsWindow2WidthText = new QLineEdit;
    BlsWindow2HeightText = new QLineEdit;

    layout->addWidget(BlsWindow2XLabel,1,0);
    layout->addWidget(BlsWindow2YLabel,1,2);
    layout->addWidget(BlsWindow2widthLabel,2,0);
    layout->addWidget(BlsWindow2heightLabel,2,2);
    layout->addWidget(BlsWindow2XText,1,1);
    layout->addWidget(BlsWindow2YText,1,3);
    layout->addWidget(BlsWindow2WidthText,2,1);
    layout->addWidget(BlsWindow2HeightText,2,3);

    BlsWindow2GroupBox->setLayout(layout);
}

void BlsTab::createBlsResultsGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsResultsValueALabel = new QLabel(tr("Black Level Value A"));
    BlsResultsValueBLabel = new QLabel(tr("Black Level Value B"));
    BlsResultsValueCLabel = new QLabel(tr("Black Level Value C"));
    BlsResultsValueDLabel = new QLabel(tr("Black Level Value D"));

    BlsResultsValueAText = new QLineEdit;
    BlsResultsValueBText = new QLineEdit;
    BlsResultsValueCText = new QLineEdit;
    BlsResultsValueDText = new QLineEdit;

    layout->addWidget(BlsResultsValueALabel,0,0);
    layout->addWidget(BlsResultsValueBLabel,0,2);
    layout->addWidget(BlsResultsValueCLabel,1,0);
    layout->addWidget(BlsResultsValueDLabel,1,2);
    layout->addWidget(BlsResultsValueAText,0,1);
    layout->addWidget(BlsResultsValueBText,0,3);
    layout->addWidget(BlsResultsValueCText,1,1);
    layout->addWidget(BlsResultsValueDText,1,3);

    BlsResultsGroupBox->setLayout(layout);
}

void BlsTab::createBlsMeasuringButtonGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    BlsMeasuringMeasureButton = new QPushButton(tr("measure"));
    BlsMeasuringTakeOverValuesButton = new QPushButton(tr("take over values"));

    layout->addWidget(BlsMeasuringMeasureButton,0,0);
    layout->addWidget(BlsMeasuringTakeOverValuesButton,0,1);

    BlsMeasuringButtonGroupBox->setLayout(layout);

    connect(BlsMeasuringMeasureButton, SIGNAL(clicked()), this, SLOT(BlsMeasuringMeasureButtonClicked()));
    connect(BlsMeasuringTakeOverValuesButton, SIGNAL(clicked()), this, SLOT(BlsMeasuringTakeOverValuesButtonClicked()));
}



void BlsTab::BlsMeasuringMeasureButtonClicked()
{
    printf("Bls Measuring Measure Button Clicked\n");
}



void BlsTab::BlsMeasuringTakeOverValuesButtonClicked()
{
    printf("Bls Measuring Take Over Values Button Clicked\n");
}


void BlsTab::updateStatus()
{
    uint16_t Red    = 0UL;
    uint16_t GreenR = 0UL;
    uint16_t GreenB = 0UL;
    uint16_t Blue   = 0UL;

#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            m_camDevice->VideoDisplayBlsGet( &Red, &GreenR, &GreenB, &Blue );
        }
    }
#endif

    disconnect( BlsValueRedText   , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    disconnect( BlsValueGreenRText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    disconnect( BlsValueGreenBText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    disconnect( BlsValueBlueText  , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );

    BlsValueRedText->setValue( Red );
    BlsValueGreenRText->setValue( GreenR );
    BlsValueGreenBText->setValue( GreenB );
    BlsValueBlueText->setValue( Blue );

    connect( BlsValueRedText   , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueGreenRText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueGreenBText, SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
    connect( BlsValueBlueText  , SIGNAL( valueChanged(int) ), this, SLOT( BlsSetValues() ) );
}

/******************************************************************************
 * BLS Block END
 *****************************************************************************/



/******************************************************************************
 * WB Block
 *****************************************************************************/
WbTab::WbTab
(
    VirtualCamera *camDevice, QWidget *parent
)
: QWidget( parent ),
m_camDevice( camDevice )
{
    WbBlockGroupBox = new QGroupBox( tr("White Balance") );

    createWbBlockGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( WbBlockGroupBox );
    setLayout( mainLayout );

    updateStatus();
}



void WbTab::createWbBlockGroupBox
(
)
{
    QGridLayout* layout = new QGridLayout;

    WBGainsGroupBox     = new QGroupBox( tr("White Balance Gains") );
    XTalkMatrixGroupBox = new QGroupBox( tr("Color Correction Matrix (X-Talk)") );

    createWBGainsGroupBox();
    createXTalkMatrixGroupBox();

    layout->addWidget( WBGainsGroupBox    , 0, 0 );
    layout->addWidget( XTalkMatrixGroupBox, 1, 0 );
    layout->setRowStretch( 2, 1 );

    WbBlockGroupBox->setLayout( layout );

    connect( parent(), SIGNAL( UpdateCamEngineStatus() ), SLOT( updateStatus() ) );
}



void WbTab::createWBGainsGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    WbRedLabel = new QLabel(tr("Red"));
    WbRedLabel->setFixedWidth( 150 );

    WbGreenRLabel = new QLabel(tr("Green R"));
    WbGreenRLabel->setFixedWidth( 150 );

    WbGreenBLabel = new QLabel(tr("Green B"));
    WbGreenBLabel->setFixedWidth( 150 );

    WbBlueLabel = new QLabel(tr("Blue"));
    WbBlueLabel->setFixedWidth( 150 );

    WbRedText = new QDoubleSpinBox;
    WbRedText->setFixedWidth( 150 );
    WbRedText->setDecimals( 3 );
    WbRedText->setSingleStep( 0.001f );
    WbRedText->setRange( 0.000f, 3.999f );

    WbGreenRText = new QDoubleSpinBox;
    WbGreenRText->setFixedWidth( 150 );
    WbGreenRText->setDecimals( 3 );
    WbGreenRText->setSingleStep( 0.001f );
    WbGreenRText->setRange( 0.000f, 3.999f );

    WbGreenBText = new QDoubleSpinBox;
    WbGreenBText->setFixedWidth( 150 );
    WbGreenBText->setDecimals( 3 );
    WbGreenBText->setSingleStep( 0.001f );
    WbGreenBText->setRange( 0.000f, 3.999f );

    WbBlueText = new QDoubleSpinBox;
    WbBlueText->setFixedWidth( 150 );
    WbBlueText->setDecimals( 3 );
    WbBlueText->setSingleStep( 0.001f );
    WbBlueText->setRange( 0.000f, 3.999f );

    WBRefreshButton = new QPushButton( tr("&Refresh") );
    WBRefreshButton->setFixedWidth( 120 );

    WBLoadFromFileButton = new QPushButton( tr("&Load from file"));
    WBLoadFromFileButton->setFixedWidth( 120 );

    WBSaveToFileButton = new QPushButton( tr("&Save to file"));
    WBSaveToFileButton->setFixedWidth( 120 );

    layout->addWidget( WbRedLabel,    0, 0 );
    layout->addWidget( WbGreenRLabel, 1, 0 );
    layout->addWidget( WbGreenBLabel, 2, 0 );
    layout->addWidget( WbBlueLabel,   3, 0 );

    layout->addWidget( WbRedText,    0, 1 );
    layout->addWidget( WbGreenRText, 1, 1 );
    layout->addWidget( WbGreenBText, 2, 1 );
    layout->addWidget( WbBlueText,   3, 1 );

    layout->setColumnStretch( 2, 1 );

    layout->addWidget( WBRefreshButton, 0, 3 );
//    layout->addWidget( WBLoadFromFileButton, 2, 3 );
//    layout->addWidget( WBSaveToFileButton,   3, 3 );

    WBGainsGroupBox->setLayout(layout);

    connect( WbRedText   , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbGreenRText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbGreenBText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbBlueText  , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );

    connect( WBRefreshButton,      SIGNAL( clicked() ), this, SLOT( updateStatus() ) );
    connect( WBLoadFromFileButton, SIGNAL( clicked() ), this, SLOT( WBLoadFromFileButtonClicked() ) );
    connect( WBSaveToFileButton,   SIGNAL( clicked() ), this, SLOT( WBSaveToFileButtonClicked() ) );
}



void WbTab::createXTalkMatrixGroupBox()
{
    QGridLayout* layout = new QGridLayout;

    XTalkRedLabel = new QLabel(tr("Red"));
    XTalkGreenLabel = new QLabel(tr("Green"));
    XTalkBlueLabel = new QLabel(tr("Blue"));

    XTalkRed1Text = new QDoubleSpinBox;
    XTalkRed1Text->setFixedWidth( 120 );
    XTalkRed1Text->setDecimals( 3 );
    XTalkRed1Text->setSingleStep( 0.004f );
    XTalkRed1Text->setRange( -8.000f, 7.996f );

    XTalkRed2Text = new QDoubleSpinBox;
    XTalkRed2Text->setFixedWidth( 120 );
    XTalkRed2Text->setDecimals( 3 );
    XTalkRed2Text->setSingleStep( 0.004f );
    XTalkRed2Text->setRange( -8.000f, 7.996f );

    XTalkRed3Text = new QDoubleSpinBox;
    XTalkRed3Text->setFixedWidth( 120 );
    XTalkRed3Text->setDecimals( 3 );
    XTalkRed3Text->setSingleStep( 0.004f );
    XTalkRed3Text->setRange( -8.000f, 7.996f );

    XTalkGreen1Text = new QDoubleSpinBox;
    XTalkGreen1Text->setFixedWidth( 120 );
    XTalkGreen1Text->setDecimals( 3 );
    XTalkGreen1Text->setSingleStep( 0.004f );
    XTalkGreen1Text->setRange( -8.000f, 7.996f );

    XTalkGreen2Text = new QDoubleSpinBox;
    XTalkGreen2Text->setFixedWidth( 120 );
    XTalkGreen2Text->setDecimals( 3 );
    XTalkGreen2Text->setSingleStep( 0.004f );
    XTalkGreen2Text->setRange( -8.000f, 7.996f );

    XTalkGreen3Text = new QDoubleSpinBox;
    XTalkGreen3Text->setFixedWidth( 120 );
    XTalkGreen3Text->setDecimals( 3 );
    XTalkGreen3Text->setSingleStep( 0.004f );
    XTalkGreen3Text->setRange( -8.000f, 7.996f );

    XTalkBlue1Text = new QDoubleSpinBox;
    XTalkBlue1Text->setFixedWidth( 120 );
    XTalkBlue1Text->setDecimals( 3 );
    XTalkBlue1Text->setSingleStep( 0.004f );
    XTalkBlue1Text->setRange( -8.000f, 7.996f );

    XTalkBlue2Text = new QDoubleSpinBox;
    XTalkBlue2Text->setFixedWidth( 120 );
    XTalkBlue2Text->setDecimals( 3 );
    XTalkBlue2Text->setSingleStep( 0.004f );
    XTalkBlue2Text->setRange( -8.000f, 7.996f );

    XTalkBlue3Text = new QDoubleSpinBox;
    XTalkBlue3Text->setFixedWidth( 120 );
    XTalkBlue3Text->setDecimals( 3 );
    XTalkBlue3Text->setSingleStep( 0.004f );
    XTalkBlue3Text->setRange( -8.000f, 7.996f );

    XTalkOffsetLabel = new QLabel(tr("Offset"));

    XTalkOffsetRText = new QSpinBox;
    XTalkOffsetRText->setFixedWidth( 120 );
    XTalkOffsetRText->setRange( -2048, 2047 );

    XTalkOffsetGText = new QSpinBox;
    XTalkOffsetGText->setFixedWidth( 120 );
    XTalkOffsetGText->setRange( -2048, 2047 );

    XTalkOffsetBText = new QSpinBox;
    XTalkOffsetBText->setFixedWidth( 120 );
    XTalkOffsetBText->setRange( -2048, 2047 );

    XTalkRedSpaceLabel = new QLabel;
    XTalkGreenSpaceLabel = new QLabel;
    XTalkBlueSpaceLabel = new QLabel;

    layout->addWidget( XTalkRedLabel,    1, 0 );
    layout->addWidget( XTalkGreenLabel,  2, 0 );
    layout->addWidget( XTalkBlueLabel,   3, 0 );

    layout->addWidget( XTalkRed1Text,    1, 1 );
    layout->addWidget( XTalkRed2Text,    1, 2 );
    layout->addWidget( XTalkRed3Text,    1, 3 );
    layout->addWidget( XTalkGreen1Text,  2, 1 );
    layout->addWidget( XTalkGreen2Text,  2, 2 );
    layout->addWidget( XTalkGreen3Text,  2, 3 );
    layout->addWidget( XTalkBlue1Text,   3, 1 );
    layout->addWidget( XTalkBlue2Text,   3, 2 );
    layout->addWidget( XTalkBlue3Text,   3, 3 );

    layout->addWidget( XTalkOffsetLabel, 0, 5 );
    layout->addWidget( XTalkOffsetRText, 1, 5 );
    layout->addWidget( XTalkOffsetGText, 2, 5 );
    layout->addWidget( XTalkOffsetBText, 3, 5 );

    layout->addWidget( XTalkRedSpaceLabel,   1, 4 );
    layout->addWidget( XTalkGreenSpaceLabel, 2, 4 );
    layout->addWidget( XTalkBlueSpaceLabel,  3, 4 );

    XTalkMatrixGroupBox->setLayout(layout);

    connect( XTalkRed1Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkRed2Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkRed3Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen1Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen2Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen3Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue1Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue2Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue3Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );

    connect( XTalkOffsetRText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    connect( XTalkOffsetGText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    connect( XTalkOffsetBText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
}



void WbTab::WBSetGains()
{
    double value;

    float Red;
    float GreenR;
    float GreenB;
    float Blue;

    /* check red value */
    value = WbRedText->value();
    if ( value < 0.000f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for red pixel has to be > 0"));
        return;
    }
    else if ( value > 3.999f )
    {

        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for red pixel has to be < 4.000"));
        return;
    }
    Red = (float)value;

    /* check greenR value */
    value = WbGreenRText->value();
    if ( value < 0.000f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for green pixel (in red-lines) has to be > 0"));
        return;
    }
    else if ( value > 3.999f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for green pixel (in red-lines) has to be < 4.000"));
        return;
    }
    GreenR = (float)value;

    /* check greenB value */
    value = WbGreenBText->value();
    if ( value < 0.000f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for green pixel (in blue-lines) has to be > 0"));
        return;
    }
    else if ( value > 3.999f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for greenR pixel (in blue-lines) has to be < 4.000"));
        return;
    }
    GreenB = (float)value;

     /* check blue value */
    value = WbBlueText->value();
    if ( value < 0.000f )
    {
        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for blue pixel has to be > 0"));

        return;
    }
    else if ( value > 3.999f )
    {

        QErrorMessage *ErrorDialog = new QErrorMessage( this );
        ErrorDialog->showMessage(tr("gain for blue pixel has to be < 8191"));

        return;
    }
    Blue = (float)value;
#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            m_camDevice->VideoDisplayGainSet( Red, GreenR, GreenB, Blue );
        }
    }
#endif
}



void WbTab::WBSetCcMatrix()
{
    double value;

    float Red1      = 1.0f;
    float Red2      = 0.0f;
    float Red3      = 0.0f;

    float Green1    = 0.0f;
    float Green2    = 1.0f;
    float Green3    = 0.0f;

    float Blue1     = 0.0f;
    float Blue2     = 0.0f;
    float Blue3     = 1.0f;

    Red1 = (float)XTalkRed1Text->value();
    Red2 = (float)XTalkRed2Text->value();
    Red3 = (float)XTalkRed3Text->value();

    Green1 = (float)XTalkGreen1Text->value();
    Green2 = (float)XTalkGreen2Text->value();
    Green3 = (float)XTalkGreen3Text->value();

    Blue1 = (float)XTalkBlue1Text->value();
    Blue2 = (float)XTalkBlue2Text->value();
    Blue3 = (float)XTalkBlue3Text->value();

#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            m_camDevice->VideoDisplayCcMatrixSet( Red1, Red2, Red3, Green1, Green2, Green3, Blue1, Blue2, Blue3 );
        }
    }
#endif
}



void WbTab::WBSetCcOffset()
{
    double value;

    int16_t Red   = 0.0f;
    int16_t Green = 0.0f;
    int16_t Blue  = 0.0f;

    Red     = (int16_t)XTalkOffsetRText->value();
    Green   = (int16_t)XTalkOffsetGText->value();
    Blue    = (int16_t)XTalkOffsetBText->value();

#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            m_camDevice->VideoDisplayCcOffsetSet( Red, Green, Blue );
        }
    }
#endif
}

void WbTab::WBLoadFromFileButtonClicked()
{
    printf("WB Load Button Clicked\n");
}



void WbTab::WBSaveToFileButtonClicked()
{
    printf("WB Save Button Clicked\n");
}



void WbTab::updateStatus()
{
    float Red       = 1.0f;
    float GreenR    = 1.0f;
    float GreenB    = 1.0f;
    float Blue      = 1.0f;

    float Red1      = 1.0f;
    float Red2      = 0.0f;
    float Red3      = 0.0f;

    float Green1    = 0.0f;
    float Green2    = 1.0f;
    float Green3    = 0.0f;

    float Blue1     = 0.0f;
    float Blue2     = 0.0f;
    float Blue3     = 1.0f;

    int16_t OffRed   = 0L;
    int16_t OffGreen = 0L;
    int16_t OffBlue  = 0L;

#if 0
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {

            m_camDevice->VideoDisplayGainGet( &Red, &GreenR, &GreenB, &Blue );
            m_camDevice->VideoDisplayCcMatrixGet
                ( &Red1, &Red2, &Red3, &Green1, &Green2, &Green3, &Blue1, &Blue2, &Blue3 );
            m_camDevice->VideoDisplayCcOffsetGet( &OffRed, &OffGreen, &OffBlue );

        }
    }
#endif
    disconnect( WbRedText   , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    disconnect( WbGreenRText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    disconnect( WbGreenBText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    disconnect( WbBlueText  , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );

    WbRedText->setValue( Red );
    WbGreenRText->setValue( GreenR );
    WbGreenBText->setValue( GreenB );
    WbBlueText->setValue( Blue );

    connect( WbRedText   , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbGreenRText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbGreenBText, SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );
    connect( WbBlueText  , SIGNAL( valueChanged(double) ), this, SLOT( WBSetGains() ) );

    disconnect( XTalkRed1Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkRed2Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkRed3Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkGreen1Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkGreen2Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkGreen3Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkBlue1Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkBlue2Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    disconnect( XTalkBlue3Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );

    XTalkRed1Text->setValue( Red1 );
    XTalkRed2Text->setValue( Red2 );
    XTalkRed3Text->setValue( Red3 );
    XTalkGreen1Text->setValue( Green1 );
    XTalkGreen2Text->setValue( Green2 );
    XTalkGreen3Text->setValue( Green3 );
    XTalkBlue1Text->setValue( Blue1 );
    XTalkBlue2Text->setValue( Blue2 );
    XTalkBlue3Text->setValue( Blue3 );

    connect( XTalkRed1Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkRed2Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkRed3Text,   SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen1Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen2Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkGreen3Text, SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue1Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue2Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );
    connect( XTalkBlue3Text,  SIGNAL( valueChanged(double) ), this, SLOT( WBSetCcMatrix() ) );

    disconnect( XTalkOffsetRText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    disconnect( XTalkOffsetGText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    disconnect( XTalkOffsetBText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );

    XTalkOffsetRText->setValue( OffRed );
    XTalkOffsetGText->setValue( OffGreen );
    XTalkOffsetBText->setValue( OffBlue );

    connect( XTalkOffsetRText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    connect( XTalkOffsetGText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
    connect( XTalkOffsetBText,  SIGNAL( valueChanged(int) ), this, SLOT( WBSetCcOffset() ) );
}




/***********************
 ***************************/

    BpTab::BpTab(QWidget *parent)
: QWidget(parent)
{

}

    DegammaTab::DegammaTab(QWidget *parent)
: QWidget(parent)
{

}

    ColCorTab::ColCorTab(QWidget *parent)
: QWidget(parent)
{

}



/***********************
 **Dpf Block
 **
 **************************/
DpfTab::DpfTab(VirtualCamera *camDevice, QWidget *parent)
    : QWidget(parent),
      m_camDevice( camDevice )
{
    createDpfBlockGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_dpfGroupBox );
    //mainlayout->addStretch(1);
    setLayout(mainLayout);

    updateStatus();
}

void DpfTab::createDpfBlockGroupBox()
{
    m_dpfGroupBox = new QGroupBox(tr("DPF (Denoising Prefilter)"));

    m_enableCheckBox = new QCheckBox(tr("DPF Enable"));

    // strength box

    m_strengthGroupBox = new QGroupBox(tr("Strength"));
    QVBoxLayout *strengthBoxLayout = new QVBoxLayout();

    QHBoxLayout *gradientBoxLayout = new QHBoxLayout();

    QLabel* gradientLabel    = new QLabel(tr("Gradient"));
    gradientLabel->setFixedWidth( 150 );

    m_gradientSpinBox = new QDoubleSpinBox();
    m_gradientSpinBox->setFixedWidth( 150 );
    m_gradientSpinBox->setRange( 0.0, 128.0 );
    m_gradientSpinBox->setSingleStep( 0.01 );
    m_gradientSpinBox->setDecimals( 2 );

    gradientBoxLayout->addWidget( gradientLabel );
    gradientBoxLayout->addWidget( m_gradientSpinBox );
    gradientBoxLayout->addStretch( 1 );

    QHBoxLayout *offsetBoxLayout = new QHBoxLayout();

    QLabel* offsetLabel    = new QLabel(tr("Offset"));
    offsetLabel->setFixedWidth( 150 );

    m_offsetSpinBox = new QDoubleSpinBox();
    m_offsetSpinBox->setFixedWidth( 150 );
    m_offsetSpinBox->setRange( -128.0, 128.0 );
    m_offsetSpinBox->setSingleStep( 0.01 );
    m_offsetSpinBox->setDecimals( 2 );

    offsetBoxLayout->addWidget( offsetLabel );
    offsetBoxLayout->addWidget( m_offsetSpinBox );
    offsetBoxLayout->addStretch( 1 );


    QHBoxLayout *minBoxLayout = new QHBoxLayout();

    QLabel* minLabel    = new QLabel(tr("Minimum Bound"));
    minLabel->setFixedWidth( 150 );

    m_minSpinBox = new QDoubleSpinBox();
    m_minSpinBox->setFixedWidth( 150 );
    m_minSpinBox->setRange( 0.0, 128.0 );
    m_minSpinBox->setSingleStep( 0.01 );
    m_minSpinBox->setDecimals( 2 );

    minBoxLayout->addWidget( minLabel );
    minBoxLayout->addWidget( m_minSpinBox );
    minBoxLayout->addStretch( 1 );

    QHBoxLayout *divBoxLayout = new QHBoxLayout();

    QLabel* divLabel    = new QLabel(tr("Division Factor"));
    divLabel->setFixedWidth( 150 );

    m_divSpinBox = new QDoubleSpinBox();
    m_divSpinBox->setFixedWidth( 150 );
    m_divSpinBox->setRange( 0.0, 64.0 );
    m_divSpinBox->setSingleStep( 0.01 );
    m_divSpinBox->setDecimals( 2 );

    divBoxLayout->addWidget( divLabel );
    divBoxLayout->addWidget( m_divSpinBox );
    divBoxLayout->addStretch( 1 );

    strengthBoxLayout->addLayout( gradientBoxLayout );
    strengthBoxLayout->addLayout( offsetBoxLayout );
    strengthBoxLayout->addLayout( minBoxLayout );
    strengthBoxLayout->addLayout( divBoxLayout );

    m_strengthGroupBox->setLayout( strengthBoxLayout );

    // weight box

    m_weightGroupBox = new QGroupBox(tr("Weight"));
    QVBoxLayout *weightBoxLayout = new QVBoxLayout();

    QHBoxLayout *greenBoxLayout = new QHBoxLayout();

    QLabel* greenLabel    = new QLabel(tr("Sigma Green"));
    greenLabel->setFixedWidth( 150 );

    m_greenSpinBox = new QSpinBox();
    m_greenSpinBox->setFixedWidth( 150 );
    m_greenSpinBox->setRange( 1, 255 );

    greenBoxLayout->addWidget( greenLabel );
    greenBoxLayout->addWidget( m_greenSpinBox );
    greenBoxLayout->addStretch( 1 );

    QHBoxLayout *redBlueBoxLayout = new QHBoxLayout();

    QLabel* redBlueLabel    = new QLabel(tr("Sigma Red/Blue"));
    redBlueLabel->setFixedWidth( 150 );

    m_redBlueSpinBox = new QSpinBox();
    m_redBlueSpinBox->setFixedWidth( 150 );
    m_redBlueSpinBox->setRange( 1, 255 );

    redBlueBoxLayout->addWidget( redBlueLabel );
    redBlueBoxLayout->addWidget( m_redBlueSpinBox );
    redBlueBoxLayout->addStretch( 1 );

    weightBoxLayout->addLayout( greenBoxLayout );
    weightBoxLayout->addLayout( redBlueBoxLayout );

    m_weightGroupBox->setLayout( weightBoxLayout );




    //DpfHandleGroupBox = new QGroupBox;
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(m_enableCheckBox);
    layout->addWidget( m_strengthGroupBox );
    layout->addWidget( m_weightGroupBox );
    layout->addStretch( 1 );

    m_dpfGroupBox->setLayout(layout);

    /*connect(m_enableCheckBox,SIGNAL(toggled(bool)),this,SLOT(DpfEnableApply()));*/
    connect(m_enableCheckBox, SIGNAL(clicked()), this, SLOT(DpfEnableApply()));

    connect(m_gradientSpinBox, SIGNAL( valueChanged( double )), this, SLOT(configure()));
    connect(m_offsetSpinBox, SIGNAL( valueChanged( double )), this, SLOT(configure()));
    connect(m_minSpinBox, SIGNAL( valueChanged( double )), this, SLOT(configure()));
    connect(m_divSpinBox, SIGNAL( valueChanged( double )), this, SLOT(configure()));
    connect(m_greenSpinBox, SIGNAL( valueChanged( int )), this, SLOT(configure()));
    connect(m_redBlueSpinBox, SIGNAL( valueChanged( int )), this, SLOT(configure()));

    connect(parent(),SIGNAL(UpdateCamEngineStatus()),SLOT(updateStatus()));
}

void DpfTab::DpfEnableApply()
{
    if(m_camDevice==NULL)
    {
        m_enableCheckBox->setChecked(false);
        dpfConfigErrorMessageDialog = new QErrorMessage();
        dpfConfigErrorMessageDialog->showMessage(tr("Please Load Driver and Start the Video play first"));
        return;
    }

    if(m_enableCheckBox->isChecked())
    {

        {
            int ret;
            Json::Value jRequest, jResponse;
            jRequest[DPF_ENABLE_PARAMS] = true;

            ret = m_camDevice->ioctl(ISPCORE_MODULE_DPF_ENABLE_SET, jRequest, jResponse);
            assert( 0 == ret );
            //m_camDevice->dpfEnableSet(true);
        }

        m_strengthGroupBox->setEnabled( false );
        m_weightGroupBox->setEnabled( false );
    }
    else
    {
        {
            int ret;
            Json::Value jRequest, jResponse;
            jRequest[DPF_ENABLE_PARAMS] = false;

            ret = m_camDevice->ioctl(ISPCORE_MODULE_DPF_ENABLE_SET, jRequest, jResponse);
            assert( 0 == ret );
            //m_camDevice->dpfEnableSet(false);
        }

        m_strengthGroupBox->setEnabled( true );
        m_weightGroupBox->setEnabled( true );
    }
}

void DpfTab::updateStatus()
{
    bool running = false;
    float gradient = -1.f;
    float offset   = -1.f;
    float min = -1.f;
    float div   = -1.f;
    uint8_t sigmaGreen    = 0;
    uint8_t sigmaRedBlue  = 0;
#if 0
    if ( true == m_camDevice->getAdpfStatus( running, gradient, offset, min, div, sigmaGreen, sigmaRedBlue ) )
    {
        m_enableCheckBox->setEnabled( true );
        m_strengthGroupBox->setEnabled( !running );
        m_weightGroupBox->setEnabled( !running );

        m_enableCheckBox->blockSignals( true );
        m_gradientSpinBox->blockSignals( true );
        m_offsetSpinBox->blockSignals( true );
        m_minSpinBox->blockSignals( true );
        m_divSpinBox->blockSignals( true );
        m_greenSpinBox->blockSignals( true );
        m_redBlueSpinBox->blockSignals( true );

        m_enableCheckBox->setChecked( running );

        m_gradientSpinBox->setValue( gradient );
        m_offsetSpinBox->setValue( offset );
        m_minSpinBox->setValue( min );
        m_divSpinBox->setValue( div );
        m_greenSpinBox->setValue( sigmaGreen );
        m_redBlueSpinBox->setValue( sigmaRedBlue );

        m_enableCheckBox->blockSignals( false );
        m_gradientSpinBox->blockSignals( false );
        m_offsetSpinBox->blockSignals( false );
        m_minSpinBox->blockSignals( false );
        m_divSpinBox->blockSignals( false );
        m_greenSpinBox->blockSignals( false );
        m_redBlueSpinBox->blockSignals( false );
    }
    else
    {
        m_enableCheckBox->setChecked( false );
        m_enableCheckBox->setEnabled( false );
        m_strengthGroupBox->setEnabled( false );
        m_weightGroupBox->setEnabled( false );
    }
#endif
}


void DpfTab::configure()
{
    float gradient = m_gradientSpinBox->value();
    float offset   = m_offsetSpinBox->value();
    float min = m_minSpinBox->value();
    float div = m_divSpinBox->value();
    uint8_t sigmaGreen   = m_greenSpinBox->value();
    uint8_t sigmaRedBlue = m_redBlueSpinBox->value();
#if 0
    m_camDevice->configureAdpf( gradient, offset, min, div, sigmaGreen, sigmaRedBlue );
#endif
}



/***********************
 **Dpcc Block
 **
 **************************/
DpccTab::DpccTab(VirtualCamera *camDevice, QWidget *parent)
    : QWidget(parent),
      m_camDevice( camDevice )
{
    DpccBlockGroupBox = new QGroupBox(tr("DPCC (Defect Pixel Cluster Correction)"));

    createDpccBlockGroupBox();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(DpccBlockGroupBox );
    //mainLayout->addStretch( 1 );
    setLayout(mainLayout);

    updateStatus();
}

void DpccTab::createDpccBlockGroupBox()
{
    DpccEnableCheckBox = new QCheckBox(tr("DPCC Enable"));
    //DpccHandleGroupBox = new QGroupBox;
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(DpccEnableCheckBox, 0, 0);
    //layout->addWidget(DpccHandleGroupBox, 1, 0);
    layout->setRowStretch( 1, 1 );
    layout->setColumnStretch( 1, 1 );

    DpccBlockGroupBox->setLayout(layout);
    /*connect(DpccEnableCheckBox, SIGNAL(clicked()), this,SLOT(DpccEnableApply())); */
    connect(DpccEnableCheckBox, SIGNAL(clicked()), this, SLOT(DpccEnableApply()));

    connect(parent(),SIGNAL(UpdateCamEngineStatus()),SLOT(updateStatus()));
}

void DpccTab::DpccEnableApply()
{
    if(m_camDevice==NULL)
    {
        DpccEnableCheckBox->setChecked(false);
        dpccConfigErrorMessageDialog = new QErrorMessage();
        dpccConfigErrorMessageDialog->showMessage(tr("Please Load Driver and Start the Video play first"));
        return;
    }

    if(DpccEnableCheckBox->isChecked())
    {
        //m_camDevice->dpccEnableSet(1);
    }
    else
    {
        //m_camDevice->dpccEnableSet(0);
    }
}


void DpccTab::updateStatus()
{
    bool running = false;
#if 0
    if ( true == m_camDevice->dpccEnableGet( running ) )
    {
        DpccEnableCheckBox->setEnabled( true );
        DpccEnableCheckBox->setChecked( running );
    }
    else
    {
        DpccEnableCheckBox->setChecked( false );
        DpccEnableCheckBox->setEnabled( false );
    }
#endif

    /*
    if ( m_camDevice != NULL )
    {
        if ( CamEngineItf::State::Invalid != m_camDevice->state() )
        {
            CamEngineHandle_t hCamEngine = m_camDevice->camDeviceHandle();
            if ( NULL != hCamEngine )
            {
                bool_t Running = BOOL_FALSE;
                RESULT result = CamEngineAdpccStatus ( hCamEngine, &Running );
                if ( result == RET_SUCCESS )
                {
                    success = true;
                    if ( BOOL_TRUE == Running )
                    {
                        DpccEnableCheckBox->setEnabled( true );
                        DpccEnableCheckBox->setChecked( true );
                    }
                    else
                    {
                        DpccEnableCheckBox->setEnabled( true );
                        DpccEnableCheckBox->setChecked( false );
                    }
                }
            }
        }
    }

    if ( true != success )
    {
        DpccEnableCheckBox->setChecked( false );
        DpccEnableCheckBox->setEnabled( false );
    }
    */
}


/***********************
 **Lsc Block
 **
 **************************/
    LscTab::LscTab(QWidget *parent)
: QWidget(parent)
{

}




/***********************
 **Cac Block
 **
 **************************/
CacTab::CacTab(VirtualCamera *camDevice, QWidget *parent)
    : QWidget(parent),
      m_camDevice( camDevice )
{
    CacBlockGroupBox = new QGroupBox(tr("Chromatic Aberration Correction"));

    createCacBlockGroupBox();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(CacBlockGroupBox );
    setLayout(mainLayout);
}

void CacTab::createCacBlockGroupBox()
{
    CacEnableCheckBox = new QCheckBox(tr("CAC_Enable"));
    CacHandleGroupBox = new QGroupBox;
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(CacEnableCheckBox, 0, 0);
    layout->addWidget(CacHandleGroupBox, 1, 0);

    createCacHandleGroupBox();

    CacBlockGroupBox->setLayout(layout);

    connect(CacEnableCheckBox, SIGNAL(clicked()), this, SLOT(CacEnableApply()));
}

void CacTab::createCacHandleGroupBox()
{

    SubCacHandleGroupBox = new QGroupBox;
    SubCacCurveGroupBox = new QGroupBox;
    QGridLayout* layout = new QGridLayout;

    layout->addWidget(SubCacHandleGroupBox , 0, 0);
    layout->addWidget(SubCacCurveGroupBox , 1, 0);

    CacHandleGroupBox->setLayout(layout);

    createSubCacHandleGroupBox();
    createSubCacCurveGroupBox();
}

void CacTab::createSubCacHandleGroupBox()
{
    AllBox = new QVBoxLayout();

    hbox1 = new QHBoxLayout();
    hbox2 = new QHBoxLayout();

    Para1Box = new QGroupBox();
    Para1Layout = new QGridLayout();
    RedParaLabel = new QLabel("red parameters");
    BlueParaLabel = new QLabel("blue parameters");
    LinearCoeffLabel = new QLabel("linear coeff");
    SquareCoeffLabel = new QLabel("square coeff");
    CubicalCoeffLabel = new QLabel("cubical coeff");

    redLinearCoeffEdit = new QLineEdit();
    redSquareCoeffEdit = new QLineEdit();
    redCubicalCoeffEdit = new QLineEdit();
    blueLinearCoeffEdit = new QLineEdit();
    blueSquareCoeffEdit = new QLineEdit();
    blueCubicalCoeffEdit = new QLineEdit();

    Para1Layout->addWidget(LinearCoeffLabel ,0,1);
    Para1Layout->addWidget(SquareCoeffLabel ,0,2);
    Para1Layout->addWidget(CubicalCoeffLabel ,0,3);

    Para1Layout->addWidget(RedParaLabel ,1,0);
    Para1Layout->addWidget(redLinearCoeffEdit ,1,1);
    Para1Layout->addWidget(redSquareCoeffEdit ,1,2);
    Para1Layout->addWidget(redCubicalCoeffEdit ,1,3);

    Para1Layout->addWidget(BlueParaLabel ,2,0);
    Para1Layout->addWidget(blueLinearCoeffEdit ,2,1);
    Para1Layout->addWidget(blueSquareCoeffEdit ,2,2);
    Para1Layout->addWidget(blueCubicalCoeffEdit ,2,3);

    Para1Box->setLayout(Para1Layout);


    Para2Box = new QGroupBox();
    Para2Layout = new QGridLayout();
    ratioParaLabel = new QLabel("ratio k_xy");
    centerHoriOffsetParaLabel = new QLabel("center horizontal offset");
    centerVertOffsetParaLabel = new QLabel("center vertical offset");
    pixelLabel = new QLabel("pixel");
    linesLabel = new QLabel("lines");

    ratioParaEdit = new QLineEdit();
    centerHoriOffsetEdit = new QLineEdit();
    centerVertOffsetEdit = new QLineEdit();

    Para2Layout->addWidget(ratioParaLabel ,0,0 );
    Para2Layout->addWidget(ratioParaEdit ,0,1 );
    Para2Layout->addWidget(centerHoriOffsetParaLabel ,1,0 );
    Para2Layout->addWidget(centerHoriOffsetEdit , 1,1);
    Para2Layout->addWidget(pixelLabel , 1,2 );
    Para2Layout->addWidget(centerVertOffsetParaLabel ,2,0 );
    Para2Layout->addWidget(centerVertOffsetEdit ,2,1 );
    Para2Layout->addWidget(linesLabel ,2,2 );

    Para2Box->setLayout(Para2Layout);

    hbox1->addWidget(Para1Box);
    hbox1->addWidget(Para2Box);


    Para3Box = new QGroupBox();
    Para3Layout = new QGridLayout();
    horiLimitParaLabel = new QLabel("horizontal limit(0..64)");
    vertLimitParaLabel = new QLabel("vertical limit  (0..64)");
    horiLimitParaEndLabel = new QLabel("/16 = number of max horizontal pixel displacement");
    vertLimitParaEndLabel = new QLabel("/16 = number of max vertical lines displacement");
    plotButton = new QPushButton("plot");

    horiLimitParaEdit = new QLineEdit();
    vertLimitParaEdit = new QLineEdit();

    Para3Layout->addWidget(horiLimitParaLabel , 0,0 );
    Para3Layout->addWidget(horiLimitParaEdit , 0,1);
    Para3Layout->addWidget(horiLimitParaEndLabel , 0,2);
    Para3Layout->addWidget(vertLimitParaLabel , 1,0);
    Para3Layout->addWidget(vertLimitParaEdit , 1,1);
    Para3Layout->addWidget(vertLimitParaEndLabel , 1,2);
    Para3Layout->addWidget(plotButton ,2,2);

    Para3Box->setLayout(Para3Layout);

    hbox2->addWidget(Para3Box);
    AllBox->addLayout(hbox1);
    AllBox->addLayout(hbox2);
    SubCacHandleGroupBox->setLayout( AllBox );
    resize( 800, 600 );

}

void CacTab::createSubCacCurveGroupBox()
{
    plot = new CacPlot( this );

    QHBoxLayout *Layout = new QHBoxLayout;

    QGroupBox *box = new QGroupBox;

    box->setLayout( Layout );

    Layout->addWidget( plot );

    SubCacCurveGroupBox->setLayout( Layout );

}

void CacTab::CacEnableApply()
{
    if(m_camDevice==NULL)
    {
        CacEnableCheckBox->setChecked(false);
        CacConfigErrorMessageDialog = new QErrorMessage();
        CacConfigErrorMessageDialog->showMessage(tr("Please Load Driver and Start the Video play first"));
        return;
    }

    if(CacEnableCheckBox->isChecked())
    {
        //m_camDevice->cacEnableSet(true);
    }
    else
    {
        //m_camDevice->cacEnableSet(false);
    }
}


    DemosaicTab::DemosaicTab(QWidget *parent)
: QWidget(parent)
{

}

    FitterTab::FitterTab(QWidget *parent)
: QWidget(parent)
{

}

    GammaTab::GammaTab(QWidget *parent)
: QWidget(parent)
{

}

    CsmTab::CsmTab(QWidget *parent)
: QWidget(parent)
{

}

#if 0
    Level2Tab::Level2Tab( QWidget *parent )
: QTabWidget(parent)
{
    QTabWidget *tabWidget = new QTabWidget;

    tabWidget->addTab( new BLS( this ), tr("BLS") );
    tabWidget->addTab( new BP( this ), tr("BP") );
    tabWidget->addTab( new Degamma( this ), tr("Degamma") );
    tabWidget->addTab( new ColCor( this ), tr("ColCor") );
    tabWidget->addTab( new WDR( this ), tr("WDR") );

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    createMainSelectionModuleGroupBox();
    createMainWindowDisplayGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( MainSelectionModuleGroupBox );
    mainLayout->addWidget(MainWindowDisplayGroupBox);

    setLayout( mainLayout );
}

void Level2Tab::createMainSelectionModuleGroupBox()
{
    MainSelectionModuleGroupBox = new QGroupBox( tr("Block Selection") );
    QGridLayout *layout = new QGridLayout;

    MFBlsButton           = new QPushButton(tr("01 BLS"));
    MFBpButton        = new QPushButton(tr("02 BP"));
    MFDegammaButton      = new QPushButton(tr("03 Degamma"));
    MFColCorButton         = new QPushButton(tr("04 ColCor"));
    MFWdrButton            = new QPushButton(tr("05 WDR"));
    MFDpfButton            = new QPushButton(tr("06 DPF"));
    MFDpccButton           = new QPushButton(tr("07 DPCC"));
    MFLSCButton            = new QPushButton(tr("08 LSC"));
    MFCacButton            = new QPushButton(tr("09 CAC"));
    MFDemosaicButton       = new QPushButton(tr("10 Demosaic"));
    MFFitterButton         = new QPushButton(tr("11 Fitter"));
    MFGammaButton          = new QPushButton(tr("12 Gamma"));
    MFCsmButton            = new QPushButton(tr("13 CSM"));

    layout->addWidget(MFBlsButton, 0, 0);
    layout->addWidget(MFBpButton, 0, 1);
    layout->addWidget(MFDegammaButton, 0, 2);
    layout->addWidget(MFColCorButton, 0, 3);
    layout->addWidget(MFWdrButton, 0, 4);
    layout->addWidget(MFDpfButton, 0, 5);
    layout->addWidget(MFDpccButton, 0, 6);
    layout->addWidget(MFLSCButton, 1, 0);
    layout->addWidget(MFCacButton, 1, 1);
    layout->addWidget(MFDemosaicButton, 1, 2);
    layout->addWidget(MFFitterButton, 1, 3);
    layout->addWidget(MFGammaButton, 1, 4);
    layout->addWidget(MFCsmButton, 1, 5);

    MainSelectionModuleGroupBox->setLayout(layout);
    MainSelectionModuleGroupBox->setMaximumHeight(100);


    connect(MFBlsButton, SIGNAL(clicked()), this, SLOT(MFBlsButtonClicked()));
    connect(MFBpButton, SIGNAL(clicked()), this, SLOT(MFBpButtonClicked()));
    connect(MFDegammaButton, SIGNAL(clicked()), this, SLOT(MFDegammaButtonClicked()));
    connect(MFColCorButton, SIGNAL(clicked()), this, SLOT(MFColCorButtonClicked()));
    connect(MFWdrButton, SIGNAL(clicked()), this, SLOT(MFWdrButtonClicked()));
    connect(MFDpfButton, SIGNAL(clicked()), this, SLOT(MFDpfButtonClicked()));
    connect(MFDpccButton, SIGNAL(clicked()), this, SLOT(MFDpccButtonClicked()));
    connect(MFLSCButton, SIGNAL(clicked()), this, SLOT(MFLSCButtonClicked()));
    connect(MFCacButton, SIGNAL(clicked()), this, SLOT(MFCacButtonClicked()));
    connect(MFDemosaicButton, SIGNAL(clicked()), this, SLOT(MFDemosaicButtonClicked()));
    connect(MFFitterButton, SIGNAL(clicked()), this, SLOT(MFFitterButtonClicked()));
    connect(MFGammaButton, SIGNAL(clicked()), this, SLOT(MFGammaButtonClicked()));
    connect(MFCsmButton, SIGNAL(clicked()), this, SLOT(MFCsmButtonClicked()));
}

void Level2Tab::releaseButtonFlat()
{
    MFBlsButton->setFlat(false);
    MFBpButton->setFlat(false);
    MFDegammaButton->setFlat(false);
    MFColCorButton->setFlat(false);
    MFWdrButton->setFlat(false);
    MFDpfButton->setFlat(false);
    MFDpccButton->setFlat(false);
    MFLSCButton->setFlat(false);
    MFCacButton->setFlat(false);
    MFDemosaicButton->setFlat(false);
    MFFitterButton->setFlat(false);
    MFGammaButton->setFlat(false);
    MFCsmButton->setFlat(false);
}

void Level2Tab::createMainWindowDisplayGroupBox()
{
    MainWindowDisplayGroupBox = new QGroupBox;

    CurveDisplayGroupBox = new QGroupBox;
    CurveHandleButtonGroupBox = new QGroupBox;

    QGridLayout* layout = new QGridLayout;

    layout->addWidget(CurveDisplayGroupBox, 0,0 );
    layout->addWidget(CurveHandleButtonGroupBox, 1, 0);
    CurveHandleButtonGroupBox->setMaximumSize(800,100);
    MainWindowDisplayGroupBox->setLayout(layout);
}

void Level2Tab::createMFBlsBlockGroupBox()
{
    MFBlsBlockGroupBox = new QGroupBox(stringModuleName);

    QGridLayout* layout = new QGridLayout;

    MFBlsBlockGroupBox->setLayout(layout);

}

void Level2Tab::MFBlsButtonClicked()
{
    releaseButtonFlat();
    MFBlsButton->setFlat(true);

    printf("MFBlsButton is clicked. \n");
    registerSelectedNo = 0;
    stringModuleName = "Black Level Subtraction";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    /*    MainWindowDisplayGroupBox->setTitle(stringModuleName);*/
    createMFBlsBlockGroupBox();
}

void Level2Tab::MFBpButtonClicked()
{
    releaseButtonFlat();
    MFBpButton->setFlat(true);
    stringModuleName = "Bad Pixel Detection/Correction(still-image-based)";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFDegammaButtonClicked()
{
    releaseButtonFlat();
    MFDegammaButton->setFlat(true);
    stringModuleName = "Sensor de-gamma";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFColCorButtonClicked()
{
    releaseButtonFlat();
    MFColCorButton->setFlat(true);
    stringModuleName = "Color Correction";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFWdrButtonClicked()
{
    releaseButtonFlat();
    MFWdrButton->setFlat(true);

    printf("MFWdrButton is clicked. \n");
    registerSelectedNo = 1;
    stringModuleName = "Wide Dynamic Range(WDR)";
    xmlStringModuleName = "CAMERIC_MODULE_WDR";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFDpfButtonClicked()
{
    releaseButtonFlat();
    MFDpfButton->setFlat(true);

    printf("MFDpfButton is clicked. \n");
    registerSelectedNo = 2;
    stringModuleName = "Advanced Fittering";
    xmlStringModuleName = "CAMERIC_MODULE_DPF";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);
}

void Level2Tab::MFDpccButtonClicked()
{
    releaseButtonFlat();
    MFDpccButton->setFlat(true);
    printf("MFDpccButton is clicked. \n");
    registerSelectedNo = 3;
    stringModuleName = "DPCC Fitter (Defect Pixel Cluster Correction)";
    xmlStringModuleName = "CAMERIC_MODULE_DPCC";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFLSCButtonClicked()
{
    releaseButtonFlat();
    MFLSCButton->setFlat(true);
    stringModuleName = "Lens Shading Correction (LSC)";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFCacButtonClicked()
{
    releaseButtonFlat();
    MFCacButton->setFlat(true);
    stringModuleName = "Chromatic Aberration Correction";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFDemosaicButtonClicked()
{
    releaseButtonFlat();
    MFDemosaicButton->setFlat(true);
    stringModuleName = "Demosaic";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFFitterButtonClicked()
{
    releaseButtonFlat();
    MFFitterButton->setFlat(true);
    stringModuleName = "Fittering";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFGammaButtonClicked()
{
    releaseButtonFlat();
    MFGammaButton->setFlat(true);
    stringModuleName = "Gamma Correction ";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}

void Level2Tab::MFCsmButtonClicked()
{
    releaseButtonFlat();
    MFCsmButton->setFlat(true);
    stringModuleName = "Color Space Matrix,422 Conversion Mode";
    xmlStringModuleName = "CAMERIC_MODULE_BLS";
    MainWindowDisplayGroupBox->setTitle(stringModuleName);

}




#endif
