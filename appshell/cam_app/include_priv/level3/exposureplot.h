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
 * @file gainitimeplot.h
 *
 * @brief
 *   Widget to plot gain and integration time.
 *
 *****************************************************************************/
#ifndef __EXPOSUREPLOT_H__
#define __EXPOSUREPLOT_H__

#include <qwt_plot.h>
#include <qwt_interval.h>
#include <qwt_system_clock.h>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;

class ExposurePlot: public QwtPlot
{
    Q_OBJECT

public:
    ExposurePlot( QWidget * = NULL );
    virtual ~ExposurePlot();

    virtual void replot();

    void append( float gain, float itime );
    void clear();

public Q_SLOTS:
    void setIntervalLength( int interval );

protected:
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void timerEvent(QTimerEvent *);

private:
    void initGradient();

    void updateCurve();
    void incrementInterval();

private:
    QwtPlotMarker           *m_origin;
    QwtPlotCurve            *m_itimeCurve;
    QwtPlotCurve            *m_gainCurve;
    QwtPlotDirectPainter    *m_directPainter;
    int                     m_paintedPoints;
    QwtInterval             m_interval;
    int                     m_timerId;
    QwtSystemClock          m_clock;
};

#endif /*__EXPOSUREPLOT_H__*/
