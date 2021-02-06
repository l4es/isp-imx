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


#ifndef __ENV_TAB_H__
#define __ENV_TAB_H__

#include <QGroupBox>
#include <QTabWidget>
#include <QtGui>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>


#include "VirtualCamera.h"


//TODO: change to IOCTL
/*****************************************************************************/
/**
 * @brief Flicker period types for the AEC algorithm.
 *
 *****************************************************************************/
typedef enum CamEngineFlickerPeriod_e
{
    CAM_ENGINE_FLICKER_OFF   = 0x00,
    CAM_ENGINE_FLICKER_100HZ = 0x01,
    CAM_ENGINE_FLICKER_120HZ = 0x02
} CamEngineFlickerPeriod_t;


class EnvironmentTab : public QTabWidget
{
    Q_OBJECT

public:
    EnvironmentTab(  VirtualCamera* camDevice, QWidget *parent = 0);

public slots:
    void opened();
    void closed();
    void resChanged();

signals:
    void sensorSelectButtonClicked();
    void flickerPeriodChanged();
    void enable3DToggled( bool enable );
    void enableTestpatternToggled( bool enable );
    void resolutionChanged();
    void enableAfpsToggled( bool enable );

public:
    CamEngineFlickerPeriod_t flickerPeriod() const;
    uint32_t selectedResolution() const;

private:
    VirtualCamera *m_camDevice;

    void createProjectInfoGroupBox();
    void createGeneralInfoGroupBox();
    void createFlickerAvoidInfoGroupBox();
    void createSensorSetupGroupBox();
    void createSensorInfoGroupBox();
    void createSensorAInfoGroupBox();
    void createSensorBInfoGroupBox();

    void clearSensorInfo();
    void clearSensorConfig();
    void updateSensorInfo();
    void updateSensorConfig();

    QGroupBox *ProjectInfoGroupBox;
    QGroupBox *GeneralInfoGroupBox;
    QGroupBox *FlickerAvoidInfoGroupBox;
    QGroupBox *SensorSetupGroupBox;
    QGroupBox *SensorInfoGroupBox;
    QGroupBox *SensorAInfoGroupBox;
    QGroupBox *SensorBInfoGroupBox;

    QLabel *LblSensorDriverValue;
    QLabel *LblCalibDataValue;

    QLabel *LblSensorANameValue;
    QLabel *LblSensorAStatusValue;
    QLabel *LblSensorAIDValue;
    QLabel *LblSensorAConnectValue;
    QLabel *LblSABayerPatternValue;
    QLabel *LblSABusWidthValue;
    QLabel *LblSABlkLvlCtrlValue;
    QLabel *LblSAAutoGainCtrlValue;
    QLabel *LblSAAutoExpCtrlValue;
    QLabel *LblSAAutoWBValue;
    QLabel *LblSADefectPCValue;
    QLabel *LblSAMipiModeValue;

    QLabel *LblSensorBNameValue;
    QLabel *LblSensorBStatusValue;
    QLabel *LblSensorBIDValue;
    QLabel *LblSensorBConnectValue;
    QLabel *LblSBBayerPatternValue;
    QLabel *LblSBBusWidthValue;
    QLabel *LblSBBlkLvlCtrlValue;
    QLabel *LblSBAutoGainCtrlValue;
    QLabel *LblSBAutoExpCtrlValue;
    QLabel *LblSBAutoWBValue;
    QLabel *LblSBDefectPCValue;
    QLabel *LblSBMipiModeValue;

    QComboBox *flickerPeriodComboBox;

    QCheckBox *sensorBCheckBox;
    QCheckBox *testpatternCheckBox;

    QComboBox *ResolutionComboBox;
    QCheckBox *AfpsCheckBox;
};

#endif /* __ENV_TAB_H__ */
