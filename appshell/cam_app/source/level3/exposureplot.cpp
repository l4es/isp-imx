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

#include "level3/exposureplot.h"

#include <QtCore/QEvent>
#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>

#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_curve_fitter.h>
#include <qwt_painter.h>
#include <qwt_series_data.h>

class CurveData: public QwtSeriesData<QPointF>
{
public:
    CurveData();
    virtual ~CurveData();

private:
    CurveData(const CurveData &);
    CurveData &operator=( const CurveData & );

public:
    virtual QPointF sample(size_t i) const;
    virtual size_t size() const;

    virtual QRectF boundingRect() const;

    void lock();
    void unlock();

    void append(const QPointF &pos);
    void clear();
    void clear(double min);

private:
    class PrivateData;
    PrivateData *m_data;
};

class CurveData::PrivateData
{
public:
    PrivateData():
        boundingRect(1.0, 1.0, -2.0, -2.0) // invalid
    {
        values.reserve(1000);
    }

    inline void append(const QPointF &sample)
    {
        values.append(sample);

        // adjust the bounding rectangle

        if ( boundingRect.width() < 0 || boundingRect.height() < 0 )
        {
            boundingRect.setRect(sample.x(), sample.y(), 0.0, 0.0);
        }
        else
        {
            boundingRect.setRight(sample.x());

            if ( sample.y() > boundingRect.bottom() )
                boundingRect.setBottom(sample.y());

            if ( sample.y() < boundingRect.top() )
                boundingRect.setTop(sample.y());
        }
    }

    QReadWriteLock lock;

    QVector<QPointF> values;
    QRectF boundingRect;

    QMutex mutex; // protecting pendingValues
    QVector<QPointF> pendingValues;
};

CurveData::CurveData()
    : m_data (NULL)
{
    m_data = new PrivateData();
}

CurveData::~CurveData()
{
    if ( NULL != m_data )
    {
        delete m_data;
    }
}

size_t CurveData::size() const
{
    return m_data->values.size();
}

QPointF CurveData::sample(size_t i) const
{
    return m_data->values[i];
}

QRectF CurveData::boundingRect() const
{
    return m_data->boundingRect;
}

void CurveData::lock()
{
    m_data->lock.lockForRead();
}

void CurveData::unlock()
{
    m_data->lock.unlock();
}

void CurveData::append(const QPointF &sample)
{
    m_data->mutex.lock();
    m_data->pendingValues += sample;

    const bool isLocked = m_data->lock.tryLockForWrite();
    if ( isLocked )
    {
        const int numValues = m_data->pendingValues.size();
        const QPointF *pendingValues = m_data->pendingValues.data();

        for ( int i = 0; i < numValues; ++i )
        {
            m_data->append(pendingValues[i]);
        }

        m_data->pendingValues.clear();

        m_data->lock.unlock();
    }

    m_data->mutex.unlock();
}

void CurveData::clear()
{
    m_data->lock.lockForWrite();

    m_data->boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );//QRectF(1.0, 1.0, -2.0, -2.0); // invalid
    m_data->values.clear();
    m_data->pendingValues.clear();

    m_data->lock.unlock();
}

void CurveData::clear(double limit)
{
    m_data->lock.lockForWrite();

    m_data->boundingRect = QRectF(1.0, 1.0, -2.0, -2.0); // invalid

    const QVector<QPointF> values = m_data->values;
    m_data->values.clear();
    m_data->values.reserve(values.size());

    int index;
    for ( index = values.size() - 1; index >= 0; --index )
    {
        if ( values[index].x() < limit )
        {
            break;
        }
    }

    if ( index > 0 )
    {
        m_data->append(values[index++]);
    }

    while ( index < values.size() - 1 )
    {
        m_data->append(values[index++]);
    }

    m_data->lock.unlock();
}

/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/


/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/


/******************************************************************************
 * ExposurePlot
 *****************************************************************************/
ExposurePlot::ExposurePlot(QWidget *parent):
    QwtPlot(parent),
    m_paintedPoints(0),
    m_interval(0.0, 20.0),
    m_timerId(-1)
{
    QwtPlotCanvas *qpCanvas;
    m_directPainter = new QwtPlotDirectPainter();

    setAutoReplot(false);

    qpCanvas = dynamic_cast<QwtPlotCanvas *>(canvas());
    if (qpCanvas)
    {
        // The backing store is important, when working with widget
        // overlays ( f.e rubberbands for zooming ).
        // Here we don't have them and the internal
        // backing store of QWidget is good enough.

        qpCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);


        // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
        // works on X11. This has a nice effect on the performance.
        //qpCanvas->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);

        // Disabling the backing store of Qt improves the performance
        // for the direct painter even more, but the canvas becomes
        // a native window of the window system, receiving paint events
        // for resize and expose operations. Those might be expensive
        // when there are many points and the backing store of
        // the canvas is disabled. So in this application
        // we better don't both backing stores.

        if ( qpCanvas->testPaintAttribute( QwtPlotCanvas::BackingStore ) )
        {
            qpCanvas->setAttribute(Qt::WA_PaintOnScreen, true);
            qpCanvas->setAttribute(Qt::WA_NoSystemBackground, true);
        }
    }

    initGradient();

    plotLayout()->setAlignCanvasToScales(true);

    // axis
    QwtText bottomTitle("Time [s]");
    QwtText leftTitle("Integration Time [ms]");
    QwtText rightTitle("Gain");
    bottomTitle.setFont( this -> font() );
    leftTitle.setFont( this -> font() );
    rightTitle.setFont( this -> font() );

    enableAxis( QwtPlot::yRight );
    enableAxis( QwtPlot::yLeft );
    setAxisTitle( QwtPlot::xBottom, bottomTitle );
    setAxisTitle( QwtPlot::yLeft, leftTitle );
    setAxisTitle( QwtPlot::yRight, rightTitle );
    setAxisScale( QwtPlot::xBottom, m_interval.minValue(), m_interval.maxValue() );
    setAxisAutoScale( QwtPlot::yLeft, true );
    setAxisAutoScale( QwtPlot::yRight, true );
    //setAxisScale(QwtPlot::yLeft, 0.0, 200.0);
    //setAxisScale(QwtPlot::yRight, 0.0, 16.0);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(false);
    grid->attach(this);

    // marker
    m_origin = new QwtPlotMarker();
    m_origin->setLineStyle(QwtPlotMarker::Cross);
    m_origin->setValue(m_interval.minValue() + m_interval.width() / 2.0, 0.0);
    m_origin->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_origin->setLinePen(QPen(Qt::gray, 0.0, Qt::DashLine));
    m_origin->attach(this);

    // curves
    m_itimeCurve = new QwtPlotCurve( "Integration Time" );
    m_itimeCurve->setStyle(QwtPlotCurve::Lines);
    m_itimeCurve->setPen(QPen(Qt::green));
    m_itimeCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_itimeCurve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
    m_itimeCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    m_itimeCurve->setYAxis(QwtPlot::yLeft);
    m_itimeCurve->setData(new CurveData());
    m_itimeCurve->attach(this);

    m_gainCurve = new QwtPlotCurve( "Gain" );
    m_gainCurve->setStyle(QwtPlotCurve::Lines);
    m_gainCurve->setPen(QPen(Qt::red));
    m_gainCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_gainCurve->setPaintAttribute(QwtPlotCurve::ClipPolygons, false);
    m_gainCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    m_gainCurve->setYAxis(QwtPlot::yRight);
    m_gainCurve->setData(new CurveData());
    m_gainCurve->attach(this);
}


/******************************************************************************
 * ~ExposurePlot
 *****************************************************************************/
ExposurePlot::~ExposurePlot()
{
    delete m_directPainter;
}

void ExposurePlot::initGradient()
{
    QPalette pal = canvas()->palette();

#if QT_VERSION >= 0x040400
    QLinearGradient gradient( 0.0, 0.0, 1.0, 0.0 );
    gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
    gradient.setColorAt(0.0, QColor( 0, 49, 110 ) );
    gradient.setColorAt(1.0, QColor( 0, 87, 174 ) );

    pal.setBrush(QPalette::Window, QBrush(gradient));
#else
    pal.setBrush(QPalette::Window, QBrush( color ));
#endif

    canvas()->setPalette(pal);
}


/******************************************************************************
 * replot
 *****************************************************************************/
void ExposurePlot::replot()
{
    CurveData *itimeData = (CurveData *)m_itimeCurve->data();
    CurveData *gainData = (CurveData *)m_gainCurve->data();
    itimeData->lock();
    gainData->lock();

    QwtPlot::replot();
    m_paintedPoints = itimeData->size();

    itimeData->unlock();
    gainData->unlock();
}


/******************************************************************************
 * append
 *****************************************************************************/
void ExposurePlot::append( float gain, float itime )
{
    CurveData *itimeData = (CurveData *)m_itimeCurve->data();
    CurveData *gainData = (CurveData *)m_gainCurve->data();

    const double elapsed = m_clock.elapsed() / 1000.0;

    itimeData->append( QPointF( elapsed, itime ) );
    gainData->append( QPointF( elapsed, gain ) );
}


/******************************************************************************
 * clear
 *****************************************************************************/
void ExposurePlot::clear()
{
    CurveData *itimeData = (CurveData *)m_itimeCurve->data();
    CurveData *gainData = (CurveData *)m_gainCurve->data();

    itimeData->clear();
    gainData->clear();
}


/******************************************************************************
 * setIntervalLength
 *****************************************************************************/
void ExposurePlot::setIntervalLength( int interval )
{
    if ( interval > 0 && interval != (int)m_interval.width() )
    {
        m_interval.setMaxValue(m_interval.minValue() + (double)interval);
        setAxisScale(QwtPlot::xBottom, 
            m_interval.minValue(), m_interval.maxValue());

        replot();
    }
}


/******************************************************************************
 * updateCurve
 *****************************************************************************/
void ExposurePlot::updateCurve()
{
    CurveData *itimeData = (CurveData *)m_itimeCurve->data();
    CurveData *gainData = (CurveData *)m_gainCurve->data();
    itimeData->lock();
    gainData->lock();

    const int numPoints = itimeData->size();
    if ( numPoints > m_paintedPoints )
    {
        updateAxes();

        const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
        if ( doClip )
        {
            const QwtScaleMap itimeXMap = canvasMap( m_itimeCurve->xAxis() );
            const QwtScaleMap itimeYMap = canvasMap( m_itimeCurve->yAxis() );

            QRectF itimeBr = qwtBoundingRect( *itimeData,
                m_paintedPoints - 1, numPoints - 1 );

            const QwtScaleMap gainXMap = canvasMap( m_gainCurve->xAxis() );
            const QwtScaleMap gainYMap = canvasMap( m_gainCurve->yAxis() );

            QRectF gainBr = qwtBoundingRect( *gainData,
                m_paintedPoints - 1, numPoints - 1 );

            const QRect itimeClipRect = QwtScaleMap::transform( itimeXMap, itimeYMap, itimeBr ).toRect();
            m_directPainter->setClipRegion(
                    itimeClipRect.united( QwtScaleMap::transform( gainXMap, gainYMap, gainBr ).toRect() ) );
        }

        m_directPainter->drawSeries( m_itimeCurve,
            m_paintedPoints - 1, numPoints - 1 );
        m_directPainter->drawSeries( m_gainCurve,
            m_paintedPoints - 1, numPoints - 1 );
        m_paintedPoints = numPoints;
    }

    itimeData->unlock();
    gainData->unlock();
}


/******************************************************************************
 * incrementInterval
 *****************************************************************************/
void ExposurePlot::incrementInterval()
{
    m_interval = QwtInterval(m_interval.maxValue(),
        m_interval.maxValue() + m_interval.width());

    CurveData *itimeData = (CurveData *)m_itimeCurve->data();
    CurveData *gainData = (CurveData *)m_gainCurve->data();
    itimeData->clear(m_interval.minValue());
    gainData->clear(m_interval.minValue());


    // To avoid, that the grid is jumping, we disable 
    // the autocalculation of the ticks and shift them
    // manually instead.

    QwtScaleDiv scaleDiv = axisScaleDiv(QwtPlot::xBottom);
    scaleDiv.setInterval(m_interval);

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        QList<double> ticks = scaleDiv.ticks(i);
        for ( int j = 0; j < ticks.size(); j++ )
            ticks[j] += m_interval.width();
        scaleDiv.setTicks(i, ticks);
    }
    setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);

    m_origin->setValue(m_interval.minValue() + m_interval.width() / 2.0, 0.0);

    m_paintedPoints = 0;
    replot();
}


/******************************************************************************
 * timerEvent
 *****************************************************************************/
void ExposurePlot::timerEvent(QTimerEvent *event)
{
    if ( event->timerId() == m_timerId )
    {
        updateCurve();

        const double elapsed = m_clock.elapsed() / 1000.0;
        if ( elapsed > m_interval.maxValue() )
            incrementInterval();

        return;
    }

    QwtPlot::timerEvent(event);
}


/******************************************************************************
 * resizeEvent
 *****************************************************************************/
void ExposurePlot::resizeEvent(QResizeEvent *event)
{
    m_directPainter->reset();
    QwtPlot::resizeEvent(event);
}


/******************************************************************************
 * showEvent
 *****************************************************************************/
void ExposurePlot::showEvent( QShowEvent * )
{
    if ( -1 == m_timerId )
    {
        double interval = m_interval.width();
        m_interval.setMinValue( 0.0 );
        m_interval.setMaxValue( m_interval.minValue() + interval );

        setAxisScale(QwtPlot::xBottom,
            m_interval.minValue(), m_interval.maxValue());

        m_origin->setValue(m_interval.minValue() + m_interval.width() / 2.0, 0.0);

        clear();
        replot();

        m_timerId = startTimer(10);
        m_clock.start();
    }
}


/******************************************************************************
 * hideEvent
 *****************************************************************************/
void ExposurePlot::hideEvent( QHideEvent * )
{
    if ( -1 != m_timerId )
    {
        killTimer( m_timerId );
        m_timerId = -1;
    }
}
