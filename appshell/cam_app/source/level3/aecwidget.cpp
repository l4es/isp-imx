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

#include "level3/aecwidget.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>

#include "level3/exposureplot.h"

#include "Histogram/HistPlot.h"
#include "Scene/scene.h"
#include <string.h>


AecTab::AecTab( VirtualCamera *camDevice, QWidget *parent )
    : QWidget    ( parent ),
      m_camDevice( camDevice ),
      m_interval ( 1 ),
      m_counter  ( 0 )
{
    createAecGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_aecGroupBox );
    setLayout(mainLayout);
    updateStatus();
}

void AecTab::createAecGroupBox() {
    m_aecGroupBox = new QGroupBox( tr("Auto Exposure") );
    QVBoxLayout *AllLayout = new QVBoxLayout();

    // Scene Evaluation Mode
    m_modeGroupBox = new QGroupBox(tr("Scene Evaluation Mode"));
    QHBoxLayout *modeLayout = new QHBoxLayout();

    m_modeComboBox = new QComboBox();
    m_modeComboBox->addItem( tr("Disabled (Setpoint)"), QVariant::fromValue<uint32_t>( CAM_ENGINE_AEC_SCENE_EVALUATION_DISABLED ) ); // index 1, AEC_SCENE_EVALUATION_DISABLED
    m_modeComboBox->addItem( tr("Fix (Static ROI)"), QVariant::fromValue<uint32_t>( CAM_ENGINE_AEC_SCENE_EVALUATION_FIX ) );    // index 2, AEC_SCENE_EVALUATION_FIX
    m_modeComboBox->addItem( tr("Adaptive (Adaptive ROI)"), QVariant::fromValue<uint32_t>( CAM_ENGINE_AEC_SCENE_EVALUATION_ADAPTIVE ) );    // index 3, AEC_SCENE_EVALUATION_ADAPTIVE
    modeLayout->addWidget( m_modeComboBox );

    m_modeGroupBox->setLayout( modeLayout );
    // Setpoint
    m_setPointGroupBox = new QGroupBox(tr("Setpoint"));
    QFormLayout *setPointBoxLayout = new QFormLayout();

    m_setPointSpinBox = new QDoubleSpinBox();
    m_setPointSpinBox->setMinimumSize(QSize(120, 22));
    m_setPointSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_setPointSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_setPointSpinBox->setDecimals( 1 );
    m_setPointSpinBox->setRange(0.0f, 255.0f);
    m_setPointSpinBox->setSingleStep( 1.0f );

    m_setPointSlider = new QSlider();
    m_setPointSlider->setOrientation(Qt::Horizontal);
    m_setPointSlider->setTracking( false );
    m_setPointSlider->setRange( 0, 255 );

    QGridLayout *setPointLayout = new QGridLayout();
    setPointLayout->addWidget( m_setPointSlider, 0, 0, 1, 2 );
    setPointLayout->addWidget( m_setPointSpinBox, 0, 2, 1, 1 );

    m_clmToleranceSpinBox = new QDoubleSpinBox();
    m_clmToleranceSpinBox->setMinimumSize(QSize(120, 22));
    m_clmToleranceSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_clmToleranceSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_clmToleranceSpinBox->setDecimals( 1 );
    m_clmToleranceSpinBox->setRange(0.0f, 100.0f);
    m_clmToleranceSpinBox->setSingleStep( 1.0f );

    m_clmToleranceSlider = new QSlider();
    m_clmToleranceSlider->setOrientation(Qt::Horizontal);
    m_clmToleranceSlider->setTracking( false );
    m_clmToleranceSlider->setRange( 0, 100 );

    QGridLayout *clmToleranceLayout = new QGridLayout();
    clmToleranceLayout->addWidget( m_clmToleranceSlider, 0, 0, 1, 2 );
    clmToleranceLayout->addWidget( m_clmToleranceSpinBox, 0, 2, 1, 1 );

    QLabel *setPointLabel = new QLabel(tr("Setpoint"));
    setPointLabel->setFixedWidth(125);

    QHBoxLayout *AutoIlluminationLayout = new QHBoxLayout();

    QLabel *clmToleranceLabel = new QLabel(tr("Tolerance"));
    clmToleranceLabel->setFixedWidth(125);

    setPointBoxLayout->addRow( tr("Setpoint"), setPointLayout );
    setPointBoxLayout->addRow( tr("Tolerance"), clmToleranceLayout );

    m_setPointGroupBox->setLayout( setPointBoxLayout );

    connect( m_setPointSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setSetPoint( double ) ) );
    connect( m_setPointSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeSetPoint( int ) ) );
    connect( m_clmToleranceSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setClmTolerance( double ) ) );
    connect( m_clmToleranceSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeClmTolerance( int ) ) );

    // Damping
    m_dampingGroupBox = new QGroupBox(tr("Damping"));
    QFormLayout *dampingBoxLayout = new QFormLayout();

    m_dampOverSpinBox = new QDoubleSpinBox();
    m_dampOverSpinBox->setMinimumSize(QSize(120, 22));
    m_dampOverSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_dampOverSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_dampOverSpinBox->setDecimals( 2 );
    m_dampOverSpinBox->setRange(0.0f, 1.0f);
    m_dampOverSpinBox->setSingleStep( 0.01f );

    m_dampOverSlider = new QSlider();
    m_dampOverSlider->setOrientation(Qt::Horizontal);
    m_dampOverSlider->setTracking( false );
    m_dampOverSlider->setRange( 0, 100 );

    QGridLayout *dampOverLayout = new QGridLayout();
    dampOverLayout->addWidget( m_dampOverSlider, 0, 0, 1, 2 );
    dampOverLayout->addWidget( m_dampOverSpinBox, 0, 2, 1, 1 );

    m_dampUnderSpinBox = new QDoubleSpinBox();
    m_dampUnderSpinBox->setMinimumSize(QSize(120, 22));
    m_dampUnderSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_dampUnderSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    m_dampUnderSpinBox->setDecimals( 2 );
    m_dampUnderSpinBox->setRange(0.0f, 1.0f);
    m_dampUnderSpinBox->setSingleStep( 0.01f );

    m_dampUnderSlider = new QSlider();
    m_dampUnderSlider->setOrientation(Qt::Horizontal);
    m_dampUnderSlider->setTracking( false );
    m_dampUnderSlider->setRange( 0, 100 );

    QGridLayout *dampUnderLayout = new QGridLayout();
    dampUnderLayout->addWidget( m_dampUnderSlider, 0, 0, 1, 2 );
    dampUnderLayout->addWidget( m_dampUnderSpinBox, 0, 2, 1, 1 );

    dampingBoxLayout->addRow( tr("Over"), dampOverLayout );
    dampingBoxLayout->addRow( tr("Under"), dampUnderLayout );

    m_dampingGroupBox->setLayout( dampingBoxLayout );

    connect( m_dampOverSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setDampOver( double ) ) );
    connect( m_dampOverSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeDampOver( int ) ) );
    connect( m_dampUnderSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( setDampUnder( double ) ) );
    connect( m_dampUnderSlider,    SIGNAL( valueChanged( int    ) ), this, SLOT( changeDampUnder( int ) ) );

    // Config Box
    m_configGroupBox = new QGroupBox();

    QVBoxLayout *configLayout = new QVBoxLayout();

    configLayout->addWidget(m_modeGroupBox);
    configLayout->addWidget(m_setPointGroupBox);
    configLayout->addWidget(m_dampingGroupBox);

    m_configGroupBox->setLayout(configLayout);

    // Button Box
    QGroupBox *buttonGroupBox = new QGroupBox();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    startButton = new QPushButton("Start");
    stopButton  = new QPushButton("Stop");
    resetButton = new QPushButton("Reset");
    startButton->setFixedWidth(100);
    stopButton->setFixedWidth(100);
    resetButton->setFixedWidth(100);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch( 1 );

    buttonLayout->setSpacing(10);
    buttonGroupBox->setLayout(buttonLayout);

    // Status Label
    QGroupBox *LabelAecBox = new QGroupBox();
    QLabel *AecStatusLabel = new QLabel("Status :");
    AecStatusLabel->setFixedSize(75,25);
    AecStatusChangeLabel = new QLabel("N/A");
    AecStatusChangeLabel->setFixedSize(75,25);

    QHBoxLayout *LabelAecLayout = new QHBoxLayout();
    LabelAecLayout->addWidget( AecStatusLabel );
    LabelAecLayout->addWidget( AecStatusChangeLabel );
    LabelAecLayout->addStretch(1);

    LabelAecBox->setLayout( LabelAecLayout );

    // Measurement Box
    QGroupBox *measureGroupBox = new QGroupBox(tr("Measurements"));

    m_measureTab = new QTabWidget( this );

    m_histogram = new HistPlot( this );
    m_scene= new Scene( this );
    m_exposure = new ExposurePlot( this );

    m_measureTab->addTab( m_scene, tr( "Scene Luminance" ) );
    m_measureTab->addTab( m_histogram, tr( "Histogram" ) );
    m_measureTab->addTab( m_exposure, tr( "Exposure" ) );

    QSpinBox *intervalSpinBox = new QSpinBox( this );
    intervalSpinBox->setMinimumSize(QSize(120, 22));
    intervalSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    intervalSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    intervalSpinBox->setRange ( 0, 20 );
    intervalSpinBox->setSingleStep ( 1 );
    intervalSpinBox->setValue( m_interval );

    QLabel *intervalLabel = new QLabel( tr("Sample Interval") );
    intervalSpinBox->setMinimumSize(QSize(120, 22));
    intervalSpinBox->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );

    QHBoxLayout* intervalLayout = new QHBoxLayout();
    intervalLayout->addStretch( 1 );
    intervalLayout->addWidget( intervalLabel );
    intervalLayout->addWidget( intervalSpinBox );

    QVBoxLayout* measureLayout = new QVBoxLayout();
    measureLayout->addWidget( m_measureTab );
    measureLayout->addLayout( intervalLayout );
    measureGroupBox->setLayout( measureLayout );

    //Actions
    connect(startButton,SIGNAL(clicked()),SLOT(StartBtn()));
    connect(stopButton,SIGNAL(clicked()),SLOT(StopBtn()));
    connect(resetButton,SIGNAL(clicked()),SLOT(ResetBtn()));

    connect( m_setPointSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( configure() ) );
    connect( m_clmToleranceSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( configure() ) );
    connect( m_dampOverSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( configure() ) );
    connect( m_dampUnderSpinBox,   SIGNAL( valueChanged( double ) ), this, SLOT( configure() ) );

    connect( m_setPointSlider,   SIGNAL( valueChanged( int ) ), this, SLOT( configure() ) );
    connect( m_clmToleranceSlider,   SIGNAL( valueChanged( int ) ), this, SLOT( configure() ) );
    connect( m_dampOverSlider,   SIGNAL( valueChanged( int ) ), this, SLOT( configure() ) );
    connect( m_dampUnderSlider,   SIGNAL( valueChanged( int ) ), this, SLOT( configure() ) );

    connect(m_modeComboBox,SIGNAL(currentIndexChanged(int)),SLOT(changeMode(int)));
    connect(intervalSpinBox, SIGNAL(valueChanged( int )), SLOT(changeInterval( int )) );

    AllLayout->addWidget( LabelAecBox );
    AllLayout->addWidget( m_configGroupBox );
    AllLayout->addWidget( measureGroupBox );
    AllLayout->addWidget( buttonGroupBox );
    AllLayout->addStretch( 1 ); 

    m_aecGroupBox->setLayout( AllLayout );
    resize( 800, 600 );
}

void AecTab::StopBtn() {
    if ( m_camDevice == NULL ) {
        return;
    }

    AecStatusChangeLabel->setText("Stopped");
    m_configGroupBox->setEnabled( true  );

    if ( 0 == m_modeComboBox->currentIndex() ) {
        // manual
        //ManualBox->setEnabled( true  );
        //AutoBox  ->setEnabled( false );
    } else {
        // auto
        //ManualBox->setEnabled( false );
        //AutoBox  ->setEnabled( true  );
    }

    startButton->setEnabled( true  );
    stopButton ->setEnabled( false );
    resetButton->setEnabled( true  );
    m_camDevice->post<bool>(ISPCORE_MODULE_AE_ENABLE_SET, AE_ENABLE_PARAMS, false);
}



void AecTab::ResetBtn() {
    if ( m_camDevice == NULL ) {
        return;
    }
    m_camDevice->post<int>(ISPCORE_MODULE_AE_RESET, 0, 0);
    updateStatus();
}

void AecTab::StartBtn() {
    if ( m_camDevice == NULL ) {
        return;
    }
    int ret = m_camDevice->post<bool>(ISPCORE_MODULE_AE_ENABLE_SET, AE_ENABLE_PARAMS, true);
    if ( ret == 0 ) {
        AecStatusChangeLabel->setText("Running");
        m_configGroupBox->setEnabled( false );

        startButton->setEnabled( false );
        stopButton ->setEnabled( true  );
        resetButton->setEnabled( false );
    }
}

void AecTab::changeInterval( int interval ) {
    if ( m_interval != interval )
    {
        m_interval = interval;

        if ( 0 != m_interval )
        {
            m_measureTab->setEnabled( true );
        }
        else
        {
            m_measureTab->setEnabled( false );
        }
    }
}

void AecTab::changeMode(int index) {
    configure();
}

void AecTab::updateStatus() {
    bool running = false;
    CamEngineAecSemMode_t mode = CAM_ENGINE_AEC_SCENE_EVALUATION_INVALID;
    float setPoint      = -1.f;
    float clmTolerance  = -1.f;
    float dampOver      = -1.f;
    float dampUnder     = -1.f;

    Json::Value jRequest, jResponse;
    int ret = m_camDevice->ioctl(ISPCORE_MODULE_AE_CFG_GET, jRequest, jResponse);

    if ( ret == 0 ) {
        mode = (CamEngineAecSemMode_t)jResponse[AE_MODE_PARAMS].asInt();
        setPoint = jResponse[AE_SET_POINT_PARAMS].asFloat();
        clmTolerance= jResponse[AE_CLM_TOLERANCE_PARAMS].asFloat();
        dampOver = jResponse[AE_DAMP_OVER_PARAMS].asFloat();
        dampUnder = jResponse[AE_DAMP_UNDER_PARAMS].asFloat();
        int idx = m_modeComboBox->findData( QVariant::fromValue<uint32_t>( mode ) );
        if ( -1 != idx ) {
            m_modeComboBox->blockSignals( true );
            m_modeComboBox->setCurrentIndex( idx );
            m_modeComboBox->blockSignals( false );
        }

        setSetPoint( setPoint );
        setClmTolerance( clmTolerance );
        setDampOver( dampOver );
        setDampUnder( dampUnder );

        AecStatusChangeLabel->setText( ( true == running ) ? "Running" : "Stopped" );
        m_configGroupBox    ->setEnabled( !running );

        startButton->setEnabled( !running );
        stopButton ->setEnabled( running  );
        resetButton->setEnabled( !running );
    } else {
       AecStatusChangeLabel->setText("N/A");
       m_configGroupBox    ->setEnabled( false );

       startButton->setEnabled( false );
       stopButton ->setEnabled( false );
       resetButton->setEnabled( false );
    }
}

void AecTab::buffer(isp_metadata *meta) {
    if ( 0 == m_interval ) {
        return;
    }

    if ( 0 == ( m_counter % m_interval ) ) {

        if ( true == m_histogram->isVisible()) {
            m_histogram->populate( CAM_ENGINE_AEC_HIST_NUM_BINS, meta->hist.mean );
        }

        if (true == m_scene->isVisible()) {
            if ( CAM_ENGINE_AEC_SCENE_EVALUATION_ADAPTIVE !=
                    (CamEngineAecSemMode_t)m_modeComboBox->itemData( m_modeComboBox->currentIndex() ).toUInt() ) {
                m_scene->setValues( meta->exp.mean );
            } else {
                m_scene->setValues( meta->exp.mean, meta->exp.sem );
            }
        }

        if ( true == m_exposure->isVisible()) {
            float gain = 0.0f;
            float itime = 0.0f;
            m_exposure->append( gain, itime * 1000.0f );
        }
    }

    ++m_counter;
}


void AecTab::setSetPoint( double value )
{
    m_setPointSlider->blockSignals( true );
    m_setPointSpinBox->blockSignals( true );

    int sliderValue = (int)((value - m_setPointSpinBox->minimum()) / m_setPointSpinBox->singleStep() + 0.5f);
    m_setPointSlider->setValue( sliderValue );
    m_setPointSpinBox->setValue( value );

    m_setPointSlider->blockSignals( false );
    m_setPointSpinBox->blockSignals( false );
}


void AecTab::changeSetPoint( int value )
{
    double newSetPoint = m_setPointSpinBox->minimum() + (double)value * m_setPointSpinBox->singleStep();
    setSetPoint( newSetPoint );
}


void AecTab::setClmTolerance( double value )
{
    m_clmToleranceSlider->blockSignals( true );
    m_clmToleranceSpinBox->blockSignals( true );

    int sliderValue = (int)((value - m_clmToleranceSpinBox->minimum()) / m_clmToleranceSpinBox->singleStep() + 0.5f);
    m_clmToleranceSlider->setValue( sliderValue );
    m_clmToleranceSpinBox->setValue( value );

    m_clmToleranceSlider->blockSignals( false );
    m_clmToleranceSpinBox->blockSignals( false );
}


void AecTab::changeClmTolerance( int value )
{
    double newClmTolerance = m_clmToleranceSpinBox->minimum() + (double)value * m_clmToleranceSpinBox->singleStep();
    setClmTolerance( newClmTolerance );
}


void AecTab::setDampOver( double value )
{
    m_dampOverSlider->blockSignals( true );
    m_dampOverSpinBox->blockSignals( true );

    int sliderValue = (int)((value - m_dampOverSpinBox->minimum()) / m_dampOverSpinBox->singleStep() + 0.5f);
    m_dampOverSlider->setValue( sliderValue );
    m_dampOverSpinBox->setValue( value );

    m_dampOverSlider->blockSignals( false );
    m_dampOverSpinBox->blockSignals( false );
}


void AecTab::changeDampOver( int value )
{
    double newDampOver = m_dampOverSpinBox->minimum() + (double)value * m_dampOverSpinBox->singleStep();
    setDampOver( newDampOver );
}


void AecTab::setDampUnder( double value )
{
    m_dampUnderSlider->blockSignals( true );
    m_dampUnderSpinBox->blockSignals( true );

    int sliderValue = (int)((value - m_dampUnderSpinBox->minimum()) / m_dampUnderSpinBox->singleStep() + 0.5f);
    m_dampUnderSlider->setValue( sliderValue );
    m_dampUnderSpinBox->setValue( value );

    m_dampUnderSlider->blockSignals( false );
    m_dampUnderSpinBox->blockSignals( false );
}


void AecTab::changeDampUnder( int value )
{
    double newDampUnder = m_dampUnderSpinBox->minimum() + (double)value * m_dampUnderSpinBox->singleStep();
    setDampUnder( newDampUnder );
}


void AecTab::configure() {
    Json::Value jRequest, jResponse;
    jRequest[AE_MODE_PARAMS] = m_modeComboBox->itemData( m_modeComboBox->currentIndex() ).toUInt();
    jRequest[AE_SET_POINT_PARAMS] = m_setPointSpinBox->value();
    jRequest[AE_CLM_TOLERANCE_PARAMS] = m_clmToleranceSpinBox->value();
    jRequest[AE_DAMP_OVER_PARAMS ]= m_dampOverSpinBox->value();
    jRequest[AE_DAMP_UNDER_PARAMS] = m_dampUnderSpinBox->value();

    m_camDevice->ioctl(ISPCORE_MODULE_AE_CFG_SET, jRequest, jResponse);
}
