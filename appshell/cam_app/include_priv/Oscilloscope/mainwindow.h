#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <qwidget.h>

#include "Oscilloscope/plot.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget * = NULL);

    void start();

    double amplitude() const;
    double frequency() const;
    double signalInterval() const;

Q_SIGNALS:
    void amplitudeChanged(double);
    void frequencyChanged(double);
    void signalIntervalChanged(double);

private:

    Plot *d_plot;
};

#endif //_MAIN_WINDOW_H_