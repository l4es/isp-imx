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

/**
 * @file calibdatawidget.cpp
 *
 * @brief
 *   Implementation of widget for calibration data.
 *
 *****************************************************************************/
#include "level1/calibdatawidget.h"

#include <calib_xml/calibtreewidget.h>

#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtGui/QScrollArea>
#include <QtGui/QVBoxLayout>

CalibDataWidget::CalibDataWidget( VirtualCamera* camDevice, QWidget* parent )
  : QWidget( parent ),
    m_camDevice( camDevice )
{
    DCT_ASSERT( NULL != m_camDevice );

    // setup ui
    QWidget *scrollAreaWidgetContents = new QWidget();
    QVBoxLayout *verticalLayoutScroll = new QVBoxLayout(scrollAreaWidgetContents);

    QGroupBox *infoGroupBox = new QGroupBox(scrollAreaWidgetContents);
    infoGroupBox->setTitle( tr( "Information" ));
    QFormLayout *formLayout = new QFormLayout(infoGroupBox);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    QLabel *fileNameText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(fileNameText->sizePolicy().hasHeightForWidth());
    fileNameText->setText( tr( "Filename" ) );
    fileNameText->setSizePolicy(sizePolicy);
    fileNameText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(0, QFormLayout::LabelRole, fileNameText);

    m_fileNameLabel = new QLabel(infoGroupBox);
    m_fileNameLabel->setFrameShape(QFrame::WinPanel);
    m_fileNameLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(0, QFormLayout::FieldRole, m_fileNameLabel);

    QLabel *dateText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(dateText->sizePolicy().hasHeightForWidth());
    dateText->setText( tr( "Creation Date" ) );
    dateText->setSizePolicy(sizePolicy);
    dateText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(1, QFormLayout::LabelRole, dateText);

    m_dateLabel = new QLabel(infoGroupBox);
    m_dateLabel->setFrameShape(QFrame::WinPanel);
    m_dateLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(1, QFormLayout::FieldRole, m_dateLabel);

    QLabel *creatorText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(creatorText->sizePolicy().hasHeightForWidth());
    creatorText->setText( tr( "Creator" ) );
    creatorText->setSizePolicy(sizePolicy);
    creatorText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(2, QFormLayout::LabelRole, creatorText);

    m_creatorLabel = new QLabel(infoGroupBox);
    m_creatorLabel->setFrameShape(QFrame::WinPanel);
    m_creatorLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(2, QFormLayout::FieldRole, m_creatorLabel);

    QLabel *sensorNameText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(sensorNameText->sizePolicy().hasHeightForWidth());
    sensorNameText->setText( tr( "Sensor Name" ) );
    sensorNameText->setSizePolicy(sizePolicy);
    sensorNameText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(3, QFormLayout::LabelRole, sensorNameText);

    m_sensorNameLabel = new QLabel(infoGroupBox);
    m_sensorNameLabel->setFrameShape(QFrame::WinPanel);
    m_sensorNameLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(3, QFormLayout::FieldRole, m_sensorNameLabel);

    QLabel *sampleNameText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(sampleNameText->sizePolicy().hasHeightForWidth());
    sampleNameText->setText( tr( "Sample Name" ) );
    sampleNameText->setSizePolicy(sizePolicy);
    sampleNameText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(4, QFormLayout::LabelRole, sampleNameText);

    m_sampleNameLabel = new QLabel(infoGroupBox);
    m_sampleNameLabel->setFrameShape(QFrame::WinPanel);
    m_sampleNameLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(4, QFormLayout::FieldRole, m_sampleNameLabel);

    QLabel *generatorText = new QLabel(infoGroupBox);
    sizePolicy.setHeightForWidth(generatorText->sizePolicy().hasHeightForWidth());
    generatorText->setText( tr( "Generator Version" ) );
    generatorText->setSizePolicy(sizePolicy);
    generatorText->setMinimumSize(QSize(150, 0));
    formLayout->setWidget(5, QFormLayout::LabelRole, generatorText);

    m_generatorLabel = new QLabel(infoGroupBox);
    m_generatorLabel->setFrameShape(QFrame::WinPanel);
    m_generatorLabel->setFrameShadow(QFrame::Sunken);
    formLayout->setWidget(5, QFormLayout::FieldRole, m_generatorLabel);

    verticalLayoutScroll->addWidget(infoGroupBox);

    QGroupBox *xmlGroupBox = new QGroupBox(scrollAreaWidgetContents);
    xmlGroupBox->setTitle( tr( "XML Data" ));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(xmlGroupBox->sizePolicy().hasHeightForWidth());
    xmlGroupBox->setSizePolicy(sizePolicy1);

    QVBoxLayout *verticalLayoutXml = new QVBoxLayout(xmlGroupBox);

    m_treeWidget = new CalibTreeWidget(scrollAreaWidgetContents);
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(m_treeWidget->sizePolicy().hasHeightForWidth());
    m_treeWidget->setSizePolicy(sizePolicy2);

    verticalLayoutXml->addWidget(m_treeWidget);
    verticalLayoutScroll->addWidget(xmlGroupBox);

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
    setLayout( verticalLayout );
}

void CalibDataWidget::opened()
{
    refreshCalibData();
}

void CalibDataWidget::closed()
{
    refreshCalibData();
}

void CalibDataWidget::showEvent( QShowEvent *event )
{
    refreshCalibData();

    QWidget::showEvent( event );
}

void CalibDataWidget::refreshCalibData()
{
#if 0
    const char* fileName = m_camDevice->calibrationDataFile();
    if ( NULL != fileName )
    {
        QFile file ( QString::fromAscii( fileName ) );

		if ( true == m_treeWidget->readFile( &file ) )
		{
	    	m_treeWidget->setEnabled( true );

			m_fileNameLabel->setText( fileName );
			m_dateLabel->setText( m_treeWidget->getCreationDate() );
			m_creatorLabel->setText( m_treeWidget->getCreator() );
			m_sensorNameLabel->setText( m_treeWidget->getSensorName() );
			m_sampleNameLabel->setText( m_treeWidget->getSampleName() );
			m_generatorLabel->setText( m_treeWidget->getGeneratorVersion() );
		}
    }
    else
    {
    	m_treeWidget->setEnabled( false );

        m_fileNameLabel->setText( tr( "N/A" ) );
        m_dateLabel->setText( tr( "N/A" ) );
        m_creatorLabel->setText( tr( "N/A" ) );
        m_sensorNameLabel->setText( tr( "N/A" ) );
        m_sampleNameLabel->setText( tr( "N/A" ) );
        m_generatorLabel->setText( tr( "N/A" ) );
    }
#endif
}

