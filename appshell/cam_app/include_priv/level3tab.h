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


#ifndef __LEVEL_3_TAB_H__
#define __LEVEL_3_TAB_H__

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

#include <QWidget>
#include <QVBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>
#include <QSplitter>
#include <QComboBox>
#include "VirtualCamera.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QErrorMessage>

class AfTab : public QWidget
{
    Q_OBJECT

    public:
        AfTab(QWidget *parent = 0);

    private slots:
        void StartBtn();
        void StopBtn();
        void OneShotBtn();

    private:
        void createAfBlockGroupBox();
        QGroupBox *AfBlockGroupBox;
        QLabel    *AfStatusChangeLabel;

        QGroupBox *SearchAlgorithmBox;

        QComboBox *SearchAlgorithmCombo;

        QPushButton *startButton;
        QPushButton *stopButton;
        QPushButton *oneShotButton;

    public slots:
        void updateStatus();
};


class Level3Tab : public QTabWidget
{
	Q_OBJECT

	public:
		Level3Tab( VirtualCamera *camDevice,  QWidget *parent = 0 );

    signals:
        void UpdateCamEngineStatus();

    private:
        VirtualCamera *m_camDevice;
};

#endif /* __LEVEL_3_TAB_H__ */
