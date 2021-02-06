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
 * @file calibdatawidget.h
 *
 * @brief
 *   Widget for calibration data.
 *
 *****************************************************************************/
#ifndef __CALIBDATA_WIDGET_H__
#define __CALIBDATA_WIDGET_H__

#include <QtWidgets/QWidget>
#include "VirtualCamera.h"



class QLabel;
class CalibTreeWidget;

/**
 * @brief CalibDataWidget class declaration.
 */
class CalibDataWidget
  : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Standard constructor for the CalibDataWidget object.
     *
     * @param   parent      Qt parent object.
     */
    CalibDataWidget( VirtualCamera* camDevice, QWidget* parent );

public slots:
    void opened();
    void closed();

protected:
    virtual void showEvent( QShowEvent *event );

private:
    void refreshCalibData();

private:
    VirtualCamera   	*m_camDevice;

    CalibTreeWidget *m_treeWidget;
    QLabel 			*m_fileNameLabel;
    QLabel 			*m_dateLabel;
    QLabel 			*m_creatorLabel;
    QLabel 			*m_sensorNameLabel;
    QLabel 			*m_sampleNameLabel;
    QLabel 			*m_generatorLabel;
};

#endif /*__CALIBDATA_WIDGET_H__*/
