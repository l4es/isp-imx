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

#include "level3/awbwidget.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGridLayout>


AwbTab::AwbTab( VirtualCamera *camDevice, QWidget *parent )
    : QWidget    ( parent ),
      m_camDevice( camDevice )
{
    createAwbGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( m_awbGroupBox );
    setLayout(mainLayout);

    updateStatus();
}


void AwbTab::createAwbGroupBox()
{
    m_awbGroupBox = new QGroupBox( tr("Auto White Balance") );

    QVBoxLayout *AllLayout = new QVBoxLayout();

    // White Balance Mode
    ModeBox = new QGroupBox(tr("White Balance Mode"));
    QHBoxLayout *ModeLayout = new QHBoxLayout();

    ModeCombo = new QComboBox();
    ModeCombo->addItem( "Manual", QVariant::fromValue<uint32_t>( CAM_ENGINE_AWB_MODE_MANUAL) ); // index 0
    ModeCombo->addItem( "Auto", QVariant::fromValue<uint32_t>( CAM_ENGINE_AWB_MODE_AUTO ) );    // index 1
    ModeLayout->addWidget( ModeCombo );

    ModeBox->setLayout( ModeLayout );

    // Manual Box
    ManualBox = new QGroupBox(tr("Manual"));
    QVBoxLayout *ManualLayout = new QVBoxLayout();

    QHBoxLayout *ManualIlluminationLayout = new QHBoxLayout();

    QLabel *ManualIlluminationLabel = new QLabel(tr("Illumination Profile"));
    ManualIlluminationLabel->setFixedWidth(125);

    ManualIlluminationCombo = new QComboBox();

    ManualIlluminationLayout->addWidget(ManualIlluminationLabel, 0);
    ManualIlluminationLayout->addWidget(ManualIlluminationCombo, 1);

    ManualLayout->addLayout(ManualIlluminationLayout ); 

    ManualBox->setLayout(ManualLayout);

    // Auto Box
    AutoBox = new QGroupBox(tr("Auto"));
    QVBoxLayout *AutoLayout = new QVBoxLayout();

    QHBoxLayout *AutoIlluminationLayout = new QHBoxLayout();

    QLabel *AutoIlluminationLabel = new QLabel(tr("Start Profile"));
    AutoIlluminationLabel->setFixedWidth(125);

    AutoIlluminationCombo = new QComboBox();

    AutoIlluminationLayout->addWidget(AutoIlluminationLabel, 0);
    AutoIlluminationLayout->addWidget(AutoIlluminationCombo, 1);
 
    // Rg Projection
    m_RGBox = new QGroupBox( tr("R/G - Projections") );
    QGridLayout *RGLayout = new QGridLayout();

    QLabel *m_RGMaxSkyLabel     = new QLabel( "R/G Max Sky Projection" );
    QLabel *m_RGIndoorMinLabel  = new QLabel( "R/G Indoor Min Projection" );
    QLabel *m_RGOutdoorMinLabel = new QLabel( "R/G Outdoor Min Projection" );
    QLabel *m_RGMaxLabel        = new QLabel( "R/G Max Projection" );

    m_RGMaxSkyEdit = new QLineEdit();
    m_RGMaxSkyEdit->setInputMask("0.000");
    m_RGMaxSkyEdit->setMaxLength( 5 );
    m_RGMaxSkyEdit->setEnabled( false );

    m_RGIndoorMinEdit = new QLineEdit();
    m_RGIndoorMinEdit->setInputMask("0.000");
    m_RGIndoorMinEdit->setMaxLength( 5 );
    m_RGIndoorMinEdit->setEnabled( false );
    
    m_RGOutdoorMinEdit = new QLineEdit();
    m_RGOutdoorMinEdit->setInputMask("0.000");
    m_RGOutdoorMinEdit->setMaxLength( 5 );
    m_RGOutdoorMinEdit->setEnabled( false );

    m_RGMaxEdit = new QLineEdit();
    m_RGMaxEdit->setInputMask("0.000");
    m_RGMaxEdit->setMaxLength( 5 );
    m_RGMaxEdit->setEnabled( false );

    RGLayout->addWidget( m_RGMaxSkyLabel    , 1, 1 );
    RGLayout->addWidget( m_RGMaxSkyEdit     , 1, 2 );
    RGLayout->addWidget( m_RGIndoorMinLabel , 2, 1 );
    RGLayout->addWidget( m_RGIndoorMinEdit  , 2, 2 );
    RGLayout->addWidget( m_RGOutdoorMinLabel, 3, 1 );
    RGLayout->addWidget( m_RGOutdoorMinEdit , 3, 2 );
    RGLayout->addWidget( m_RGMaxLabel       , 4, 1 );
    RGLayout->addWidget( m_RGMaxEdit        , 4, 2 );

    m_RGBox->setLayout( RGLayout );

    // Damping
    m_DampingBox = new QGroupBox( tr("Damping") );
    QGridLayout *DampingLayout = new QGridLayout();

    m_enableDampingCheckBox = new QCheckBox( tr("Enable") );
    m_enableDampingCheckBox->setChecked( true );

#if 0
    QLabel *IIRFiltersizeLabel      = new QLabel( "IIR - Filtersize" );
    m_IIRFiltersizeEdit             = new QSpinBox();
    m_IIRFiltersizeEdit->setRange( 2 , 50 );

    QLabel *IIRFilterInitValueLabel = new QLabel( "Filter Init Value" );
    m_IIRFilterInitValueEdit        = new QLineEdit();
#endif    

    DampingLayout->addWidget( m_enableDampingCheckBox   , 1, 1 );
#if 0
    DampingLayout->addWidget( IIRFiltersizeLabel        , 2, 1 );
    DampingLayout->addWidget( m_IIRFiltersizeEdit       , 2, 2 );
    DampingLayout->addWidget( IIRFilterInitValueLabel   , 3, 1 );
    DampingLayout->addWidget( m_IIRFilterInitValueEdit  , 3, 2 );
#endif    

    m_DampingBox->setLayout( DampingLayout );


    AutoLayout->addLayout( AutoIlluminationLayout ); 
    AutoLayout->addSpacing( 10 );    
    AutoLayout->addWidget( m_RGBox );
    AutoLayout->addWidget( m_DampingBox );

    AutoBox->setLayout(AutoLayout);

    // Button Box
    QGroupBox *ButtonBox = new QGroupBox();

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
    ButtonBox->setLayout(buttonLayout);

    // Status Label
    QGroupBox *LabelAwbBox = new QGroupBox();
    QLabel *AwbStatusLabel = new QLabel("Status :");
    AwbStatusLabel->setFixedSize(75,25);
    AwbStatusChangeLabel = new QLabel("N/A");
    AwbStatusChangeLabel->setFixedSize(75,25);

    QHBoxLayout *LabelAwbLayout = new QHBoxLayout();
    LabelAwbLayout->addWidget( AwbStatusLabel );
    LabelAwbLayout->addWidget( AwbStatusChangeLabel );
    LabelAwbLayout->addStretch(1);

    LabelAwbBox->setLayout( LabelAwbLayout );

    //Actions
    connect(startButton,SIGNAL(clicked()),SLOT(StartBtn()));
    connect(stopButton,SIGNAL(clicked()),SLOT(StopBtn()));
    connect(resetButton,SIGNAL(clicked()),SLOT(ResetBtn()));

    connect(ModeCombo,SIGNAL(currentIndexChanged(int)),SLOT(changeMode(int)));
    connect(parent(),SIGNAL(UpdateCamEngineStatus()),SLOT(updateStatus()));

    AllLayout->addWidget( LabelAwbBox );
    AllLayout->addWidget( ModeBox );    
    AllLayout->addWidget( ManualBox );
    AllLayout->addWidget( AutoBox );
    AllLayout->addWidget( ButtonBox );
    AllLayout->addStretch( 1 ); 

    m_awbGroupBox->setLayout( AllLayout );
    resize( 800, 600 );
}



void AwbTab::StopBtn()
{
    if ( m_camDevice == NULL )
    {
        return;
    }

    AwbStatusChangeLabel->setText("Stopped");
    ModeBox->setEnabled( true  );

    if( 0 == ModeCombo->currentIndex() )
    {
        // manual
        ManualBox->setEnabled( true  );
        AutoBox  ->setEnabled( false );
    }
    else  
    {
        // auto
        ManualBox->setEnabled( false );
        AutoBox  ->setEnabled( true  );
    }

    startButton->setEnabled( true  );
    stopButton ->setEnabled( false );
    resetButton->setEnabled( true  );

#if 0
    m_camDevice->stopAwb();
#endif
}



void AwbTab::ResetBtn()
{
    if ( m_camDevice == NULL )
    {
        return;
    }

#if 0
    m_camDevice->resetAwb();
#endif
}



void AwbTab::StartBtn()
{
    if ( m_camDevice == NULL )
    {
        return;
    }

    CamEngineAwbMode_t Mode = CAM_ENGINE_AWB_MODE_INVALID;
    uint32_t CieProfile     = 0UL;

    Mode = (CamEngineAwbMode_t)ModeCombo->itemData( ModeCombo->currentIndex() ).toUInt();
    if ( Mode == CAM_ENGINE_AWB_MODE_MANUAL )
    {
        CieProfile = (uint32_t)ManualIlluminationCombo->itemData ( ManualIlluminationCombo->currentIndex() ).toUInt();
    }
    else
    {
        CieProfile = (uint32_t) AutoIlluminationCombo->itemData ( AutoIlluminationCombo->currentIndex() ).toUInt();
    }

    bool_t damping = ( true == m_enableDampingCheckBox->isChecked() ) ? BOOL_TRUE : BOOL_FALSE;
#if 0
    if ( true == m_camDevice->startAwb( Mode, CieProfile, damping ) )
	{
		AwbStatusChangeLabel->setText("Running");
		ModeBox    ->setEnabled( false );
		ManualBox  ->setEnabled( false );
		AutoBox    ->setEnabled( false );

		startButton->setEnabled( false );
		stopButton ->setEnabled( true  );
		resetButton->setEnabled( false );
	}
#endif
}



void AwbTab::changeMode(int index)
{
    if ( 0 == index )
    {
        // manual
        ManualBox->setEnabled( true  );
        AutoBox  ->setEnabled( false );
    }
    else
    {
        // auto
        ManualBox->setEnabled( false );
        AutoBox  ->setEnabled( true  );
    }
}



void AwbTab::updateStatus()
{
#if 0
    //get illumination profiles
    ManualIlluminationCombo->clear( );
    AutoIlluminationCombo  ->clear( );


    std::vector<CamIlluProfile_t *> profiles;
    if ( true == m_camDevice->getIlluminationProfiles( profiles ) )
    {
        for ( std::vector<CamIlluProfile_t *>::iterator iter = profiles.begin(); iter != profiles.end(); ++iter )
        {
            ManualIlluminationCombo->addItem( QString( (*iter)->name ), (uint32_t)( std::distance( profiles.begin(), iter ) ) );
            AutoIlluminationCombo->addItem( QString( (*iter)->name ), (uint32_t)( std::distance( profiles.begin(), iter ) ) );
        }
    }

    bool running = false;
    CamEngineAwbMode_t mode = CAM_ENGINE_AWB_MODE_INVALID;
    uint32_t cieProfile = 0;
    CamEngineAwbRgProj_t RgConfig;
    bool damping = false;
    if ( true == m_camDevice->getAwbStatus( running, mode, cieProfile, RgConfig, damping ) )
    {
        if ( CAM_ENGINE_AWB_MODE_MANUAL == mode )
        {
            ModeCombo->setCurrentIndex(0);//Manual
            
            int idx = ManualIlluminationCombo->findData( QVariant::fromValue<uint32_t>( cieProfile ) );
            if ( -1 != idx )
            {
                ManualIlluminationCombo->setCurrentIndex( idx );
            }
        }
        else
        {
            ModeCombo->setCurrentIndex(1);//Auto

            int idx = AutoIlluminationCombo->findData( QVariant::fromValue<uint32_t>( cieProfile ) );
            if ( -1 != idx )
            {
                AutoIlluminationCombo->setCurrentIndex( idx );
            }
        }
    
        m_RGIndoorMinEdit->setText( QString().setNum( RgConfig.fRgProjIndoorMin, 'f', 3 ) );
        m_RGOutdoorMinEdit->setText( QString().setNum( RgConfig.fRgProjOutdoorMin, 'f', 3 ) );
        m_RGMaxEdit->setText( QString().setNum( RgConfig.fRgProjMax, 'f', 3 ) );
        m_RGMaxSkyEdit->setText( QString().setNum( RgConfig.fRgProjMaxSky, 'f', 3 ) );

        AwbStatusChangeLabel->setText( ( true == running ) ? "Running" : "Stopped" );
        ModeBox    ->setEnabled( !running );
        ManualBox  ->setEnabled( !running );
        AutoBox    ->setEnabled( !running );

        startButton->setEnabled( !running );
        stopButton ->setEnabled( running  );
        resetButton->setEnabled( !running );
    
        m_enableDampingCheckBox->setChecked( damping );
    }
    else
    {
       AwbStatusChangeLabel->setText("N/A");
       ModeBox    ->setEnabled( false );
       ManualBox  ->setEnabled( false );
       AutoBox    ->setEnabled( false );

       startButton->setEnabled( false );
       stopButton ->setEnabled( false );
       resetButton->setEnabled( false );
    }
#endif
}


