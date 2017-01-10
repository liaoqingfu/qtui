#ifndef __IMX_V4L2_CAPTURE_H__
#define __IMX_V4L2_CAPTURE_H__

int video_capture_init(int frame_width,int frame_height,int fps);
void video_capture_destroy(void);
int v4l_fps_get(void);
int video_capture_read(char **data_ptr,int* data_len);


/*
if (video_capture_init( image_width, image_height, image_fps) < 0 
||  h264_enc_init ( image_width, image_height, image_fps) < 0   ) {	
		SPON_LOG_ERR("init fail\n");
		result =  FALSE;
	}


char * yub_buf = NULL;
int yuv_len = 0;
int length = 0; 

if (video_capture_read(&yub_buf,&yuv_len) > 0) {
	
	//h264 enc
	data_len = h264_enc_process(yub_buf, yuv_len,data_ptr);   //lhg comment:  yuv --->h264 (data_ptr)
}


h264_dec_run(h264_dec_ctx , *data_ptr, data_len);


*/


#endif

