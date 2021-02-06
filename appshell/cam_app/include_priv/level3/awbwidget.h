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
 * @file awbwidget.h
 *
 * @brief
 *   Auto White Balance Widget.
 *
 *****************************************************************************/
#ifndef __AWB_WIDGET_H__
#define __AWB_WIDGET_H__


#include "VirtualCamera.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QGroupBox;
class QPushButton;
class QLineEdit;
class QSpinBox;


class AwbTab : public QWidget
{
Q_OBJECT

public:
    AwbTab( VirtualCamera *camDevice, QWidget *parent = NULL );

public slots:
    void updateStatus();

private slots:

    void StartBtn();
    void StopBtn();
    void ResetBtn();

    void changeMode( int index );

private:
    void createAwbGroupBox();

private:
    VirtualCamera  			*m_camDevice;

    QGroupBox               *m_awbGroupBox;
    QLabel                  *AwbStatusChangeLabel;

    QGroupBox               *ModeBox;
    QGroupBox               *ManualBox;
    QGroupBox               *AutoBox;
    QGroupBox               *m_RGBox;
    QGroupBox               *m_DampingBox;

    QComboBox               *ModeCombo;
    QComboBox               *ManualIlluminationCombo;
    QComboBox               *AutoIlluminationCombo;

    QPushButton             *startButton;
    QPushButton             *stopButton;
    QPushButton             *resetButton;
    
    QLineEdit               *m_RGIndoorMinEdit;
    QLineEdit               *m_RGOutdoorMinEdit;
    QLineEdit               *m_RGMaxEdit;
    QLineEdit               *m_RGMaxSkyEdit;

    QCheckBox               *m_enableDampingCheckBox;
    QSpinBox                *m_IIRFiltersizeEdit;
    QLineEdit               *m_IIRFilterInitValueEdit;
};


#endif /* __AWB_WIDGET_H__ */
