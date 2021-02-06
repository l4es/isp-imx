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

#include "level2/wdrwidget.h"



WdrWidget::WdrWidget(VirtualCamera *camDevice, QWidget *parent)
    : QWidget    (parent),
      m_camDevice(camDevice)
{
    createWdrGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_wdrGroupBox);
    setLayout(mainLayout);

    connect(parent, SIGNAL(UpdateCamEngineStatus()), SLOT(updateStatus()));

    updateStatus();
}

void WdrWidget::createWdrGroupBox()
{
    m_wdrGroupBox = new QGroupBox(tr("WDR (Wide Dynamic Range)"));

    QGridLayout *mainLayout = new QGridLayout;

    m_enableCheckBox = new QCheckBox(tr("Enable Wdr3"));
    m_enableCheckBox3dnr = new QCheckBox(tr("Enable 3Dnr"));
    //createWdrCurveSelectGroupBox();
    createWdrSliderGroupBox();
    createDnr3SliderGroupBox();
    mainLayout->addWidget(m_enableCheckBox);
    mainLayout->addWidget(m_enableCheckBox3dnr, 1, 0);
    mainLayout->addWidget(wdr3SliderGroupBox, 2, 0);
    mainLayout->addWidget(dnr3SliderGroupBox, 3, 0);
    mainLayout->setRowStretch(4, 1);
    m_enableCheckBox3dnr->setEnabled(false);
    connect(m_enableCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleEnable(bool)));
    connect(m_enableCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleEnable3Dnr(bool)));

    m_wdrGroupBox->setLayout(mainLayout);
}


void WdrWidget::toggleEnable(bool checked) {
    if (true == checked) {
        //updateWdr3Strength(true);
        Json::Value jRequest, jResponse;
        jRequest[WDR_GENERATION_PARAMS] = 2;
        jRequest[WDR_ENABLE_PARAMS] = true;
        m_camDevice->ioctl(ISPCORE_MODULE_WDR_ENABLE_SET, jRequest, jResponse);
    } else {
        Json::Value jRequest, jResponse;
        jRequest[WDR_GENERATION_PARAMS] = 2;
        jRequest[WDR_ENABLE_PARAMS] = false;
        m_camDevice->ioctl(ISPCORE_MODULE_WDR_ENABLE_SET, jRequest, jResponse);
    }
}

void WdrWidget::toggleEnable3dnr(bool checked) {
    if (true == checked) {
        int val = dnr3SpinBox->value();
        update3dnrStrength(val);
        Json::Value jRequest, jResponse;
        jRequest["generation"] = 0;
        jRequest[NR3D_ENABLE_PARAMS] = true;
        m_camDevice->ioctl(ISPCORE_MODULE_3DNR_ENABLE_SET, jRequest, jResponse);
    } else {
        Json::Value jRequest, jResponse;
        jRequest["generation"] = 0;
        jRequest[NR3D_ENABLE_PARAMS] = false;
        m_camDevice->ioctl(ISPCORE_MODULE_3DNR_ENABLE_SET, jRequest, jResponse);
    }
}


void WdrWidget::updateStatus() {
    Json::Value jRequest, jResponse;
    jRequest[WDR_GENERATION_PARAMS] = 2;
    m_camDevice->ioctl(ISPCORE_MODULE_WDR_ENABLE_SET, jRequest, jResponse);
    //setEnabled(jRequest[WDR_ENABLE_PARAMS].asBool());
}

void WdrWidget::updateWdr3Strength(int strength, int gain, int g_strength) {
    Json::Value jRequest, jResponse;
    jRequest[WDR_GENERATION_PARAMS] = 2;
    jRequest[WDR_STRENGTH_PARAMS] = strength;
    jRequest[WDR_GAIN_MAX_PARAMS] = gain;
    jRequest[WDR_STRENGTH_GLOBAL_PARAMS] = g_strength;
    m_camDevice->ioctl(ISPCORE_MODULE_WDR_CFG_SET, jRequest, jResponse);
}

void WdrWidget::createWdrSliderGroupBox()
{
    wdr3GridLayout = new QGridLayout();
    wdr3SliderGroupBox = new QGroupBox(tr("WDR3"));
    wdr3HorizontalSlider = new QSlider(wdr3SliderGroupBox);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(wdr3HorizontalSlider->sizePolicy().hasHeightForWidth());
    wdr3HorizontalSlider->setSizePolicy(sizePolicy1);
    wdr3HorizontalSlider->setOrientation(Qt::Horizontal);
    wdr3HorizontalSlider->setTracking(false);

    wdr3GridLayout->addWidget(wdr3HorizontalSlider, 0, 0, 1, 2);

    wdr3SpinBox = new QSpinBox(wdr3SliderGroupBox);
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(wdr3SpinBox->sizePolicy().hasHeightForWidth());
    wdr3SpinBox->setSizePolicy(sizePolicy2);
    wdr3SpinBox->setMinimumSize(QSize(100, 22));
    wdr3SpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    wdr3SpinBox->setRange(0, 128);
    wdr3GridLayout->addWidget(wdr3SpinBox, 0, 2, 1, 1);

    wdr3MinText = new QLabel(tr("0"), wdr3SliderGroupBox);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(wdr3MinText->sizePolicy().hasHeightForWidth());
    wdr3MinText->setSizePolicy(sizePolicy);
    wdr3MinText->setMinimumSize(QSize(150, 0));

    wdr3GridLayout->addWidget(wdr3MinText, 1, 0, 1, 1);

    wdr3MaxText = new QLabel(tr("128"), wdr3SliderGroupBox);
    sizePolicy.setHeightForWidth(wdr3MaxText->sizePolicy().hasHeightForWidth());
    wdr3MaxText->setSizePolicy(sizePolicy);
    wdr3MaxText->setMinimumSize(QSize(150, 0));
    wdr3MaxText->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    wdr3GridLayout->addWidget(wdr3MaxText, 1, 1, 1, 1);

    wdr3SliderGroupBox->setLayout(wdr3GridLayout);

    wdr3HorizontalSlider->setRange(0, 128);

    connect(wdr3SpinBox, SIGNAL(valueChanged(int)),  this, SLOT(updateWdr(int)));
    connect(wdr3HorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeWdr(int)));
}

void WdrWidget::updateWdr(int val) {
    wdr3HorizontalSlider->blockSignals(true);
    wdr3HorizontalSlider->setValue(val);
    wdr3HorizontalSlider->blockSignals(false);
    updateWdr3Strength(val, val, val);
}

void WdrWidget::changeWdr(int val) {
    wdr3SpinBox->setValue(val);
    updateWdr3Strength(val, val, val);
}


void WdrWidget::createDnr3SliderGroupBox()
{
    dnr3GridLayout = new QGridLayout();
    dnr3SliderGroupBox = new QGroupBox(tr("3DNR"));
    dnr3HorizontalSlider = new QSlider(dnr3SliderGroupBox);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(dnr3HorizontalSlider->sizePolicy().hasHeightForWidth());
    dnr3HorizontalSlider->setSizePolicy(sizePolicy1);
    dnr3HorizontalSlider->setOrientation(Qt::Horizontal);
    dnr3HorizontalSlider->setTracking(false);

    dnr3GridLayout->addWidget(dnr3HorizontalSlider, 0, 0, 1, 2);

    dnr3SpinBox = new QSpinBox(dnr3SliderGroupBox);
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(dnr3SpinBox->sizePolicy().hasHeightForWidth());
    dnr3SpinBox->setSizePolicy(sizePolicy2);
    dnr3SpinBox->setMinimumSize(QSize(100, 22));
    dnr3SpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    dnr3SpinBox->setRange(0, 128);
    dnr3GridLayout->addWidget(dnr3SpinBox, 0, 2, 1, 1);

    dnr3MinText = new QLabel(tr("0"), dnr3SliderGroupBox);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(dnr3MinText->sizePolicy().hasHeightForWidth());
    dnr3MinText->setSizePolicy(sizePolicy);
    dnr3MinText->setMinimumSize(QSize(150, 0));

    dnr3GridLayout->addWidget(dnr3MinText, 1, 0, 1, 1);

    dnr3MaxText = new QLabel(tr("128"), dnr3SliderGroupBox);
    sizePolicy.setHeightForWidth(dnr3MaxText->sizePolicy().hasHeightForWidth());
    dnr3MaxText->setSizePolicy(sizePolicy);
    dnr3MaxText->setMinimumSize(QSize(150, 0));
    dnr3MaxText->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    dnr3GridLayout->addWidget(dnr3MaxText, 1, 1, 1, 1);

    dnr3SliderGroupBox->setLayout(dnr3GridLayout);

    dnr3HorizontalSlider->setRange(0, 128);

    dnr3SpinBox->setEnabled(false);
    dnr3HorizontalSlider->setEnabled(false);
    connect(dnr3SpinBox, SIGNAL(valueChanged(int)), this, SLOT(update3dnr(int)));
    connect(dnr3HorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(change3dnr(int)));
}

void WdrWidget::update3dnrStrength(int strength) {
    Json::Value jRequest, jResponse;
    jRequest["generation"] = 0;
    jRequest[NR3D_STRENGTH_PARAMS] = strength;
    m_camDevice->ioctl(ISPCORE_MODULE_3DNR_CFG_SET, jRequest, jResponse);
}

void WdrWidget::update3dnr(int val) {
    dnr3HorizontalSlider->blockSignals(true);
    dnr3HorizontalSlider->setValue(val);
    dnr3HorizontalSlider->blockSignals(false);
    update3dnrStrength(val);
}

void WdrWidget::change3dnr(int val) {
    dnr3SpinBox->setValue(val);
    update3dnrStrength(val);
}
