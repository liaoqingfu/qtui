#include "myscene.h"  
  
MyScene::MyScene(QObject *parent) :  
	QGraphicsScene(parent)	
{  
	clearFocus();  
}  
  
void MyScene::keyPressEvent(QKeyEvent *event)  
{  
    //qDebug("**MyScene::keyPress*");
	return QGraphicsScene::keyPressEvent(event);  
}  
  
void MyScene::mousePressEvent(QGraphicsSceneMouseEvent *event)	
{  
  //  qDebug("**MyScene::mousePress*");
	QGraphicsScene::mousePressEvent(event);  
}  

