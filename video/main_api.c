
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include <sys/time.h>
#include <unistd.h>
#include "imx_v4l2_capture.h"
#include "imx_h264_enc.h"
#include "imx_h264_utils.h"

#define FIFO_H264_ENC  "/tmp/h264_enc_fifo"
#define FIFO_H264_DEC  "/tmp/h264_dec_fifo"


struct ENC_DEC_BUF{
        char * buf ;
        int len ;
};

int image_width = 1280;
int image_height = 720;
int image_fps = 25;
	
volatile int bDecRun = 0;
volatile int bEncRun = 0;
volatile int bCapRun = 0;

void setRunstate(int bRun)
{
	bDecRun = bRun;
	bEncRun = bRun;
	bCapRun = bRun;
}


void * decThread( void * args)
{
	int fd_dec;
	struct ENC_DEC_BUF h264_dec;

	h264_dec_Context* h264_dec_ctx = NULL;
	if ((h264_dec_ctx = h264_dec_init( )) == NULL)
	{
		printf("h264_dec_init fail\n");
		return -1;
	}
	
	if((mkfifo(FIFO_H264_ENC,O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))
		printf("FIFO_H264_ENC cannot create fifoserver\n");

	if((mkfifo(FIFO_H264_DEC,O_CREAT|O_EXCL)<0)&&(errno!=EEXIST))
		printf("FIFO_H264_DEC cannot create fifoserver\n");

	printf("decThread Preparing for reading bytes....\n");

	fd_dec = open(FIFO_H264_DEC,O_RDONLY,0);  //|O_NONBLOCK
	if(fd_dec < 0)
	{
		perror("FIFO_H264_DEC open read error\n");
		exit(1);
	}

	while( bDecRun ){
		if(  read(fd_dec,&h264_dec,sizeof(h264_dec) ) == sizeof(h264_dec)){
			
			//printf("decode - buf:%d, len:%d\n",h264_dec.buf, h264_dec.len);
			h264_dec_run(h264_dec_ctx , h264_dec.buf, h264_dec.len);
		}
		else{
			sleep(1);
			printf("read dec fifo errno:%d\n",errno);
		}
	}
	h264_dec_uninit( h264_dec_ctx );
	printf("decThread exit....\n");
	close(fd_dec);
	unlink(FIFO_H264_ENC);
	unlink(FIFO_H264_DEC);
	return NULL;
}


void * encThread( void * args)
{
	int fd_enc, fd_dec;	
	struct ENC_DEC_BUF h264_enc,h264_dec;
	
	if (h264_enc_init ( image_width, image_height, image_fps) < 0   ) {	
		printf("h264_enc_init fail\n");
		return -1;
	}

	printf("encThread Preparing for reading bytes....\n");

	fd_enc = open(FIFO_H264_ENC,O_RDONLY,0);  //|O_NONBLOCK
	if(fd_enc < 0)
	{
		perror("open");
		exit(1);
	}

	fd_dec = open(FIFO_H264_DEC,O_WRONLY,0);  //|O_NONBLOCK
	if(fd_dec < 0)
	{
		perror("open");
		exit(1);
	}
	
	while(bEncRun){
		if(  read(fd_enc,&h264_enc,sizeof(h264_enc) ) == sizeof(h264_enc)){

			//printf("encode - buf:%d, len:%d\n",h264_enc.buf, h264_enc.len);
			h264_dec.len = h264_enc_process(h264_enc.buf, h264_enc.len,&(h264_dec.buf));
			//printf("encode - buf:%d, len:%d\n",h264_dec.buf, h264_dec.len);
			write(fd_dec,&h264_dec,sizeof(h264_dec));
		}
		else{
			sleep(1);
			printf("read enc fifo errno:%d\n",errno);
		}
	}
	printf("encThread exit....\n");

	h264_enc_uninit( );
	pause();
	close(fd_enc);
	close(fd_dec);
	return NULL;
}


void * capThread( void *args)
{
	int fd_enc;	
	struct ENC_DEC_BUF h264_enc;
	
	if (video_capture_init( image_width, image_height, image_width, image_height,image_fps) < 0 ) {	
		printf("video_capture init fail\n");
		return -1;
	}

	printf("Preparing for capture pic....\n");

	fd_enc = open(FIFO_H264_ENC,O_WRONLY,0);  //|O_NONBLOCK
	if(fd_enc < 0)
	{
		perror("open");
		exit(1);
	}
	
	while(bCapRun){
		if (video_capture_read(&h264_enc.buf,&h264_enc.len) > 0){
			write(fd_enc,&h264_enc,sizeof(h264_enc));
			//printf("capture - buf:%d, len:%d\n",h264_enc.buf, h264_enc.len);
		}
		else{
			sleep(1);
			printf("capture errno:%d\n",errno);
		}

	}
	printf("capThread exit....\n");
	video_capture_destroy();
	
	close(fd_enc);
	return NULL;
}

int startVideoTest()
{

	pthread_t pid;
	
	if (pthread_create(&pid, NULL, decThread, NULL) < 0){
		printf("thread  create error:%s\n", strerror(errno));
		return -1;
	} 

	if (pthread_create(&pid, NULL, encThread, NULL) < 0){
		printf("thread  create error:%s\n", strerror(errno));
		return -1;
	} 

	if (pthread_create(&pid, NULL, capThread, NULL) < 0){
		printf("thread  create error:%s\n", strerror(errno));
		return -1;
	} 

	return 0;

}


//mutli-thread example
int main_api()
{
	char c;
	printf("enter S (start test), P(stop test)\n");
	setRunstate(1);
	startVideoTest();

}


void * sequence_execute( void * args)
{

	
	char * yuv_buf = NULL;
	int yuv_len = 0;
	int enc_re_len = 0; 
	char *enc_re_data_ptr = NULL;

	
	if (video_capture_init( image_width, image_height, image_width, image_height,image_fps) < 0 ) {	
		printf("video_capture _init fail\n");
		return -1;
	}
	
	if (h264_enc_init ( image_width, image_height, image_fps) < 0   ) {	
		printf("h264_enc_init fail\n");
		return -1;
	}

	h264_dec_Context* h264_dec_ctx = NULL;
	if ((h264_dec_ctx = h264_dec_init( )) == NULL)
	{
		printf("h264_dec_init fail\n");
		return -1;
	}
	while( 1 ) {
		if (video_capture_read(&yuv_buf,&yuv_len) > 0) {
			enc_re_len = h264_enc_process(yuv_buf, yuv_len,&enc_re_data_ptr);	//lhg comment:	yuv --->h264 (data_ptr)
		}
		h264_dec_run(h264_dec_ctx , enc_re_data_ptr, enc_re_len);
	}
	
}


int main_sequence_execute()
{

	pthread_t pid;
	
	if (pthread_create(&pid, NULL, sequence_execute, NULL) < 0){
		printf("sequence_execute  create error:%s\n", strerror(errno));
		return -1;
	} 
	return 0;
}






