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
 * @file aecwidget.h
 *
 * @brief
 *   Auto Exposure Control Widget.
 *
 *****************************************************************************/
#ifndef __AEC_WIDGET_H__
#define __AEC_WIDGET_H__


#include "VirtualCamera.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QGroupBox;
class QPushButton;
class QLineEdit;
class QDoubleSpinBox;
class QSlider;
class QTabWidget;
class ExposurePlot;
class HistPlot;
class Scene;



class AecTab : public QWidget
{
    Q_OBJECT

public:
    AecTab( VirtualCamera *camDevice, QWidget *parent = NULL );

public slots:
    void updateStatus();
    void buffer(isp_metadata *meta);

private slots:

    void StartBtn();
    void StopBtn();
    void ResetBtn();

    void changeInterval( int interval );
    void changeMode( int index );

    void setSetPoint( double value );
    void setClmTolerance( double value );
    void setDampOver( double value );
    void setDampUnder( double value );

    void changeSetPoint( int value );
    void changeClmTolerance( int value );
    void changeDampOver( int value );
    void changeDampUnder( int value );

    void configure();

private:
    void createAecGroupBox();

private:
    VirtualCamera            *m_camDevice;

    QGroupBox               *m_aecGroupBox;
    QLabel                  *AecStatusChangeLabel;

    QGroupBox               *m_configGroupBox;
    QGroupBox               *m_modeGroupBox;
    QGroupBox               *m_setPointGroupBox;
    QGroupBox               *m_dampingGroupBox;

    QComboBox               *m_modeComboBox;

    QDoubleSpinBox          *m_setPointSpinBox;
    QSlider                 *m_setPointSlider;
    QDoubleSpinBox          *m_clmToleranceSpinBox;
    QSlider                 *m_clmToleranceSlider;

    QDoubleSpinBox          *m_dampOverSpinBox;
    QSlider                 *m_dampOverSlider;
    QDoubleSpinBox          *m_dampUnderSpinBox;
    QSlider                 *m_dampUnderSlider;

    QPushButton             *startButton;
    QPushButton             *stopButton;
    QPushButton             *resetButton;

    uint8_t                 m_interval;
    uint8_t                 m_counter;
    QTabWidget              *m_measureTab;
    HistPlot                *m_histogram;
    Scene                   *m_scene;
    ExposurePlot            *m_exposure;
};


#endif /* __AEC_WIDGET_H__ */
