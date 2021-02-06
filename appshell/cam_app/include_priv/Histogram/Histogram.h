#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>


class Histogram: public QwtPlotHistogram
{
public:
    Histogram(const QString &, const QColor &);

    void setColor(const QColor &);
    void setValues(uint numValues, const double *);
    void setValues(uint numValues, const uint32_t *);
};


#endif /* __HISTOGRAM_H__ */
