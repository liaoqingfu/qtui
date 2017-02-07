#ifndef MYVIEW_H  
#define MYVIEW_H  
  
#include <QGraphicsView>  
#include "myscene_main.h"
#include "myscene_num_call.h"
#include "myscene_video.h"
#include "myscene_listcall.h"  
#include "myscene_piccall.h"  
#include "myscene_calling.h"  



class MyView : public QGraphicsView  
{  
    Q_OBJECT  
public:  
	void changeWindowType(	int winType  );
	
	int WindowType ;
	int topWindowType;
	myscene_calling * scene_calling;
	myscene_main *scene_main;
    myscene_num_call *scene_num_call;
	myscene_video *scene_video;
	myscene_list * scene_list;
	myscene_pic * scene_pic;

    explicit MyView(QWidget *parent = 0);  
	~MyView( );  
  
protected:  
    void WriteSettings(QString sector, QString sItem,int value);
	void ReadAllSettings( );
	QString getLocalIp();




	void timerEvent( QTimerEvent *event );
	int m_nTimerId;
	int m_nTimerTalkId;
	void gpio_init(  );
	pthread_t pid_keyDetect;  

	
//    void keyPressEvent(QKeyEvent *event);  
//    void mousePressEvent(QMouseEvent *event);  
	 
//    void paintEvent(QPaintEvent * event);  
//    void mouseMoveEvent(QMouseEvent *event);  
signals:  
  
public slots:  
  
};  
  
#endif // MYVIEW_H 

