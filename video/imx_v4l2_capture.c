
/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "imx_h264_utils.h" 


#define TEST_BUFFER_NUM 3
#define MAX_CAPTURE_MODES   10
#define DEV_CAPTURE		"/dev/video0"
		
struct capture_testbuffer {
	unsigned char *start;
	size_t offset;
	unsigned int length;
};

#ifdef H264_DECODE_LOCAL
	unsigned char yuvBuf[1024*600*2];
#endif

static int cap_fd = -1;
struct capture_testbuffer cap_buffers[TEST_BUFFER_NUM];
static int sizes_buf[MAX_CAPTURE_MODES][2];

static int getCaptureMode(int width, int height)
{
	int i, mode = -1;

	for (i = 0; i < MAX_CAPTURE_MODES; i++) {
		if (width == sizes_buf[i][0] &&
		    height == sizes_buf[i][1]) {
			mode = i;
			break;
		}
	}

	return mode;
}

int
v4l_start_capturing(void)
{
	unsigned int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof (buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (ioctl(cap_fd, VIDIOC_QUERYBUF, &buf) < 0)
		{
				printf("VIDIOC_QUERYBUF error\n");
				return -1;
		}

		cap_buffers[i].length = buf.length;
		cap_buffers[i].offset = (size_t) buf.m.offset;
		cap_buffers[i].start = mmap (NULL, cap_buffers[i].length,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			cap_fd, cap_buffers[i].offset);
		memset(cap_buffers[i].start, 0xFF, cap_buffers[i].length);
	}
	printf("lhg add:v4l2 buf.length:%d\n",buf.length);
	
	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		buf.m.offset = cap_buffers[i].offset;
		if (ioctl(cap_fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF error\n");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(cap_fd, VIDIOC_STREAMON, &type) < 0) {
		printf("VIDIOC_STREAMON error\n");
		return -1;
	}

	return 0;
}

void
v4l_stop_capturing(void)
{
	printf("v4l_stop_capturing begin -------------> \n");

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int i = 0;
	
	for (i = 0; i < TEST_BUFFER_NUM; i++) {
		if( cap_buffers[i].start > 0)
			munmap(cap_buffers[i].start,cap_buffers[i].length);
		cap_buffers[i].start = 0;
		cap_buffers[i].length = 0;
	}
	if (cap_fd > 0){
		ioctl(cap_fd, VIDIOC_STREAMOFF, &type);
		close(cap_fd);
	}
	cap_fd = -1;
	printf("v4l_stop_capturing end -------------> \n");

}

int
v4l_capture_setup(int width, int height, int ow, int oh, int fps)
{
	char v4l_device[80], node[8];
	struct v4l2_format fmt = {0};
	struct v4l2_streamparm parm = {0};
	struct v4l2_requestbuffers req = {0};
	struct v4l2_control ctl;
	struct v4l2_crop crop;
	struct v4l2_frmsizeenum fsize;
	
	printf("v4l_capture_setup begin -------------> \n");
	printf("frame_width:%d frame_height:%d fps:%d \n",width,height,fps);

	/* <input mode, 0-use csi->prp_enc->mem, 1-use csi->mem> */
	int g_input = 1 ;
	
	/*<capture mode, 0-low resolution, 1-high resolution>*/
	int mode = 0; 
	
	int chromaInterleave = 0;

	int i;
	
	if (cap_fd > 0) {
		printf("capture device already opened\n");
		return 0;
	}

	strcat(v4l_device, DEV_CAPTURE);

	if ((cap_fd = open(DEV_CAPTURE, O_RDWR, 0)) < 0) {
		printf("Unable to open %s\n", DEV_CAPTURE);
		return -1;
	}

	memset(sizes_buf, 0, sizeof(sizes_buf));
	for (i = 0; i < MAX_CAPTURE_MODES; i++) {
		fsize.index = i;
		if (ioctl(cap_fd, VIDIOC_ENUM_FRAMESIZES, &fsize))
			break;
		else {
			sizes_buf[i][0] = fsize.discrete.width;
			sizes_buf[i][1] = fsize.discrete.height;
		}
	}

	ctl.id = V4L2_CID_PRIVATE_BASE;
	ctl.value = 0;
	if (ioctl(cap_fd, VIDIOC_S_CTRL, &ctl) < 0)
	{
		printf("set control failed\n");
		goto fail;
	}

	mode = getCaptureMode(width, height);
	if (mode == -1) {
		printf("Not support the resolution in camera\n");
		goto fail;
	}
	printf("sensor frame size is %dx%d\n", sizes_buf[mode][0],
					       sizes_buf[mode][1]);

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = fps;
	parm.parm.capture.capturemode = mode;
	if (ioctl(cap_fd, VIDIOC_S_PARM, &parm) < 0) {
		printf("set frame rate failed\n");
		goto fail;
	}

	if (ioctl(cap_fd, VIDIOC_S_INPUT, &g_input) < 0) {
		printf("VIDIOC_S_INPUT failed\n");
		goto fail;
	}

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c.width = ow;
	crop.c.height = oh;
	crop.c.top = 0;
	crop.c.left = 0;
	if (ioctl(cap_fd, VIDIOC_S_CROP, &crop) < 0) {
		printf("VIDIOC_S_CROP failed\n");
		goto fail;
	}

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.bytesperline = width;
	fmt.fmt.pix.priv = 0;
	fmt.fmt.pix.sizeimage = 0;
	
	if (chromaInterleave == 0)
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	else
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	if (ioctl(cap_fd, VIDIOC_S_FMT, &fmt) < 0) {
		printf("set format failed\n");
		goto fail;
	}

	memset(&req, 0, sizeof(req));
	req.count = TEST_BUFFER_NUM;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(cap_fd, VIDIOC_REQBUFS, &req) < 0) {
		printf("v4l_capture setup: VIDIOC_REQBUFS failed\n");
		goto fail;
	}
	printf("v4l_capture setup end -------------> \n");

	return 0;
fail:
	if( cap_fd > 0)
	{
		close(cap_fd);
		cap_fd = -1;
		return -1;
	}
}

int
v4l_get_capture_data(struct v4l2_buffer *buf)
{
	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = V4L2_MEMORY_MMAP;
	if (ioctl(cap_fd, VIDIOC_DQBUF, buf) < 0) {
	//	printf("VIDIOC_DQBUF failed\n");
		return -1;
	}

	return 0;
}

void
v4l_put_capture_data(struct v4l2_buffer *buf)
{
	if (ioctl(cap_fd, VIDIOC_QBUF, buf) < 0) {
		printf("VIDIOC_QBUF failed\n");
	}
}

static unsigned long capture_calc_fps = 0;
int v4l_fps_get(void)
{
	return capture_calc_fps;
}

#define FRAME_CALC_MAX		300
int v4l_fps_clac(void)
{
	static unsigned long capture_count = 0;
	static unsigned long time_usec_start = 0;
	static unsigned long time_usec_end = 0;
	
	struct timeval total_end;
	gettimeofday(&total_end, NULL);
	
	capture_count++;
	if (capture_count == FRAME_CALC_MAX) {
		time_usec_end = (total_end.tv_sec * 1000000) + total_end.tv_usec;
		printf("--------> time_usec_end:%ld time_usec_start:%ld\n",time_usec_end,time_usec_start);

		capture_calc_fps = FRAME_CALC_MAX / ((time_usec_end-time_usec_start)/1000000);
		time_usec_start = time_usec_end;
		capture_count = 0;
		printf("--------> capture_calc_fps:%d \n",capture_calc_fps);
	}

	return capture_calc_fps;

}

int video_capture_init(int frame_width,int frame_height,int ow, int oh, int fps)
{
	capture_calc_fps = fps;
	if (v4l_capture_setup(frame_width,frame_height,ow,oh,fps) < 0) {
		printf("v4l_capture setup error \n");
		return -1;
	}
	return v4l_start_capturing();
}

void video_capture_destroy(void)
{
	v4l_stop_capturing();
}

int video_capture_read(char **data_ptr,int* data_len)
{
	static int errCount = 0;
#ifdef H264_DECODE_LOCAL_FILE
	static int bread = 0;
	if( bread <= 0 ){
		int fd = fopen("1024-600.yuv","r");
		*data_len = 0;

		if(fd > 0){
			*data_len = fread(yuvBuf, 1, sizeof(yuvBuf), fd);
			*data_ptr = yuvBuf;
			bread = *data_len ;
			fclose( fd );
		}
	}
	else{
		*data_len = bread;
		*data_ptr = yuvBuf;
	}
#else

	struct v4l2_buffer buf;
	
	if ( v4l_get_capture_data(&buf) < 0 ) {
		if( errCount++ % 1000 == 0)
			printf("capture error \n");
		*data_ptr = NULL;
		*data_len = 0;
		return 0;
	}
	
	//v4l_fps_clac();
	//fwrite(cap_buffers[buf.index].start, fmt.fmt.pix.sizeimage, 1, fd_y_file);
	
	*data_ptr = cap_buffers[buf.index].start;
	*data_len = cap_buffers[buf.index].length;

	//printf("read video : %p len:%d \n",*data_ptr,*data_len);

	v4l_put_capture_data(&buf);	

#endif
	return *data_len;
}



