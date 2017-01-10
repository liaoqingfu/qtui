#ifndef MYVIEW_H  
#define MYVIEW_H  
  
#include <QGraphicsView>  
#include "myscene.h"
#include "myscene_video.h"

  
class MyView : public QGraphicsView  
{  
    Q_OBJECT  
public:  
    explicit MyView(QWidget *parent = 0);  
  
protected:  

	int      showWindowType;  //=0  main window;   1:  num call    2:list call      3: video window
    MyScene *scene_main;
	myscene_video *scene_video;

	
    void keyPressEvent(QKeyEvent *event);  
    void mousePressEvent(QMouseEvent *event);  
    void paintEvent(QPaintEvent * event);  
    void mouseMoveEvent(QMouseEvent *event);  
signals:  
  
public slots:  
  
};  
  
#endif // MYVIEW_H 

