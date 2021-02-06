#ifndef __C_CANVASPICKER_H__
#define __C_CANVASPICKER_H__

#include <qobject.h>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;

class CanvasPicker: public QObject
{
    Q_OBJECT
public:
    CanvasPicker(QwtPlot *plot);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual bool event(QEvent *);

private:
    void select(const QPoint &);
    void move(const QPoint &);
    void moveBy(int dx, int dy);

    void release();

    void showCursor(bool enable);
    void shiftPointCursor(bool up);
    void shiftCurveCursor(bool up);

    QwtPlot *plot() { return (QwtPlot *)parent(); }
    const QwtPlot *plot() const { return (QwtPlot *)parent(); }

    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;
};

#endif //#ifndef __C_CANVASPICKER_H__