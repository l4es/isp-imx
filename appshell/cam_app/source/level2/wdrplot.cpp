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

#include "level2/wdrplot.h"

#include <qevent.h>

#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>

WdrCurveData::WdrCurveData()
    : m_count(0)
{
}

void WdrCurveData::append( double *x, double *y, int count )
{
    int newSize = (m_count + count);
    
    if ( newSize > size() )
    {
        m_x.resize( newSize );
        m_y.resize( newSize );
    }

    for ( register int i = 0; i < count; i++ )
    {
        m_x[m_count + i] = x[i];
        m_y[m_count + i] = y[i];
    }

    m_count += count;
}



int WdrCurveData::count() const
{
    return ( m_count );
}



int WdrCurveData::size() const
{
    return ( m_x.size() );
}



const double *WdrCurveData::x() const
{
    return ( m_x.data() );
}



const double *WdrCurveData::y() const
{
    return ( m_y.data() );
}



/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

WdrPlot::WdrPlot( const QString &title, QWidget *parent ) 
    : QwtPlot( parent )
{
    setTitle( title );

    m_CurvePlot = new QwtPlotCurve();

    setCanvasBackground( QColor(Qt::darkBlue) );

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    //sunnygrid->setMajPen( QPen(Qt::white, 0, Qt::DotLine) );
    grid->setMajorPen( QPen(Qt::white, 0, Qt::DotLine) );
    //grid->setMinPen( QPen(Qt::gray, 0 , Qt::DotLine) );
    grid->setMinorPen( QPen(Qt::gray, 0 , Qt::DotLine) );
    grid->attach( this );

    setAxisMaxMinor( QwtPlot::xBottom, 10 );

    // axes
    setAxisScale( QwtPlot::xBottom, 0.0, 4500.0 );
    setAxisScale( QwtPlot::yLeft, -10.0, 4500.0 );

    // avoid jumping when label with 3 digits
    // appear/disappear when scrolling vertically
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( sd->extent( axisWidget( QwtPlot::yLeft )->font() ) );

    plotLayout()->setAlignCanvasToScales( true );

//    insertCurve( Qt::Vertical, Qt::blue );

    replot();

    // we need the resize events, to lay out the wheel
    canvas()->installEventFilter( this );
}


bool WdrPlot::eventFilter( QObject *object, QEvent *e )
{
    return ( QwtPlot::eventFilter( object, e ) );
}



void WdrPlot::insertCurve( Qt::Orientation o, const QColor &color, const QString &title, WdrCurveData *curveData )
{
    setTitle( title );

    m_CurvePlot->setPen( color );
    m_CurvePlot->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, Qt::gray, color, QSize( 8, 8 ) ) );
    m_CurvePlot->setSamples( curveData->x(), curveData->y(), curveData->count() );
    m_CurvePlot->attach( this );
    
    replot();
}

