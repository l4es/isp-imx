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

#include "level3tab.h"

#include <QtGui>
#include <QTabWidget>
#include <QLabel>
#include <QButtonGroup>
#include <QGridLayout>
#include <QLineEdit>
#include <QPalette>
#include <QComboBox>
/*#include "AecWidget.h"    */
//#include "Oscilloscope/plot.h"
//#include "Scene/scene.h"
//#include "Matrix/matrix.h"
//#include "Histogram/HistPlot.h"

#include "level3/aecwidget.h"
#include "level3/awbwidget.h"

#define ADI_DEMO_0117 1
#define AWB_MAX_ILLUMINATION_PROFILES   32L //FIXME

static VirtualCamera *camDevice;

Level3Tab::Level3Tab( VirtualCamera *camDevice, QWidget *parent )
	: QTabWidget(parent),
	  m_camDevice( camDevice )
{
	//FIXME
	camDevice = m_camDevice;

    QTabWidget *tabWidget = new QTabWidget;

    AecTab *aecTab = new AecTab( m_camDevice, this );
    AwbTab *awbTab = new AwbTab( m_camDevice, this );

    tabWidget->addTab( aecTab, tr("AE") );
    tabWidget->addTab( new AfTab( this ), tr("AF") );
    tabWidget->addTab( awbTab, tr("AWB") );

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    connect( parent, SIGNAL( opened() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( closed() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( connected() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( disconnected() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( started() ), this, SIGNAL( UpdateCamEngineStatus() ) );
    connect( parent, SIGNAL( stopped() ), this, SIGNAL( UpdateCamEngineStatus() ) );

    connect( parent, SIGNAL( opened() ), aecTab, SLOT( updateStatus() ) );
    connect( parent, SIGNAL( closed() ), aecTab, SLOT( updateStatus() ) );
    connect( parent, SIGNAL( connected() ), aecTab, SLOT( updateStatus() ) );
    connect( parent, SIGNAL( disconnected() ), aecTab, SLOT( updateStatus() ) );
    connect( parent, SIGNAL( started() ), aecTab, SLOT( updateStatus() ) );
    connect( parent, SIGNAL( stopped() ), aecTab, SLOT( updateStatus() ) );

    connect( parent, SIGNAL( buffer( isp_metadata* ) ), aecTab, SLOT( buffer(isp_metadata*) ) );
}

    AfTab::AfTab(QWidget *parent)
: QWidget(parent)
{
    AfBlockGroupBox = new QGroupBox(tr("AF"));

    createAfBlockGroupBox();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(AfBlockGroupBox );
    setLayout(mainLayout);

    updateStatus();
}

void AfTab::createAfBlockGroupBox()
{
    QVBoxLayout *AllLayout = new QVBoxLayout();

    //Search Algorithm Box
    SearchAlgorithmBox = new QGroupBox();
    QHBoxLayout *ComboLayout = new QHBoxLayout();
    QLabel *SearchAlgorithmLabel = new QLabel("Search Algorithm");
    SearchAlgorithmLabel->setFixedWidth(125);

    SearchAlgorithmCombo = new QComboBox();
    SearchAlgorithmCombo->addItem( "Full-Search"            , QVariant::fromValue<uint32_t>( CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_FULL_RANGE ));
    SearchAlgorithmCombo->addItem( "Adaptive Range-Search"  , QVariant::fromValue<uint32_t>( CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE ) );
    SearchAlgorithmCombo->addItem( "Hill-Climbing"          , QVariant::fromValue<uint32_t>( CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_HILL_CLIMBING) );

    ComboLayout->addWidget(SearchAlgorithmLabel, 0 );
    ComboLayout->addWidget(SearchAlgorithmCombo, 1 );

    SearchAlgorithmBox->setLayout(ComboLayout);

    //Button Box
    QGroupBox *ButtonBox = new QGroupBox();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    startButton = new QPushButton("Start Tracking");
    stopButton  = new QPushButton("Stop Tracking");
    oneShotButton = new QPushButton("One Shot");
    startButton->setFixedWidth(100);
    stopButton->setFixedWidth(100);
    oneShotButton->setFixedWidth(100);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addStretch( 1 );
    buttonLayout->addWidget(oneShotButton);

    buttonLayout->setSpacing(10);
    ButtonBox->setLayout(buttonLayout);

    //Status Label
    QGroupBox *LabelAfBox = new QGroupBox();
    QLabel *AfStatusLabel = new QLabel("Status :");
    AfStatusLabel->setFixedSize(75,25);
    AfStatusChangeLabel = new QLabel("N/A");
    AfStatusChangeLabel->setFixedSize(75,25);


    QHBoxLayout *LabelAfLayout = new QHBoxLayout();
    LabelAfLayout->addWidget(AfStatusLabel);
    LabelAfLayout->addWidget(AfStatusChangeLabel);
    LabelAfLayout->addStretch(1);

    LabelAfBox->setLayout(LabelAfLayout);

    //Actions
    connect(startButton,SIGNAL(clicked()),SLOT(StartBtn()));
    connect(stopButton,SIGNAL(clicked()),SLOT(StopBtn()));
    connect(oneShotButton,SIGNAL(clicked()),SLOT(OneShotBtn()));

    connect(parent(),SIGNAL(UpdateCamEngineStatus()),SLOT(updateStatus()));

/*
    QGroupBox *ButtonBox = new QGroupBox();
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *startOneButton = new QPushButton("Start One - Shot");
    QPushButton *startContinouslyButton = new QPushButton("Start Continously");
    QPushButton *stopContinouslyButton = new QPushButton("Stop Continously");

    buttonLayout->addWidget(startOneButton);
    buttonLayout->addWidget(startContinouslyButton);
    buttonLayout->addWidget(stopContinouslyButton);

    ButtonBox->setLayout(buttonLayout);
*/
    AllLayout->addWidget(LabelAfBox);
    AllLayout->addWidget(SearchAlgorithmBox);
    AllLayout->addWidget(ButtonBox);
    AllLayout->addStretch( 1 );
    //AllBox->addWidget(VirtualAfBox);
    AfBlockGroupBox->setLayout( AllLayout );
    resize( 800, 600 );
}

void AfTab::StopBtn()
{
    if(camDevice == NULL)
    {
        QErrorMessage *AfStopConfigErrorMessageDialog = new QErrorMessage();
        AfStopConfigErrorMessageDialog->showMessage(tr("Please Load Driver and Start the Video play first"));

        return;
    }
    else
    {
       	//if ( true == camDevice->afEnableSet(false) )
        if(true)
        {
            AfStatusChangeLabel->setText("Stopped");
            SearchAlgorithmBox->setEnabled( true );

            startButton->setEnabled( true );
            stopButton ->setEnabled( false  );
            oneShotButton->setEnabled( true );
        }
       	/*
        CamEngineHandle_t hCamEngine = camDevice->camDeviceHandle();
        if ( NULL != hCamEngine )
        {
            RESULT result = CamEngineAfStop( hCamEngine );
            if ( result == RET_SUCCESS )
            {
                AfStatusChangeLabel->setText("Stopped");
                SearchAlgorithmBox    ->setEnabled( true );

                startButton->setEnabled( true );
                stopButton ->setEnabled( false  );
                oneShotButton->setEnabled( true );
            }
        }
        */
    }
}

void AfTab::OneShotBtn()
{
    if(camDevice == NULL)
    {
        QErrorMessage* AfResetConfigErrorMessageDialog = new QErrorMessage();
        AfResetConfigErrorMessageDialog->showMessage(tr("Please Load Driver and Start the Video play first"));

        return;
    }
    else
    {
    	//camDevice->afEnableSet( (CamEngineAfSearchAlgorithm_t)SearchAlgorithmCombo->itemData( SearchAlgorithmCombo->currentIndex() ).toUInt() );

    	/*
        CamEngineHandle_t hCamEngine = camDevice->camDeviceHandle();
        if ( NULL != hCamEngine )
        {
            CamEngineAfOneShot( hCamEngine, CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_ADAPTIVE_RANGE );//FIXME!!!
        }*/
    }
}

void AfTab::StartBtn()
{
    if(camDevice == NULL)
    {
        QErrorMessage* AfStartConfigErrorMessageDialog = new QErrorMessage();
        AfStartConfigErrorMessageDialog->showMessage(tr("Please load driver and start video play first !"));

        return;
    }
    else
    {
//    	if ( true == camDevice->afEnableSet( 
//    	                    (CamEngineAfSearchAlgorithm_t)SearchAlgorithmCombo->itemData( SearchAlgorithmCombo->currentIndex() ).toUInt() ) )
        if(0)
    	{
             AfStatusChangeLabel->setText("Tracking ...");
             SearchAlgorithmBox->setEnabled( false );

             startButton->setEnabled( false );
             stopButton->setEnabled( true  );
             oneShotButton->setEnabled( false );
    	}
    }
}

void AfTab::updateStatus()
{
    bool available = false;
    //camDevice->afAvailableGet( available );

    bool running = false;
#if 0
    CamEngineAfSearchAlgorithm_t searchAlgorithm = CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_INVALID;
    if ( true == camDevice->getAfStatus( running, searchAlgorithm ) )
    {
        if ( CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_FULL_RANGE == searchAlgorithm )
        {
            SearchAlgorithmCombo->setCurrentIndex( 0 );   //FullSearch
        }
        else if ( CAM_ENGINE_AUTOFOCUS_SEARCH_ALGORITHM_HILL_CLIMBING == searchAlgorithm )
        {
            SearchAlgorithmCombo->setCurrentIndex( 2 );   //FullSearch
        }
        else
        {
            SearchAlgorithmCombo->setCurrentIndex( 1 );   //RangeSearch
        }

        AfStatusChangeLabel->setText( ( true == running ) ? "Tracking ..." : "Stopped" );
        SearchAlgorithmBox->setEnabled( !running );

        startButton->setEnabled( !running );
        stopButton->setEnabled( running  );
        oneShotButton->setEnabled( !running );
    }
    else
    {
        AfStatusChangeLabel->setText("N/A");
        SearchAlgorithmBox->setEnabled( false );

        startButton->setEnabled( false );
        stopButton->setEnabled( false );
        oneShotButton->setEnabled( false );
    }
#endif

}

