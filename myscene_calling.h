#ifndef MYSCENE_CALLING_H  
#define MYSCENE_CALLING_H  
  
#include "public.h"
#include "ua_port.h"
#include "socket.h"
#include "ua_global.h"
#include "SipUA.h"
#include "SipSponServer.h"
#include "log.h"

#include "video/main_api.h"

#define CALL_IDLE  			0
#define CALL_AUDIO_OUT 		(1 << 0)
#define CALL_AUDIO_TALK 	(CALL_AUDIO_OUT << 1)
#define CALL_VIDEO_OUT 		(1 << 2)
#define CALL_VIDEO_TALK 	(CALL_VIDEO_OUT << 1)







class myscene_calling : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_calling(class MyView * pmv , QObject *parent = 0);
	int 	startCall( char * dstId );   //char * sipServerIp, char * localId, 
	void 	startCapture(  );
	int 	answerCall(  );
	int 	stopCall(  );
	void 	inCall(  );
//   static  void * dec_callback(void *);
//	static  void * capture_thread(void *);
	void 	* 	h264_dec_ctx;  //h264_dec_Context*
    CSipUA	*	sip_ua_1;
	volatile int bExitCaptureThread = 0;
	int  sip_init_once( char * sipServerIp );
	QLabel *label_calling;
    class MyView * pmv;
  	int  SipTalkType;	//0 : no talk,	1: in talking,	 2:video talk
	int    	video_init_error;//0: init success
	int 	m_nTimeoutId;


	
protected:	

  
  
signals:  



public slots:
	void  bt_stopCallClicked();

	
//	void recv_slot();

private:


    void widget_init();
	void timerEvent( QTimerEvent *event );

	QLabel *label_time;
	

    QPushButton *bt_stopCall;
	
	QLabel *label_back;
	QLabel *label_backCircle;


	QNetworkAccessManager manager;

	pthread_t pid_capture;  //camera capture thread
	int cap_pic_width, cap_pic_height;
	int crop_pic_width, crop_pic_height;

  
};	
  
#endif // MYSCENE_H  

