#ifndef WIDGET_H
#define WIDGET_H


#include "public.h"
#include "myview.h"
#include "myscene.h"



namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
void timerEvent( QTimerEvent *event );

void paintEvent(QPaintEvent *event)  ;
void CallTypeShow( int callTypeNum);


public slots:
void bt_numcallClicked( int buttonID);

private:

    void widget_init();



    qreal num;
    QString filename;
    QPixmap pixmap;

    MyScene *scene;
    MyView *view;

    QHBoxLayout *horizontalLayout_View;




    int call_type;//operation mode type :拨号盘呼叫 == 1，列表呼叫 == 2， 场景呼叫 == 3

    //call type 1
    QPushButton *bt_numcall[NUM_CALL];
    QButtonGroup *bg_numcall;
    QLabel *label_numcall;
    QString str_numcall;

    QLabel *label_pic_call;

    QLabel *label_net_status;
    int net_status;

    QLabel *label_time;
    QLabel *label_bottom_status;

    QLabel *label_welcome;

    int m_nTimerId;
};

#endif // WIDGET_H
