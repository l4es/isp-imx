#ifndef __CAC_PLOT_H__
#define __CAC_PLOT_H__

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class ColorBar;
class QwtWheel;

class CacPlot : public QwtPlot
{
    Q_OBJECT
public:
    CacPlot( QWidget *parent = NULL );
    virtual bool eventFilter( QObject *, QEvent * );
    void insertCurve(int lineID);
    void insertCurveDefault();

public Q_SLOTS:
    void insertCurve( int axis, double base );

private Q_SLOTS:

private:
    void insertCurve( Qt::Orientation, const QColor &, double base , int line);
    QwtPlotCurve *curve;
    QwtPlotCurve *curve1;
};

#endif /* __CAC_PLOT_H__ */
