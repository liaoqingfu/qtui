#include "myview.h"  
#include <QKeyEvent>  
  
MyView::MyView(QWidget *parent) :  
	QGraphicsView(parent)  
{  
    /*
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);*/
}  
  
void MyView::keyPressEvent(QKeyEvent *event)  
{  
   qDebug("*********MyView::keyPressEvent***************");  
	switch (event->key())  
	{  
	case Qt::Key_Left :  
		scale(1.2, 1.2);  
		break;	
	case Qt::Key_Right :  
		scale(1 / 1.2, 1 / 1.2);  
		break;	
	case Qt::Key_Up :  
		rotate(30);  
		break;	
	}  
	QGraphicsView::keyPressEvent(event);  
}  
  
void MyView::mousePressEvent(QMouseEvent *event)  
{  
//    qDebug("***MyView::mousePress*");
	QGraphicsView::mousePressEvent(event);	
}  
  
void MyView::paintEvent(QPaintEvent *event)  
{  

    QPainter painter(this->viewport());
    painter.setPen( QColor(0x00, 0xb2, 0xee) );//00F5FF QColor(0x00, 0xf5, 0xff)
    painter.setBrush( QColor(0x00, 0xb2, 0xee) );
    painter.drawRect(0,0,60,600);

    painter.drawRect(1024-60,0,1024,600);

     painter.end();
	QGraphicsView::paintEvent(event);  
}  
  
void MyView::mouseMoveEvent(QMouseEvent *event)  
{  
	//qDebug("************MyView::mouseMoveEvent*****************");  
	QGraphicsView::mouseMoveEvent(event);  
}  

