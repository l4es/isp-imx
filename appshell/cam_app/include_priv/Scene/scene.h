#ifndef SCENE_H
#define SCENE_H

#include <QtWidgets/QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>

class Scene : public QWidget
{
    Q_OBJECT

public:
    Scene(QWidget *parent = 0);
    QLabel *lb[25];
    void setValues(int SceneValues[25]);
    void setValues(const uint8_t (&SceneValues)[25]);
    void setValues(const uint8_t (&SceneValues)[25], const uint8_t (&ObjectRegion)[25]);
};

#endif // SCENE_H
