#ifndef MYSCENE_VIDEO_H  
#define MYSCENE_VIDEO_H  
  
#include "public.h"


#include <QUdpSocket> 

#define VIDEO_CAPTURE_ERR		1
#define VIDEO_H264_ENC_ERR		2
#define VIDEO_H264_DEC_ERR		4



class myscene_video : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_video(class MyView * pmv, QObject *parent = 0);
	void startVideo();
	void stopVideo();

  
protected:	

  
signals:  



public slots:
	void bt_stopCallClicked(  );
//	void recv_slot();

private:
        void widget_init();
		bool bShowVideo = false;

		QUdpSocket *udpSocket;
        class MyView * pmv;
		

        //call type 1
        QPushButton *bt_stopCall;


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
		int rotate;
  
};	
  
#endif // MYSCENE_H  

