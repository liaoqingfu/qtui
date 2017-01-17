#ifndef __IMX_V4L2_CAPTURE_H__
#define __IMX_V4L2_CAPTURE_H__

int video_capture_init(int frame_width,int frame_height,int ow, int oh, int fps);
void video_capture_destroy(void);
int v4l_fps_get(void);
int video_capture_read(char **data_ptr,int* data_len);



#endif

