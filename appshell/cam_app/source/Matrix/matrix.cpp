#include "Matrix/matrix.h"


Matrix::Matrix(QWidget *parent)
    : QWidget(parent)
{

    int temp=5;
    int i=0;

    this->setStyleSheet("QWidget { background-color: grey; }");


    QGridLayout *grid= new QGridLayout();
    grid->setSpacing(1);

    for(int j=0; j<5; j++)
    {
        for(i=i; i<temp; i++)
        {
            lb[i] = new QLabel();
            lb[i]->resize(10,10);
            lb[i]->setStyleSheet("QLabel { background-color: white; }");
            if(i>19)
                grid->addWidget(lb[i], j,i-20);
            else if(i>14)
                grid->addWidget(lb[i], j, i-15);
            else if(i>9)
                grid->addWidget(lb[i], j, i-10);
            else if(i>4)
                grid->addWidget(lb[i], j, i-5);
            else if(i<=4)
                grid->addWidget(lb[i], j, i);

        }
        switch(j)
        {
        case 0: i=5;
                break;
        case 1: i=10;
                break;
        case 2: i=15;
                break;
        case 3: i=20;
                break;
        }

        temp+=5;


    }

    setLayout(grid);

}

void Matrix::setBack(int matrix[25])
{
    QString color;
    bool ok=false;

    for(int j=0;j<=25;j++)
    {
        for(int i=0;i<25;i++)
        {
          ok=false;
          if(matrix[i]==j)
          {
              if(j>5)
              {
                  ok=true;
                  color="black; }";
              }
              if(i%5==0&&i!=0)
              {
                  if(matrix[i-5]==j||matrix[i+5]==j||matrix[i+1]==j||matrix[i-6]==j||matrix[i+6]==j||matrix[i-4]==j||matrix[i+4]==j)
                    ok=true;
              }
              else if(i==4||i==9||i==14||i==19)
              {
                  if(matrix[i-5]==j||matrix[i+5]==j||matrix[i-1]==j||matrix[i-6]==j||matrix[i+6]==j||matrix[i-4]==j||matrix[i+4]==j)
                    ok=true;
              }
              else
              {
                  if(matrix[i-5]==j||matrix[i+5]==j||matrix[i-1]==j||matrix[i+1]==j||matrix[i-6]==j||matrix[i+6]==j||matrix[i-4]==j||matrix[i+4]==j)
                    ok=true;
              }

              if(ok==true)
              {
                  switch(j)
                  {
                  case 1: color="blue; }";
                          break;
                  case 2: color="red; }";
                          break;
                  case 3: color="yellow; }";
                          break;
                  case 4: color="green; }";
                          break;
                  case 5: color="purple; }";
                          break;
                  default: color="white; }";
                  }
                  lb[i]->setStyleSheet("QLabel{ background-color: "+color);
              }
          }

        }
    }
}
