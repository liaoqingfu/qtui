#ifndef MAIN_API
#define MAIN_API

typedef enum
{
	RETVAL_OK = 0,
	RETVAL_ERROR = 1,
	RETVAL_EOS = 2
}
Retval;


#ifdef __cplusplus
    extern "C" {
#endif

	void setRunstate(int bRun);

    int startVideoTest();

	int main_sequence_execute();
	
	int video_capture_init(int frame_width,int frame_height,int fps);
	void video_capture_destroy(void);
	int video_capture_read(char **data_ptr,int* data_len);


	int h264_enc_process(char* buf,int nsize,char** encoded_buf);
	int h264_enc_init(int frame_width,int frame_height,int fps);
	int h264_enc_uninit(void);


	void * h264_dec_init  ( );
	Retval h264_dec_run   (void *ctx , char * , int);
	void h264_dec_uninit   (void *ctx);
	
    void ipu_para_set (int w, int h, int cx,  int cy, int cw, int ch, int rotate);


	bool gpio_open(int pin, int bOutDir);
	bool gpio_close(int pin);
	int gpio_set(int pin,int val);
	int gpio_get(int pin);



#ifdef __cplusplus
    }
#endif




#endif // MAIN_API

