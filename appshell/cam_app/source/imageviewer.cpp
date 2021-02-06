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
#include <QtXml/QDomDocument>
#include <QFile>

#include "imageviewer.h"

ImageViewer::ImageViewer( QWidget* parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
{
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();

    setWindowTitle(tr("Image Viewer"));
    resize(1000, 800);

    imageFileName = "";
}

void ImageViewer::slotOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        imageFileName = fileName;
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Image Viewer"),
                    tr("Cannot load %1.").arg(fileName));
            return; 
        }   
        imageLabel->setPixmap(QPixmap::fromImage(image));
        //imageLabel->setPixmap(QPixmap::fromImage(image));
//        scaleFactor = 1.0;

        fitToWindowAct->setEnabled(true);
        updateActions();

        if (!fitToWindowAct->isChecked())
            imageLabel->adjustSize();
    }
}

void ImageViewer::open(QDir dir)
{
    QString fileName;
    QStringList list = dir.entryList(QDir::Files, QDir::Time);        
    fileName = list.value(0)  ; 
    fileName = dir.absolutePath() +"/" + fileName;
    if (!fileName.isEmpty()) {
        imageFileName = fileName;
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Image Viewer"),
                    tr("Cannot load %1.").arg(fileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(image));
        scaleFactor = 1.0;

        fitToWindowAct->setEnabled(true);
        updateActions();

        if (!fitToWindowAct->isChecked())
            imageLabel->adjustSize();
    }
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        normalSize();
    }
    updateActions();
}

void ImageViewer::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(slotOpen()));

    saveAsAct = new QAction(tr("&Save As..."), this);
    saveAsAct->setShortcut(tr("Ctrl+S"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("&Exit"), this);
    exitAct->setShortcut(tr("Ctrl+E"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(exit()));

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

}

void ImageViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
}

void ImageViewer::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::saveAs()
{
    QString fileName;
    if (imageFileName.endsWith("ppm"))
    {
        fileName = QFileDialog::getSaveFileName(this,
            tr("Save Snapshot File"), QDir::currentPath(), tr("PPM Files(*.ppm)"));
        fileName.append(".ppm");
    }

    if (imageFileName.endsWith("pgm"))
    {
        fileName = QFileDialog::getSaveFileName(this,
            tr("Save Snapshot File"), QDir::currentPath(), tr("PGM Files(*.pgm)"));
        fileName.append(".pgm");
    }

    if (imageFileName.endsWith("jpg"))
    {
        fileName = QFileDialog::getSaveFileName(this,
            tr("Save Snapshot File"), QDir::currentPath(), tr("JPEG Files(*.jpg)"));
        fileName.append(".jpg");
    }

    QFile::copy(imageFileName, fileName);
}

void ImageViewer::exit()
{
    this->close();
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                + ((factor - 1) * scrollBar->pageStep()/2)));
}
