#ifndef MYSCENE_H  
#define MYSCENE_H  
  
#include <QGraphicsScene>  
#include <QGraphicsSceneMouseEvent>  
#include <QPaintEvent>	
#include <QKeyEvent>  
#include <QPainter>

class MyScene : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
	explicit MyScene(QObject *parent = 0);	
  
protected:	
	void keyPressEvent(QKeyEvent *event);  
	void mousePressEvent(QGraphicsSceneMouseEvent *event);	

  
signals:  
  
public slots:  
  
};	
  
#endif // MYSCENE_H  

