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
 * @file wdrplot.h
 *
 * @brief
 *   WDR Processing Widget.
 *
 *****************************************************************************/
#ifndef __WDR_PLOT_H__
#define __WDR_PLOT_H__

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class ColorBar;
class QwtWheel;



class WdrCurveData
{
    // A container class for growing data
public:
    WdrCurveData();

    void append( double *x, double *y, int );

    int count() const;
    int size() const;
    const double *x() const;
    const double *y() const;

private:
    int m_count;
    QVector<double> m_x;
    QVector<double> m_y;
};



class WdrPlot : public QwtPlot
{
Q_OBJECT
public:
    WdrPlot( const QString &title, QWidget *parent = NULL );

    virtual bool eventFilter( QObject *, QEvent * );
    
    void insertCurve( Qt::Orientation, const QColor &, const QString &, WdrCurveData * );

private:
    QwtPlotCurve *m_CurvePlot;
};

#endif /* __WDR_PLOT_H__ */
