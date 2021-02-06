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
 * @file cprocwidget.h
 *
 * @brief
 *   Color Processing Widget.
 *
 *****************************************************************************/

#ifndef __C_PROC_WIDGET_H__
#define __C_PROC_WIDGET_H__


#include "VirtualCamera.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QCheckBox;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QGroupBox;

/**
 * @brief Color Processing Widget class declaration.
 */
class CProcWidget : public QWidget
{
Q_OBJECT

public:
    CProcWidget( VirtualCamera *camDevice, QWidget *parent = NULL );

public slots:
    void updateStatus();

private slots:
    void toggleEnable( bool checked );
    void changeRange();

    void setContrast( double value );
    void setBrightness( int value );
    void setSaturation( double value );
    void setHue( double value );

    void changeContrast( int value );
    void changeSaturation( int value );
    void changeHue( int value );

private:
    void createChromaOutGroupBox();
    void createLumaInGroupBox();
    void createLumaOutGroupBox();
    void createContrastGroupBox();

    void createCProcGroupBox();

private:
    VirtualCamera  			*m_camDevice;

    QCheckBox               *m_enableCheckBox;
    QComboBox               *m_chromaOutComboBox;
    QComboBox               *m_lumaInComboBox;
    QComboBox               *m_lumaOutComboBox;

    QDoubleSpinBox          *m_contrastSpinBox;
    QSpinBox                *m_brightnessSpinBox;
    QDoubleSpinBox          *m_saturationSpinBox;
    QDoubleSpinBox          *m_hueSpinBox;
    QSlider                 *m_contrastSlider;
    QSlider                 *m_brightnessSlider;
    QSlider                 *m_saturationSlider;
    QSlider                 *m_hueSlider;

    QGroupBox               *m_cprocGroupBox;
    QGroupBox               *m_chromaOutGroupBox;
    QGroupBox               *m_lumaInGroupBox;
    QGroupBox               *m_lumaOutGroupBox;
    QGroupBox               *m_contrastGroupBox;
};


#endif /*__C_PROC_WIDGET_H__*/
