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


#ifndef __IMAGEVIEWER_H__
#define __IMAGEVIEWER_H__

#include <QtGui>
#include <QTabWidget>
#include <QGroupBox>
#include <QtGui/QMovie>
#include <QtWidgets/QWidget>
#include <QtCore/QRect>
#include <QtGui/QImage>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoFrame>

#include <QString>
#include <QAbstractItemModel>
#include <QtXml/QDomDocument>

#include <QVariant>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>

#include <QMainWindow>

#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollBar>



class QAction;
class QLabel;
class QTreeView;
class QAbstractButton;
class QAbstractVideoSurface;
class QSlider;

class ImageViewer : public QMainWindow
{
    Q_OBJECT

    public:
        ImageViewer( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
        void open(QDir dir);

        private slots:
        void slotOpen();
        void zoomIn();
        void zoomOut();
        void normalSize();
        void fitToWindow();
        void saveAs();
        void exit();

    private:
        void createActions();
        void createMenus();
        void updateActions();
        void scaleImage(double factor);
        void adjustScrollBar(QScrollBar *scrollBar, double factor);

        QLabel *imageLabel;
        QScrollArea *scrollArea;
        double scaleFactor;

#ifndef QT_NO_PRINTER
        QPrinter printer;
#endif

        QAction *openAct;
        QAction *saveAsAct;
        QAction *exitAct;
        QAction *zoomInAct;
        QAction *zoomOutAct;
        QAction *normalSizeAct;
        QAction *fitToWindowAct;  

        QMenu *fileMenu;
        QMenu *viewMenu;
        
        QString imageFileName;
};

#endif /* __IMAGEVIEWER_H__ */
