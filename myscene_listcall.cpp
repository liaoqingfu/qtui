#include "myscene_listcall.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 


void  myscene_list::widget_init()
{
	qDebug() << "myscene_list widget_init"; 

    QGraphicsProxyWidget * proxy ;

	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
	label_back->setGeometry(100,100,102,60);
	label_back->setText("LIST CALL WINDOWS");
	this->addWidget(label_back);

	label_time = new QLabel("time:");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground); 
    label_time->setFont( QFont(FONE_NAME, 40) );
    label_time->move(180,400);
    proxy = this->addWidget(label_time);
    proxy->setRotation(-90);

	
	label_date = new QLabel("time:");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_date->setPalette(pe);
	label_date->setAttribute(Qt::WA_TranslucentBackground); 
	label_date->setFont( QFont(FONE_NAME, 25) );
	label_date->move(250,470);
	proxy = this->addWidget(label_date);
	proxy->setRotation(-90);


	m_nTimerId = startTimer(1000);  
    timerEvent( new QTimerEvent(m_nTimerId) ) ;

	QPixmap pixmap;
	pixmap.load( ":/rightCall.bmp" );

	bt_retCall = new QPushButton;
    bt_retCall->setGeometry( 900, 100,pixmap.width() ,pixmap.height());
    bt_retCall->setIcon( pixmap );
    bt_retCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    this->addWidget( bt_retCall );
	
	connect( bt_retCall ,SIGNAL(clicked( )), this, SLOT(bt_retCallClicked( )));


}

	

myscene_list::myscene_list(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

}  
	
void  myscene_list::bt_retCallClicked()
{
	pmv->changeWindowType( pmv->topWindowType);
}

void myscene_list::timerEvent( QTimerEvent *event )

{
	QTime qtimeObj = QTime::currentTime();
	static int minute = -1;
	if( pmv->WindowType != WINDOW_TYPE_PIC_CALL)
		return;
	
	if( minute != qtimeObj.minute() ){
		minute = qtimeObj.minute();
		QString str;
		str.sprintf("%02d:%02d",qtimeObj.hour(),qtimeObj.minute() );  //, qtimeObj.second()
		label_time->setText(str);

		label_date->setText(QDate::currentDate().toString(tr("yyyy-MM-dd dddd")));  
	}

} 



