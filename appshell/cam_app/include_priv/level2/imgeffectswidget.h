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
 * @file imgeffectswidget.h
 *
 * @brief
 *   Image Effects Widget.
 *
 *****************************************************************************/

#ifndef __IMG_EFFECTS_WIDGET_H__
#define __IMG_EFFECTS_WIDGET_H__

#include "VirtualCamera.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QSlider;
class QGroupBox;


/**
 * @brief Image Effects Widget class declaration.
 */
class ImgEffectsWidget : public QWidget
{
    Q_OBJECT

public:
    ImgEffectsWidget( VirtualCamera *camDevice, QWidget *parent = NULL );

public slots:
    void updateStatus();

private slots:
    void toggleEnable( bool checked );
    void changeEffect( int index );
    void changeRange();

    void setTintCb( int value );
    void setTintCr( int value );
    void setColor();
    void setColorThreshold( int value );
    void setMatrix();
    void setSharpen( double value );
    void setSharpenThreshold( int value );

    void changeSharpening( int value );

private:
    void createRangeGroupBox();
    void createTintGroupBox();
    void createColorSelectionGroupBox();
    void createSharpeningGroupBox();
    void createMatrixGroupBox();

    void createImgEffectsGroupBox();

private:
    VirtualCamera  			*m_camDevice;

    QCheckBox               *m_enableCheckBox;
    QComboBox               *m_effectComboBox;
    QComboBox               *m_rangeComboBox;
    QSpinBox                *m_tintCbSpinBox;
    QSlider                 *m_tintCbSlider;
    QSpinBox                *m_tintCrSpinBox;
    QSlider                 *m_tintCrSlider;
    QCheckBox               *m_redCheckBox;
    QCheckBox               *m_greenCheckBox;
    QCheckBox               *m_blueCheckBox;
    QSpinBox                *m_colorThresholdSpinBox;
    QSlider                 *m_colorThresholdSlider;
    QDoubleSpinBox          *m_sharpenFactorSpinBox;
    QSlider                 *m_sharpenFactorSlider;
    QSpinBox                *m_sharpenThresholdSpinBox;
    QSlider                 *m_sharpenThresholdSlider;
    QComboBox               *m_matrixComboBox[9];

    QGroupBox               *m_imgeffectsGroupBox;
    QGroupBox               *m_rangeGroupBox;
    QGroupBox               *m_tintGroupBox;
    QGroupBox               *m_colorSelectionGroupBox;
    QGroupBox               *m_sharpeningGroupBox;
    QGroupBox               *m_matrixGroupBox;
};


#endif /*__IMG_EFFECTS_WIDGET_H__*/
