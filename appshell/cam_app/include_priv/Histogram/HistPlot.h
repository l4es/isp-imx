#ifndef __HIST_PLOT_H__
#define __HIST_PLOT_H__

#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <QDate>

#include "Histogram.h"


class XScaleDraw: public QwtScaleDraw
{
public:
    virtual QwtText label( double value ) const
    {
    	long ValueX = ( long )value * 16U;
        QString aux = QString::number( ValueX );
		return ( QwtText( aux ) );
    }
};



class YScaleDraw: public QwtScaleDraw
{
public:
    virtual QwtText label( double value ) const
    {
    	long ValueX = ( long )value;
        QString aux = QString::number( ValueX );
		return ( QwtText( aux ) );
    }
};



class HistPlot: public QwtPlot
{
    Q_OBJECT

public:
    HistPlot( QWidget * = NULL );
    void populate( const unsigned int size, const double *values );
    void populate( const unsigned int size, const uint32_t *values );

private:
    Histogram *histogram;
};



#endif /* __HIST_PLOT_H__ */
