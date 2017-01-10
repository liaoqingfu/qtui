#ifndef __IMX_H264_ENC_H__
#define __IMX_H264_ENC_H__

int h264_enc_process(char* buf,int nsize,char** encoded_buf);
int h264_enc_init(int frame_width,int frame_height,int fps);
int h264_enc_uninit(void);

#endif
