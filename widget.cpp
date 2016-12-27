#include "widget.h"


#define NUM_X_WID       160
#define NUM_START_X     200   //1开始位置

#define NUM_Y_HEIGHT    200
char call_num_seq[13] = {'1','2','3','4','5','6','7','8','9','*','0','#','.'};

void Widget::CallTypeShow( int callTypeNum)
{
    if( callTypeNum == 0 ) {
        label_numcall->show();
        for(int i = 0; i < NUM_CALL; i++){
           bt_numcall[i] ->show();
        }
        label_pic_call->hide();
    }
    else  if( callTypeNum == 1 ) {
        label_numcall->hide();
        str_numcall = "";
        label_numcall->setText( str_numcall );
        for(int i = 0; i < NUM_CALL; i++){
           bt_numcall[i] ->hide();
        }

        label_pic_call->show();
    }
}

void  Widget::widget_init()
{
    qDebug() << "widget_init";
    label_time = new QLabel("time:");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_time->setFont( QFont(FONE_NAME, 20) );
    label_time->move(15,560);
    QGraphicsProxyWidget * proxy = scene->addWidget(label_time);
    proxy->setRotation(-90);

    label_welcome = new QLabel();
    label_welcome->setPalette(pe);
    label_welcome->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_welcome->setFont( QFont(FONE_NAME, 20) );
    label_welcome->setGeometry(10,380,300,50);
    label_welcome->setText( "IP对讲广播系统" );
    proxy = scene->addWidget(label_welcome);
    proxy->setRotation(-90);

    net_status = 0;
    label_net_status = new QLabel();
    label_net_status->setText(tr("网络连接状态       ")  );
    label_net_status->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_net_status->setPixmap(QPixmap(":/offline.bmp"));
    label_net_status->move(15,50);
    proxy = scene->addWidget(label_net_status);
    proxy->setRotation(-90);

   label_bottom_status = new QLabel();
    label_bottom_status->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_bottom_status->setFont( QFont(FONE_NAME, 20) );
    label_bottom_status->setGeometry(1024-60,560,560,50);
    proxy = scene->addWidget(label_bottom_status);
    proxy->setRotation(-90);


    label_pic_call = new QLabel();
    label_pic_call->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_pic_call->setPixmap(QPixmap(":/pic_call.bmp"));
    label_pic_call->setGeometry(60,0,950,600);
    proxy = scene->addWidget(label_pic_call);
    label_pic_call->hide();

    //number call widget init
    label_numcall = new QLabel();
    label_numcall->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_numcall->setFont( QFont(FONE_NAME, 30) );
    label_numcall->setGeometry(80,530,400,120);
    proxy = scene->addWidget(label_numcall);
    proxy->setRotation(-90);

    bg_numcall = new QButtonGroup;
    QString bg_pic;
    QPixmap pixmap;


   // for(int i = 0; i < NUM_CALL; i++){
     for(int i = 0; i < NUM_CALL; i++){
        bt_numcall[i] = new QPushButton;
        bg_pic.sprintf(":/input_dp_%i.bmp",i+1);


   /*     QPixmap p;
           QImageReader r(bg_pic);
          // r.setDecideFormatFromContent(true);
           QImage im = r.read();
          qDebug() << "errorString" <<  r.errorString();
           if (!im.isNull()){
              p = QPixmap::fromImage(im);
              qDebug() << bg_pic << "QPixmap::fromImage"<<p.width()  ;
           }
*/

         pixmap.load( bg_pic );

        int xpos = NUM_START_X + ((int)(i/3))*NUM_X_WID;
        int ypos =  530 - (i%3) * NUM_Y_HEIGHT;

        bt_numcall[i]->setGeometry( xpos, ypos,pixmap.width() ,pixmap.height());
         bt_numcall[i]->setIcon( pixmap );

        bt_numcall[i]->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
        bg_numcall->addButton(bt_numcall[i],i);

      //  qDebug() << bg_pic << "wid"<<pixmap.width() ;
        proxy = scene->addWidget(bt_numcall[i]);
        proxy->setRotation(-90);

    }

    str_numcall = "";
    call_type = 0;

    connect( bg_numcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_numcallClicked(int)));


}

void Widget::bt_numcallClicked( int buttonID)
{
    static int reachMaxLen = 0;

    if( call_type == 0) {
            int num_len = str_numcall.length();

            if ( buttonID < 13) {
                if (num_len < MAX_NUM_LEN ){
                    str_numcall = str_numcall + call_num_seq[buttonID];

                }
                else
                    //QToolTip::showText ( QPoint(60,530),(QString::fromStdString("Warning: reach max call length!")  ));
                    label_bottom_status->setText( "Warning: reach max length!" );
                    reachMaxLen = 1;
            }
            else if ( buttonID == 13) {
               qDebug() << "call to";
               CallTypeShow( (++call_type) % MAX_CALL_TYPE );
            }
            else if ( buttonID == 14) {
               if (num_len > 0 ){
                       str_numcall = str_numcall.left( num_len - 1 );
                       if( reachMaxLen ){
                            reachMaxLen = 0;
                             label_bottom_status->setText( "" );
                       }
                   }
            }

            label_numcall->setText( str_numcall );

            qDebug() <<str_numcall;
    }

}

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{

    scene = new MyScene();
   // scene->setBackgroundBrush(QPixmap(":/othermainwindowbg.jpg"));

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

    view->setScene(scene);
    view->show();

}


Widget::~Widget()
{
    //delete ui;
}
void Widget::timerEvent( QTimerEvent *event )

{

   if( event->timerId() == m_nTimerId ) {
       QTime qtimeObj = QTime::currentTime();

       QString str;
       str.sprintf("%02d:%02d:%02d",qtimeObj.hour(),qtimeObj.minute(), qtimeObj.second() );

       label_time->setText(str);
      // if( call_type == 1)
      // CallTypeShow( (++call_type) % MAX_CALL_TYPE );

   }
   if( net_status )
        label_net_status->setPixmap(QPixmap(":/online.bmp"));
   else
       label_net_status->setPixmap(QPixmap(":/offline.bmp"));

}

void Widget::paintEvent(QPaintEvent *event)
{

//    QPainter painter(this);
//    painter.setPen( QColor(0x00, 0xb2, 0xee) );//00F5FF QColor(0x00, 0xf5, 0xff)
//    painter.setBrush( QColor(0x00, 0xb2, 0xee) );
//    painter.drawRect(0,0,60,600);

//    painter.drawRect(900,0,1024,600);
//    painter.end();
    QWidget::paintEvent(event);
}
