#include "Oscilloscope/mainwindow.h"
#include <qwt_scale_engine.h>
#include <qlabel.h>
#include <qlayout.h>

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent)
{
    const double intervalLength = 10.0; // seconds

    d_plot = new Plot(this);
    d_plot->setIntervalLength(intervalLength);


}

void MainWindow::start()
{
    d_plot->start();
}
