#include "cac/cac-plot.h"

#include <qevent.h>

#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>

#include <stdlib.h>

double cacxCurveArray[2][11] = {			
   {  -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8,
        1.0},
   {  -1.0, -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8,
        1.0},
};

double cacyCurveArray[2][11] = {			
    {  -0.1, -0.3, -0.5, -0.3, -0.1, 0.0, 0.1, 0.3, 0.5, 0.3, 0.1},
    {  0.8, 0.9, 0.8, 0.6, 0.3, 0.0, -0.3, -0.6, -0.8, -0.9, -0.8},
};

CacPlot::CacPlot( QWidget *parent ) : QwtPlot( parent )
{
    setTitle("Horizontal Chromatic Aberration Correction curve");
    curve = new QwtPlotCurve();
    curve1 = new QwtPlotCurve();

    setCanvasBackground( QColor(Qt::white) );

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    //sunnygrid->setMajPen( QPen(Qt::gray, 0, Qt::DotLine) );
    grid->setMajorPen( QPen(Qt::gray, 0, Qt::DotLine) );
    //sunnygrid->setMinPen( QPen(Qt::gray, 0 , Qt::DotLine) );
    grid->setMinorPen( QPen(Qt::gray, 0 , Qt::DotLine) );
    grid->attach( this );

    setAxisMaxMinor( QwtPlot::xBottom, 10 );

    // axes
    setAxisScale( QwtPlot::xBottom, -1.0, 1.0 );
    setAxisScale( QwtPlot::yLeft, -4.5, 4.5 );

    // Avoid jumping when label with 3 digits
    // appear/disappear when scrolling vertically
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( sd->extent( axisWidget( QwtPlot::yLeft )->font() ) );

    plotLayout()->setAlignCanvasToScales( true );

    insertCurve( Qt::Vertical, Qt::blue, 30.0, 0);
    insertCurve( Qt::Vertical, Qt::red, 30.0, 1);

    replot();

    // we need the resize events, to lay out the wheel
    canvas()->installEventFilter( this );
}

bool CacPlot::eventFilter( QObject *object, QEvent *e )
{
    return ( QwtPlot::eventFilter( object, e ) );
}
void CacPlot::insertCurve
(
	int axis,
	double base
)
{
/*
    Qt::Orientation o;
    if ( axis == yLeft || axis == yRight )
        o = Qt::Horizontal;
    else
        o = Qt::Vertical;
    
    QRgb rgb = (uint)rand();
    insertCurve(o, QColor(rgb), base);
    replot();
*/
}
int cacpotenz(const int n )
{
	int zahl2 = 1;

	for( int i=0; i<n; i++)
	{
		zahl2*=2;
	}

	return ( zahl2 );
}

void CacPlot::insertCurve
(
	Qt::Orientation o,
	const QColor &c,
	double base,
	int lines
)
{
    if(lines == 0)
    {
        curve->setPen( c );
        curve->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::gray, c, QSize(1, 1)) );

        double x[11];
        double y[11]; 
        for (int i=0; i< 11; i++)
        {
            x[i] = cacxCurveArray[lines][i] ;
            y[i] = cacyCurveArray[lines][i] ; 
        }
        curve->setSamples(x, y, (sizeof(x) / sizeof(x[0])) );
        curve->attach(this);
    }
    else
    {
        curve1->setPen( c );
        curve1->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::gray, c, QSize(1, 1)) );

        double x[11];
        double y[11]; 
        for (int i=0; i< 11; i++)
        {
            x[i] = cacxCurveArray[lines][i] ;
            y[i] = cacyCurveArray[lines][i] ; 
        }
        curve1->setSamples(x, y, (sizeof(x) / sizeof(x[0])) );
        curve1->attach(this);
    }
}


void CacPlot::insertCurve(int lineID)
{
    Qt::Orientation o = Qt::Vertical;
    const QColor &c   =  Qt::blue;
    double base = 30.0;

    curve->setPen( c );
    curve->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::gray, c, QSize(8, 8)) );

    double x[33];
	
    double y[33] 
    	= {    0.0,    0.0,    0.0,    0.0,    0.0,   10.0,   48.0,  117.0,  180.0,  289.0, 
     	     464.0,  602.0,  817.0, 1121.0, 1347.0, 1698.0, 1997.0, 2288.0, 2613.0, 2809.0,
    	    3052.0, 3205.0, 3396.0, 3515.0, 3659.0, 3744.0, 3842.0, 3897.0, 3958.0, 3991.0,
    	    4025.0, 4061.0, 4096.0 };

    for (int i=0; i< 11; i++)
    {
    	y[i] = cacyCurveArray[lineID][i];
    }

    x[0]  = 0.0; 

    for (int i=0; i< 32; i++)
    {
        x[i+1] = x[i] + 128;
    }

#if 0
	// ISP_CAC_TONECURVE_1
    x[1]  = x[0] + (double)(potenz(4 + 3));
    x[2]  = x[1] + (double)(potenz(2 + 3));
    x[3]  = x[2] + (double)(potenz(0 + 3));
    x[4]  = x[3] + (double)(potenz(0 + 3));
    x[5]  = x[4] + (double)(potenz(0 + 3));
    x[6]  = x[5] + (double)(potenz(0 + 3));
    x[7]  = x[6] + (double)(potenz(1 + 3));
    x[8]  = x[7] + (double)(potenz(1 + 3));

 	// ISP_CAC_TONECURVE_2
    x[9]  = x[8]  + (double)(potenz(2 + 3));
    x[10] = x[9]  + (double)(potenz(3 + 3));
    x[11] = x[10]  + (double)(potenz(3 + 3));
    x[12] = x[11] + (double)(potenz(4 + 3));
    x[13] = x[12] + (double)(potenz(5 + 3));
    x[14] = x[13] + (double)(potenz(5 + 3));
    x[15] = x[14] + (double)(potenz(6 + 3));
    x[16] = x[15] + (double)(potenz(6 + 3));

 	// ISP_CAC_TONECURVE_3
    x[17] = x[16] + (double)(potenz(6 + 3));
    x[18] = x[17] + (double)(potenz(6 + 3));
    x[19] = x[18] + (double)(potenz(5 + 3));
    x[20] = x[19] + (double)(potenz(5 + 3));
    x[21] = x[20] + (double)(potenz(4 + 3));
    x[22] = x[21] + (double)(potenz(4 + 3));
    x[23] = x[22] + (double)(potenz(3 + 3));
    x[24] = x[23] + (double)(potenz(3 + 3));

 	// ISP_CAC_TONECURVE_4
    x[25] = x[24] + (double)(potenz(2 + 3));
    x[26] = x[25] + (double)(potenz(2 + 3));
    x[27] = x[26] + (double)(potenz(1 + 3));
    x[28] = x[27] + (double)(potenz(1 + 3));
    x[29] = x[28] + (double)(potenz(0 + 3));
    x[30] = x[29] + (double)(potenz(0 + 3));
    x[31] = x[30] + (double)(potenz(0 + 3));
    x[32] = x[31] + (double)(potenz(0 + 3));
#endif
#if 0
    for ( uint i = 0; i < ( sizeof(x) / sizeof(x[0]) ); i++ )
    {
        double v = 5.0 + i * 10.0;
        if ( o == Qt::Horizontal )
        {
            x[i] = v;
        }
        else
        {
            x[i] = base;
        }
    }
#endif     
        
    curve->setSamples(x, y, (sizeof(x) / sizeof(x[0])) );
    curve->attach(this);

    replot();

    return ;
}

void CacPlot::insertCurveDefault()
{
    Qt::Orientation o = Qt::Vertical;
    const QColor &c   =  Qt::blue;
    double base = 30.0;

    curve->setPen( c );
    curve->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::gray, c, QSize(8, 8)) );

    double x[33];
    double y[33] 
    	= {    0.0,    0.0,    0.0,    0.0,    0.0,   10.0,   48.0,  117.0,  180.0,  289.0, 
     	     464.0,  602.0,  817.0, 1121.0, 1347.0, 1698.0, 1997.0, 2288.0, 2613.0, 2809.0,
    	    3052.0, 3205.0, 3396.0, 3515.0, 3659.0, 3744.0, 3842.0, 3897.0, 3958.0, 3991.0,
    	    4025.0, 4061.0, 4096.0 };

    x[0]  = 0.0; 

	// ISP_CAC_TONECURVE_1
    x[1]  = x[0] + (double)(cacpotenz(4 + 3));
    x[2]  = x[1] + (double)(cacpotenz(2 + 3));
    x[3]  = x[2] + (double)(cacpotenz(0 + 3));
    x[4]  = x[3] + (double)(cacpotenz(0 + 3));
    x[5]  = x[4] + (double)(cacpotenz(0 + 3));
    x[6]  = x[5] + (double)(cacpotenz(0 + 3));
    x[7]  = x[6] + (double)(cacpotenz(1 + 3));
    x[8]  = x[7] + (double)(cacpotenz(1 + 3));

 	// ISP_CAC_TONECURVE_2
    x[9]  = x[8]  + (double)(cacpotenz(2 + 3));
    x[10] = x[9]  + (double)(cacpotenz(3 + 3));
    x[11] = x[10]  + (double)(cacpotenz(3 + 3));
    x[12] = x[11] + (double)(cacpotenz(4 + 3));
    x[13] = x[12] + (double)(cacpotenz(5 + 3));
    x[14] = x[13] + (double)(cacpotenz(5 + 3));
    x[15] = x[14] + (double)(cacpotenz(6 + 3));
    x[16] = x[15] + (double)(cacpotenz(6 + 3));

 	// ISP_CAC_TONECURVE_3
    x[17] = x[16] + (double)(cacpotenz(6 + 3));
    x[18] = x[17] + (double)(cacpotenz(6 + 3));
    x[19] = x[18] + (double)(cacpotenz(5 + 3));
    x[20] = x[19] + (double)(cacpotenz(5 + 3));
    x[21] = x[20] + (double)(cacpotenz(4 + 3));
    x[22] = x[21] + (double)(cacpotenz(4 + 3));
    x[23] = x[22] + (double)(cacpotenz(3 + 3));
    x[24] = x[23] + (double)(cacpotenz(3 + 3));

 	// ISP_CAC_TONECURVE_4
    x[25] = x[24] + (double)(cacpotenz(2 + 3));
    x[26] = x[25] + (double)(cacpotenz(2 + 3));
    x[27] = x[26] + (double)(cacpotenz(1 + 3));
    x[28] = x[27] + (double)(cacpotenz(1 + 3));
    x[29] = x[28] + (double)(cacpotenz(0 + 3));
    x[30] = x[29] + (double)(cacpotenz(0 + 3));
    x[31] = x[30] + (double)(cacpotenz(0 + 3));
    x[32] = x[31] + (double)(cacpotenz(0 + 3));

    curve->setSamples(x, y, (sizeof(x) / sizeof(x[0])) );
    curve->attach(this);

    replot();

}



