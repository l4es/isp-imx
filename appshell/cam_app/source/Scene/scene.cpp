#include "Scene/scene.h"


Scene::Scene(QWidget *parent)
    : QWidget(parent)
{

    int temp=5;
    int i=0;

    //this->setStyleSheet("QWidget { background-color: grey; }");
    QPalette pal = this->palette();
    pal.setColor(this->backgroundRole(),
            Qt::gray);
    this->setPalette( pal );


    QGridLayout *grid= new QGridLayout();
    grid->setSpacing(1);

    for(int j=0; j<5; j++)
    {
        for(i=i; i<temp; i++)
        {
            lb[i] = new QLabel();
            lb[i]->resize(10,10);
            //lb[i]->setStyleSheet("QLabel { background-color: white; }");
            QPalette pal = lb[i]->palette();
            pal.setColor(lb[i]->backgroundRole(),
                    Qt::white);
            pal.setColor(lb[i]->foregroundRole(),
                    Qt::green);
            lb[i]->setPalette( pal );
            lb[i]->setAutoFillBackground(true);
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

void Scene::setValues(int SceneValues[25])
{

    QString text_int;

    for(int i=0;i<=24;i++)
    {
        Scene::lb[i]->setText(text_int.setNum(SceneValues[i]));
    }
}

void Scene::setValues(const uint8_t (&SceneValues)[25])
{

    QString text_int;

    for(int i=0;i<=24;i++)
    {
        Scene::lb[i]->setText(text_int.setNum(SceneValues[i]));

        QPalette pal = Scene::lb[i]->palette();
        pal.setColor(Scene::lb[i]->backgroundRole(),
                QColor(SceneValues[i], SceneValues[i], SceneValues[i]));
        pal.setColor(Scene::lb[i]->foregroundRole(),
                Qt::green);
        Scene::lb[i]->setPalette( pal );
    }
}

void Scene::setValues(const uint8_t (&SceneValues)[25], const uint8_t (&ObjectRegion)[25])
{

    QString text_int;

    for(int i=0;i<=24;i++)
    {
        Scene::lb[i]->setText(text_int.setNum(SceneValues[i]));

        QPalette pal = Scene::lb[i]->palette();
        pal.setColor(Scene::lb[i]->backgroundRole(),
                QColor(SceneValues[i], SceneValues[i], SceneValues[i]));
        pal.setColor(Scene::lb[i]->foregroundRole(),
                ( !ObjectRegion[i] ) ? Qt::red : Qt::green );
        Scene::lb[i]->setPalette( pal );
    }
}


