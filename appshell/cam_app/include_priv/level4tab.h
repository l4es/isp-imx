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


#ifndef __LEVEL_4_TAB_H__
#define __LEVEL_4_TAB_H__

#include <QTabWidget>

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


class Level4Tab : public QTabWidget
{
	Q_OBJECT

	public:
		Level4Tab( QWidget *parent = 0 );

    private:
        QGroupBox *MainCameraBackViewGroupBox;
        void createMainCameraBackViewGroupBox();
        QLabel *imageLabel;
        QScrollArea *scrollArea;
        double scaleFactor;
};

#endif /* __LEVEL_4_TAB_H__ */
