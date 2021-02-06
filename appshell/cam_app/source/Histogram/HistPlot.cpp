#include "Histogram/HistPlot.h"



HistPlot::HistPlot
(
    QWidget *parent
)
    : QwtPlot( parent )
{
    histogram =new Histogram( "pixel", Qt::green );

    QwtText bottomTitle("Luminance");
    QwtText leftTitle("No. of Pixel");
    bottomTitle.setFont( this -> font() );
    leftTitle.setFont( this -> font() );

    //setTitle( title );
    setCanvasBackground( QColor(Qt::gray) );
    plotLayout()->setAlignCanvasToScales( true );

    double val[16]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    populate( (sizeof(val)/sizeof(double)), val );

    setAxisTitle( QwtPlot::xBottom, bottomTitle );
    setAxisScale( QwtPlot::xBottom, 0, 16, 1 );
    setAxisMaxMinor( QwtPlot::xBottom,  0 );
    setAxisMaxMajor( QwtPlot::xBottom, 16 );
    setAxisScaleDraw( QwtPlot::xBottom, new XScaleDraw() );

    setAxisTitle( QwtPlot::yLeft, leftTitle );
    //setAxisScale( QwtPlot::yLeft, 0.0f, (double)0xFFFFF, (double)(0xFFFFF >> 4) );
    setAxisMaxMinor( QwtPlot::yLeft,  0 );
    setAxisMaxMajor( QwtPlot::yLeft, 16 );
    setAxisScaleDraw( QwtPlot::yLeft, new YScaleDraw() );

    QwtPlotItemList items = itemList( QwtPlotItem::Rtti_PlotHistogram );
    for ( int i = 0; i < items.size(); i++ )
    {
        if ( i == 0 )
        {
           items[i]->setVisible( true );
        }
    }

    setAutoReplot( true );
    resize( 800, 300 );
}



void HistPlot::populate
(
	const unsigned int 	size,
    const double 		*values
)
{
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX( false );
    grid->enableY( true );
    grid->enableXMin( false );
    grid->enableYMin( false );
    //grid->setMajPen( QPen(Qt::black, 0, Qt::DotLine) );
    grid->setMajorPen( QPen(Qt::black, 0, Qt::DotLine) );
    grid->attach(this);

    histogram->setValues( size, values );
    histogram->attach( this );
}


void HistPlot::populate
(
    const unsigned int  size,
    const uint32_t        *values
)
{
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableX( false );
    grid->enableY( true );
    grid->enableXMin( false );
    grid->enableYMin( false );
    //sunnygrid->setMajPen( QPen(Qt::black, 0, Qt::DotLine) );
    grid->setMajorPen( QPen(Qt::black, 0, Qt::DotLine) );
    grid->attach(this);

    histogram->setValues( size, values );
    histogram->attach( this );
}

