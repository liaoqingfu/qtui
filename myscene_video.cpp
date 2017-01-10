#include "myscene_video.h"  
#include <stdio.h>
#include "video/main_api.h"

/*
#define SERVER_PORT  60012
#define LOCAL_PORT  60013

void myscene_video::recv_slot()
{
	while (udpSocket->hasPendingDatagrams()) {
		   QByteArray datagram;
		   datagram.resize(udpSocket->pendingDatagramSize());
		   QHostAddress sender;
		   quint16 senderPort;
	
		   udpSocket->readDatagram(datagram.data(), datagram.size(),
								   &sender, &senderPort);
	
		  qDebug() << datagram;
	   }


}
*/
void myscene_video::startVideo()
{
	if( !bShowThisWindow )
	{
		bShowThisWindow = 1;
		setRunstate( bShowThisWindow );
		startVideoTest();
		bt_stopCall->show();
	}
	else
		qDebug() << "error call startVideo";

}

void myscene_video::stopVideo()
{
	if( bShowThisWindow )
	{
		bShowThisWindow = 0;
		setRunstate( bShowThisWindow );
		bt_stopCall->hide();
	}
	else
		qDebug() << "error call stopVideo,bRun is 0";

}



void  myscene_video::widget_init()
{
	bShowThisWindow = 0;
	qDebug() << "myscene_video widget_init"; 

/*	udpSocket = new QUdpSocket;
	udpSocket->bind(QHostAddress::LocalHost,LOCAL_PORT);
	qDebug() << QHostAddress::LocalHost;
	connect(udpSocket,SIGNAL(readyRead()),this,SLOT(recv_slot()));*/

    QGraphicsProxyWidget * proxy ;

    bt_stopCall = new QPushButton;

    bt_stopCall->setGeometry( 970, 300, 50 , 80);
	bt_stopCall->setText("STOP");
    //bt_stopCall->setIcon( pixmap );
    //bt_stopCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    proxy = this->addWidget( bt_stopCall );
    proxy->setRotation(-90);
    connect( bt_stopCall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_stopCallClicked(int)));


}

void myscene_video::bt_stopCallClicked( int buttonID)
{
  	stopVideo();
}


myscene_video::myscene_video(QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();

    widget_init();

}  
  


