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




#include <QtMultimedia>
#include <QtGui>
#include <QtXml/QDomDocument>
#include <QFile>

#include "level1tab.h"
#include "level4tab.h"
#include "imageViewer.h"


Level4Tab::Level4Tab( camdev::Engine *parent )
    : QTabWidget(parent)
{
    createMainCameraBackViewGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( MainCameraBackViewGroupBox );

    setLayout( mainLayout );

}

void Level4Tab::createMainCameraBackViewGroupBox()
{
    MainCameraBackViewGroupBox = new QGroupBox( tr("Nikon_BackView") );
    QGridLayout *layout = new QGridLayout;

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
//    setCentralWidget(scrollArea);


    QString fileName = "../../sw_gui_appl/ressource/images/Nikon_Backview.jpg";
    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Image Viewer"),
                    tr("Cannot load %1.").arg(fileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(image));
//        imageLabel->resize(400 ,300);

        scrollArea->setWidgetResizable(true);
//        fitToWindowAct->setEnabled(true);
//        updateActions();

//        if (!fitToWindowAct->isChecked())
//            imageLabel->adjustSize();
    }

    layout->addWidget(imageLabel , 1, 0);

    MainCameraBackViewGroupBox->setLayout(layout);
}






















