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
 * @file wdrwidget.h
 *
 * @brief
 *   WDR Processing Widget.
 *
 *****************************************************************************/

#ifndef __C_WDR_WIDGET_H__
#define __C_WDR_WIDGET_H__

#include "VirtualCamera.h"

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>
#include <QtGui>
#include <QTabWidget>
#include <QGroupBox>
#include <QtGui/QMovie>
#include <QtCore/QRect>
#include <QtGui/QImage>

#include "level2/canvaspicker.h"
#include "level2/wdrplot.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>

#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>

class QRadioButton;
class QCheckBox;
class QGroupBox;
class QVBoxLayout;

class WdrWidget : public QWidget
{
Q_OBJECT
public:
    WdrWidget(VirtualCamera *camDevice, QWidget *parent = NULL);

public slots:
    void updateStatus();

private slots:
    void toggleEnable(bool checked);
    void toggleEnable3dnr(bool checked);
    void updateWdr(int val);
    void changeWdr(int val);
    void update3dnr(int val);
    void change3dnr(int val);

private:
    void createWdrGroupBox();
    void createWdrSliderGroupBox();
    void createDnr3SliderGroupBox();
    void updateWdr3Strength(int strength, int gain, int g_strength);
    void update3dnrStrength(int strength);

private:
    VirtualCamera *m_camDevice;
    QCheckBox *m_enableCheckBox;
    QCheckBox *m_enableCheckBox3dnr;
    QCheckBox *m_enableCompandCheckBox;
    QGroupBox *m_wdrGroupBox;
    
    QGroupBox *wdr3SliderGroupBox;
    QGridLayout *wdr3GridLayout;
    QSlider *wdr3HorizontalSlider;
    QSpinBox *wdr3SpinBox;
    QLabel *wdr3MinText;
    QLabel *wdr3MaxText;

    QGroupBox *dnr3SliderGroupBox;
    QGridLayout *dnr3GridLayout;
    QSlider *dnr3HorizontalSlider;
    QSpinBox *dnr3SpinBox;
    QLabel *dnr3MinText;
    QLabel *dnr3MaxText;
};

#endif /*__C_WDR_WIDGET_H__*/
