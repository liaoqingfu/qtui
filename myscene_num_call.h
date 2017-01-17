#ifndef MYSCENE_NUM_CALL_H  
#define MYSCENE_NUM_CALL_H  
  
#include "public.h"


#include <QUdpSocket> 

#define VIDEO_CAPTURE_ERR		1
#define VIDEO_H264_ENC_ERR		2
#define VIDEO_H264_DEC_ERR		4



class myscene_num_call : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_num_call(class MyView * pmv,QObject *parent = 0);

  
protected:	
//	void keyPressEvent(QKeyEvent *event);  
//	void mousePressEvent(QGraphicsSceneMouseEvent *event);	
    void timerEvent( QTimerEvent *event );

    //void CallTypeShow( int callTypeNum);
  
signals:  



    public slots:
    void bt_numcallClicked( int buttonID);
    void recv_slot();

    private:

        void widget_init();

        class MyView * pmv;

		QUdpSocket *udpSocket;


        qreal num;
        QString filename;
        QPixmap pixmap;



        QHBoxLayout *horizontalLayout_View;



        //call type 1
        QPushButton *bt_numcall[NUM_CALL];
        QButtonGroup *bg_numcall;
        QLabel *label_numcall;
        QString str_numcall;


        QLabel *label_net_status;
        int net_status;

        QLabel *label_time;
        QLabel *label_bottom_status;

        QLabel *label_welcome;

        int m_nTimerId;

/*
		int video_init_err;
		int image_width = 1280;
		int image_height = 720;
		int image_fps = 25;

		char * yuv_buf = NULL;  //camera return 
		int yuv_len = 0;
		int enc_re_len = 0; 
		char *enc_re_data_ptr = NULL;

		void * h264_dec_ctx = NULL;

		int output_w ,output_h;
		int crop_x,crop_y,crop_w,crop_h;
		int rotate;*/
  
};	
  
#endif // MYSCENE_H  

