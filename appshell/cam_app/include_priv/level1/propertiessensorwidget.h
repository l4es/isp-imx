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


#ifndef __PROPERTIESSENSORWIDGET_H__
#define __PROPERTIESSENSORWIDGET_H__

#include "ui_propertiessensorwidget.h"

#include "VirtualCamera.h"


class IsiRegModel;


/**
 * @brief PropertiesSensorWidget class declaration.
 */
class PropertiesSensorWidget
  : public QWidget, private Ui::PropertiesSensorWidget
{
    Q_OBJECT

public:
    /**
     * @brief Standard constructor for the PropertiesSensorWidget object.
     *
     * @param   parent      Qt parent object.
     */
    PropertiesSensorWidget( VirtualCamera* camDevice, QWidget* parent, bool main_notsub );

public slots:
    void opened();
    void closed();
    void connected();
    void disconnected();
    void started();
    void stopped();
    void resChanged();

private slots:
    void updateSensorConfig();
    void update3D( bool checked );
    void updateTestpattern(  bool checked );
    void dumpRegister();
    void readRegister();
    void writeRegister();
    void selectRegister( const QModelIndex &index );

    void updateStatus();

protected:
    virtual void showEvent( QShowEvent *event );

private:
    void refreshSensorCaps();
    void refreshSensorConfig( bool resOnly = false );
    void refresh3D();
    void refreshTestpattern();
    void refreshRegisterAccess();
    void refreshRegisterTable();

private:
    VirtualCamera   *m_camDevice;
    IsiRegModel    *m_regModel;
    bool            m_main_notsub;
};


/* @} module_name_api*/

#endif /*__PROPERTIESSENSORWIDGET_H__*/
