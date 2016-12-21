#include "widget.h"
#include "ui_widget.h"
#include  "QToolButton"
#include "QGraphicsProxyWidget"
#include "QVBoxLayout"
#include "QIcon"
#include "QPixmap"




void  Widget::widget_init()
{
    label_time = new QLabel("time:");
    label_time->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_time->move(15,560);
    QGraphicsProxyWidget * proxy = scene->addWidget(label_time);
    proxy->setRotation(-90);

    proxy = scene->addWidget(leftCall);
    proxy->setRotation(90);

     qDebug() << "widget_init";

}

Widget::Widget(QWidget *parent) :
    QWidget(parent)//,
  //  ui(new Ui::Widget)
{
   // ui->setupUi(this);
    scene = new MyScene();
    scene->setBackgroundBrush(QPixmap(":/othermainwindowbg.jpg"));

    view = new MyView(parent);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setFixedSize(1024,600);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene->setSceneRect(0,0,1024,600);//(0,0,(static_cast<QWidget *>600),(static_cast<QWidget *>600) );




    widget_init();

   // connect(leftCall, SIGNAL(clicked()), this, SLOT(showCamera()));

    m_nTimerId = startTimer(REF_TIME_MS);
    timerEvent( new QTimerEvent(m_nTimerId) ) ;

   // view->setBackgroundBrush(QPixmap(":/../png/othermainwindowbg.bmp"));
    view->setScene(scene);
    view->show();

}


Widget::~Widget()
{
    //delete ui;
}
void Widget::timerEvent( QTimerEvent *event )

{

  // qDebug( "timer event, id %d,mid:%d",event->timerId() , m_nTimerId);
   if( event->timerId() == m_nTimerId ) {
       QTime qtimeObj = QTime::currentTime();

       QString str;
       str.sprintf("%02d:%02d:%02d",qtimeObj.hour(),qtimeObj.minute(), qtimeObj.second() );

       label_time->setText(str);

   }


}


