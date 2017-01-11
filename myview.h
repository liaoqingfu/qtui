#ifndef MYVIEW_H  
#define MYVIEW_H  
  
#include <QGraphicsView>  
#include "myscene_main.h"
#include "myscene_num_call.h"
#include "myscene_video.h"

class MyView : public QGraphicsView  
{  
    Q_OBJECT  
public:  
	void changeWindowType(	int winType  );
	
	int WindowType ;
	int topWindowType;

    explicit MyView(QWidget *parent = 0);  
	~MyView( );  
  
protected:  
    void WriteSettings(QString sector, QString sItem,int value);
	void ReadAllSettings( );
	QString getLocalIp();

	myscene_main *scene_main;
    myscene_num_call *scene_num_call;
	myscene_video *scene_video;

	
    void keyPressEvent(QKeyEvent *event);  
    void mousePressEvent(QMouseEvent *event);  
	 
    void paintEvent(QPaintEvent * event);  
    void mouseMoveEvent(QMouseEvent *event);  
signals:  
  
public slots:  
  
};  
  
#endif // MYVIEW_H 

