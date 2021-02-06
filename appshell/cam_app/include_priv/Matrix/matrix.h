#ifndef MATRIX_H
#define MATRIX_H

#include <QtWidgets/QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>

class Matrix : public QWidget
{
    Q_OBJECT

public:
    Matrix(QWidget *parent = 0);
    QLabel *lb[25];
    void setBack(int matrix[25]);
};

#endif // MATRIX_H
