/*
 * Copyright 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>
#include <linux/ipu.h>
#include<errno.h>

//#define VIDEO_TO_QT
#define FIFO_QT_VIDEO  "/tmp/qt_video"
static  int fd_qt;

struct QT_VIDEO_BUF{
	char * buf ;
	int width ;
	int height;
};

/*
typedef enum {

	IPU_ROTATE_NONE = 0,
	IPU_ROTATE_VERT_FLIP = 1,
	IPU_ROTATE_HORIZ_FLIP = 2,
	IPU_ROTATE_180 = 3,
	IPU_ROTATE_90_RIGHT = 4,
	IPU_ROTATE_90_RIGHT_VFLIP = 5,
	IPU_ROTATE_90_RIGHT_HFLIP = 6,
	IPU_ROTATE_90_LEFT = 7,
} ipu_rotate_mode_t;
*/

//#define  show_frame_time

typedef struct {
	struct ipu_task task;
	int fcount;
	int loop_cnt;
	int show_to_fb;
	char outfile[128];
} ipu_test_handle_t;

#define FB_BUFS 3

#define PAGE_ALIGN(x) (((x) + 4095) & ~4095)

int ctrl_c_rev = 0;

void ctrl_c_handler(int signum, siginfo_t *info, void *myact)
{
	ctrl_c_rev = 1;
}

int fd_fb_alloc = 0;

static unsigned int fmt_to_bpp(unsigned int pixelformat)
{
        unsigned int bpp;

        switch (pixelformat)
        {
                case IPU_PIX_FMT_RGB565:
                /*interleaved 422*/
                case IPU_PIX_FMT_YUYV:
                case IPU_PIX_FMT_UYVY:
                /*non-interleaved 422*/
                case IPU_PIX_FMT_YUV422P:
                case IPU_PIX_FMT_YVU422P:
                        bpp = 16;
                        break;
                case IPU_PIX_FMT_BGR24:
                case IPU_PIX_FMT_RGB24:
                case IPU_PIX_FMT_YUV444:
                case IPU_PIX_FMT_YUV444P:
                        bpp = 24;
                        break;
                case IPU_PIX_FMT_BGR32:
                case IPU_PIX_FMT_BGRA32:
                case IPU_PIX_FMT_RGB32:
                case IPU_PIX_FMT_RGBA32:
                case IPU_PIX_FMT_ABGR32:
                        bpp = 32;
                        break;
                /*non-interleaved 420*/
                case IPU_PIX_FMT_YUV420P:
                case IPU_PIX_FMT_YVU420P:
                case IPU_PIX_FMT_YUV420P2:
                case IPU_PIX_FMT_NV12:
		case IPU_PIX_FMT_TILED_NV12:
                        bpp = 12;
                        break;
                default:
                        bpp = 8;
                        break;
        }
        return bpp;
}

static  ipu_test_handle_t test_handle;
static 	struct ipu_task *t = &test_handle.task;
static  int fd_ipu = 0, fd_fb = 0;
static 	int isize = 0;
static 	void *inbuf = NULL;
static 	void  * fd_buf;


void ipu_para_set (int w, int h, int cx,        int cy, int cw, int ch, int rotate)
{
        t->output.width = w;
        t->output.height = h;
        t->output.crop.pos.x = cx;
        t->output.crop.pos.y = cy;
        t->output.crop.w = cw;
        t->output.crop.h = ch;
        t->output.rotate = rotate;

}

void rot_ipu_uninit()
{
	if (fd_fb){
		close(fd_fb);
		fd_fb = 0;
	}
#ifdef VIDEO_TO_QT
	if (t->output.paddr){
		free(t->output.paddr);
		t->output.paddr = 0;
	}
#else
	if (t->output.paddr){
		ioctl(fd_ipu, IPU_FREE, &t->output.paddr);
		t->output.paddr = 0;
	}
#endif

	if (inbuf){
		munmap(inbuf, isize);
		inbuf = 0;
	}
	if ( fd_buf ) {
		munmap(fd_buf, isize);
		fd_buf = 0;
	}
	
	if (t->input.paddr){
		ioctl(fd_ipu, IPU_FREE, &t->input.paddr);
		t->input.paddr = 0;
	}
	if (fd_ipu){
		close(fd_ipu);
		fd_ipu = 0;
	}

}
int rot_ipu_init( int in_wid, int in_height , int out_wid, int out_height )
{
	int ret = 0, done_cnt = 0, i = 0 ;
	
	dma_addr_t outpaddr[FB_BUFS];
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	int blank;

	


	// Cleaning the test_handle struct
	memset(&test_handle, 0, sizeof(ipu_test_handle_t));

	// Default Settings
	t->priority = 0;
	t->task_id = 0;
	t->timeout = 1000;
	test_handle.fcount = 50;
	test_handle.loop_cnt = 1;
	t->input.width = in_wid;
	t->input.height = in_height;
	t->input.format = v4l2_fourcc('Y', 'V', '1', '2');   // v4l2_fourcc('Y', 'U', 'Y', 'V');  //v4l2_fourcc('I', '4', '2', '0'); // v4l2_fourcc('Y', 'V', '1', '2');  //v4l2_fourcc('R','G','B','P');
	t->input.crop.pos.x = 0;
	t->input.crop.pos.y = 0;
	//t->input.crop.w = 1024;
	//t->input.crop.h = 600;
	t->input.deinterlace.enable = 0;
	t->input.deinterlace.motion = 0;

	t->output.width = 1024;  //out_wid;   //1024;
	t->output.height = 600;  //out_height;  //600;
	t->output.format = v4l2_fourcc('Y', 'V', '1', '2');   // v4l2_fourcc('Y', 'U', 'Y', 'V'); //v4l2_fourcc('Y', 'V', '1', '2'); //v4l2_fourcc('R','G','B','P');
	t->output.rotate = 0;
	t->output.crop.pos.x = 0;
	t->output.crop.pos.y = 0;
	t->output.crop.w = out_wid;
	t->output.crop.h = out_height;

	test_handle.show_to_fb = 1;

	fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
	if (fd_ipu < 0) {
		printf("open ipu dev fail\n");
		ret = -1;
		goto done;
	}

	if (IPU_PIX_FMT_TILED_NV12F == t->input.format) {
		isize = PAGE_ALIGN(t->input.width * t->input.height/2) +
			PAGE_ALIGN(t->input.width * t->input.height/4);
		isize = t->input.paddr = isize * 2;
	} else
		isize = t->input.paddr =
			t->input.width * t->input.height
			* fmt_to_bpp(t->input.format)/8;
	ret = ioctl(fd_ipu, IPU_ALLOC, &t->input.paddr);//input size , output phyadd
	
	if (ret < 0) {
		printf("ioctl IPU_ALLOC fail\n");
		goto done;
	}

	// Map the IPU input buffer
	inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd_ipu, t->input.paddr);
	if (!inbuf) {
		printf("mmap fail\n");
		ret = -1;
		goto done;
	}

	if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = -1;
		goto done;
	}

	if ( ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		printf("Get FB fix info failed!\n");
		ret = -1;
		goto done;
	}
	if ( ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
			printf("Get FB var info failed!\n");
			ret = -1;
			goto done;
		}
	
	ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
	ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);


	//printf("enter crop input width, height\n");
	//scanf("%d %d",&(t->input.crop.w),&(t->input.crop.h) );


	//printf("enter crop output crop x,y,w,h\n");
	//scanf("%d %d %d %d",&(t->output.crop.pos.x),&(t->output.crop.pos.y),&(t->output.crop.w),&(t->output.crop.h) );

	printf("ipu t->input.crop.w:%d, t->input.crop.h:%d!,isize:%d\n",t->input.crop.w , t->input.crop.h , isize);

	for (i=0; i<FB_BUFS; i++)
		outpaddr[i] = fb_fix.smem_start +
			i * fb_var.yres * fb_fix.line_length;

	// Unblank the display
	blank = FB_BLANK_UNBLANK;
	ioctl(fd_fb, FBIOBLANK, blank);

	if( t->input.crop.w != 0 && t->input.crop.h!= 0){
		i = t->input.crop.w*t->input.crop.h*3/2;
		t->output.width = t->input.crop.w;
		t->output.height = t->input.crop.h;
	}
	else
		i = t->output.width*t->output.height*3/2;
/*
	fd_buf = mmap(NULL, i , PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_fb, 0);
	if (fd_buf == MAP_FAILED) {
		printf("fd_buf mmap failed!\n");
		ret = -1;
		goto done;
	}*/
	
	t->output.paddr = outpaddr[done_cnt % FB_BUFS];
	
	return 0;

done:
	rot_ipu_uninit();
	return -1;


}

int rot_ipu(void * raw_image, int raw_len )
{
	int ret;
	static int autoChangeRotate = 0;
	static int rotateMode = 0;
	
#ifdef show_frame_time
	static struct timeval tv_start, tv_current;
	int total_time;
	if(autoChangeRotate % 100 == 0)
		gettimeofday(&tv_start, NULL);
#endif
	
	//Copy the raw image to the IPU buffer
	memcpy(inbuf, raw_image, raw_len);
	
	ret = ioctl(fd_ipu, IPU_QUEUE_TASK, t);
	if (ret < 0) {
		return -1;
	}
	
	#ifdef VIDEO_TO_QT
	if( (fd_qt > 0) && (autoChangeRotate == 0) )
		write(fd_qt,&t->output.paddr,sizeof(t->output.paddr));
	
	#endif
	
	if(autoChangeRotate ++ % 100 == 0){
		rotateMode = (rotateMode + 5 ) % 8;
		//printf("change rotate mode :%d\n", rotateMode);
	}
	
#ifdef show_frame_time
	if(autoChangeRotate % 100 == 99){
		gettimeofday(&tv_current, 0);
		total_time = (tv_current.tv_sec - tv_start.tv_sec) * 1000000L;
		total_time += tv_current.tv_usec - tv_start.tv_usec;
		printf("total time for %u frames = %u us =	%lld fps\n", 100, total_time, (100 * 1000000ULL) / total_time);
	 	
	}
#endif


	return 0;
}

