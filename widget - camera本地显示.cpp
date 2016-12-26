#include "widget.h"
#include "ui_widget.h"
#include  "QToolButton"
#include "QGraphicsProxyWidget"
#include "QVBoxLayout"
#include "QIcon"
#include "QPixmap"




#ifdef CAMERA_ON
void Widget::initCamera()
{
    camera=new QCamera(this);
    viewfinder=new QCameraViewfinder(view);
//    imageCapture=new QCameraImageCapture(camera);
//    horizontalLayout_View = new QHBoxLayout;
//    horizontalLayout_View->addWidget(viewfinder);
    viewfinder->setGeometry(50,50,550,400);
    camera->setViewfinder(viewfinder);
}
#endif


void  Widget::widget_init()
{
    label_time = new QLabel("time:");
    label_time->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_time->move(15,560);
    QGraphicsProxyWidget * proxy = scene->addWidget(label_time);
    proxy->setRotation(-90);

    leftCall = new QPushButton;
//    QPixmap *pixmap = NULL;
//    pixmap = new QPixmap(113, 93);
//    pixmap->load(":/../png/input_dp_0.png");
    leftCall->setIcon(QIcon(":/../png/audio.jpg"));  //input_dp_0.png
    leftCall->setIconSize(QSize(113, 93));

//    QPalette p = palette();
//    p.setBrush(QPalette::Button, QBrush(QPixmap(":/../png/input_dp_0.png")));
//    leftCall->setPalette(p);

// leftCall->setStyleSheet("background-color:transparent");
// leftCall->setText("one call");
//leftCall->setStyleSheet("QPushButton{border-image: url(:/../png/input_dp_0.png);}"  "QPushButton:pressed{border-image: url(:/../png/input_dp_0.png);}");
    leftCall->setGeometry(0,0,113,93);
    leftCall->move(100,50);
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
    //scene->addPixmap(QPixmap(":/../png/othermainwindowbg.bmp"));
    //scene->setBackgroundBrush(QPixmap(":/../png/othermainwindowbg.bmp")); //Qt::green);

    view = new MyView(parent);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setFixedSize(1024,600);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene->setSceneRect(0,0,1024,600);//(0,0,(static_cast<QWidget *>600),(static_cast<QWidget *>600) );

#ifdef CAMERA_ON
    initCamera();
#endif


    widget_init();

   // connect(leftCall, SIGNAL(clicked()), this, SLOT(showCamera()));
    bshowCamera = 0;

    m_nTimerId = startTimer(REF_TIME_MS);
    timerEvent( new QTimerEvent(m_nTimerId) ) ;

   // view->setBackgroundBrush(QPixmap(":/../png/othermainwindowbg.bmp"));
    view->setScene(scene);
    view->show();

}
#ifdef CAMERA_ON
void Widget::showCamera()
{
    if (bshowCamera == 0){
        camera->start();
        bshowCamera = 1;
        viewfinder->show();
        leftCall->setText("stop call");
    }
    else{
        camera->stop();
        bshowCamera = 0;
        leftCall->setText("start call");
        viewfinder->hide();
       // this->update();
    }
}
#endif

Widget::~Widget()
{
    //delete ui;
}
void Widget::timerEvent( QTimerEvent *event )

{

  // qDebug( "timer event, id %d,mid:%d",event->timerId() , m_nTimerId);
   if( event->timerId() == m_nTimerId ) {
       QTime qtimeObj = QTime::currentTime();
   /*    QString imgName = "border-image: url(:/pic/" + QString::number (qtimeObj.hour()/10) + ".png);";
       cout <<imgName<<endl;

       ui->label->setStyleSheet(imgName);
       imgName = "border-image: url(:/pic/" + QString::number (qtimeObj.hour()%10) + ".png);";
       ui->label_2->setStyleSheet(imgName);

       imgName = "border-image: url(:/pic/" + QString::number (qtimeObj.minute()/10) + ".png);";
       ui->label_3->setStyleSheet(imgName);

       imgName = "border-image: url(:/pic/" + QString::number (qtimeObj.minute()%10) + ".png);";
       ui->label_4->setStyleSheet(imgName);*/
       QString str;
       str.sprintf("%02d:%02d:%02d",qtimeObj.hour(),qtimeObj.minute(), qtimeObj.second() );

       label_time->setText(str);
       //label_time->setText(  QString::number ( qtimeObj.hour() ) + ":" + QString::number ( qtimeObj.minute())  + ":" + QString::number ( qtimeObj.second() ) );

   }


}



#ifdef CAMERA_ON
void Widget::captureImage()
{

    imageCapture->capture();
}

void Widget::displayImage(int , QImage image)
{
    //ui->label_Display->setPixmap(QPixmap::fromImage(image));

}
#endif
