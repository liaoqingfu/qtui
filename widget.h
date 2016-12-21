#ifndef WIDGET_H
#define WIDGET_H


#include "public.h"
#include "myview.h"
#include "myscene.h"

#ifdef CAMERA_ON
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#endif

namespace Ui {
class Widget;
}
#define REF_TIME_MS 1000
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
void timerEvent( QTimerEvent *event );

private:

    void widget_init();


    Ui::Widget *ui;
    qreal num;
    QString filename;
    QPixmap pixmap;
    MyScene *scene;
    MyView *view;
/*        QGraphicsPixmapItem *item;
    QGraphicsScene *scene;*/
    QHBoxLayout *horizontalLayout_View;


    QLabel *label_time;
    int m_nTimerId;

    QPushButton *leftCall;
    QPushButton *rightCall;

    QLabel *label_net_status;
    int net_status;

};

#endif // WIDGET_H
