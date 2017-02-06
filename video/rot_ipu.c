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


//#define IPU_TO_FB_DIRECT 1
#define  SHOW_FRAME_RATE



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
static int fb_max_len = 0;


typedef unsigned char BYTE;
 
typedef struct RGB32 {
  BYTE    rgbBlue;      // 蓝色分量
  BYTE    rgbGreen;     // 绿色分量
  BYTE    rgbRed;       // 红色分量
  BYTE    rgbReserved;  // 保留字节（用作Alpha通道或忽略）
} RGB32;


void Yuv420p2Rgb32(const BYTE *yuvBuffer_in,const BYTE *rgbBuffer_out,int width,int height)
{
    BYTE *yuvBuffer = (BYTE *)yuvBuffer_in;
    RGB32 *rgb32Buffer = (RGB32 *)rgbBuffer_out;
 	int y = 0;
	int x = 0;
    for ( y = 0; y < height; y++)
    {
        for ( x = 0; x < width; x++)
        {
            int index = y * width + x;
 
            int indexY = y * width + x;
            int indexU = width * height + y / 2 * width / 2 + x / 2;
            int indexV = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;
 
            BYTE Y = yuvBuffer[indexY];
            BYTE U = yuvBuffer[indexU];
            BYTE V = yuvBuffer[indexV];
 
            RGB32 *rgbNode = &rgb32Buffer[index];
 
            ///这转换的公式 百度有好多 下面这个效果相对好一些
 
            rgbNode->rgbRed = Y + 1.402 * (V-128);
            rgbNode->rgbGreen = Y - 0.34413 * (U-128) - 0.71414*(V-128);
            rgbNode->rgbBlue = Y + 1.772*(U-128);
        }
    }
}


unsigned int fmt_to_bpp(unsigned int pixelformat)
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
static 	void *ipuUserPaddr = NULL;   // user space p-address
static 	void  * fd_buf;
static  int   bFullScreenVideo = 0;


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
	int ret = 1, done_cnt = 0, i = 0 ;
	
	struct fb_var_screeninfo fb_var;
	struct fb_fix_screeninfo fb_fix;
	int blank;
	dma_addr_t fb_Paddr[FB_BUFS];

	


	// Cleaning the test_handle struct
	memset(&test_handle, 0, sizeof(ipu_test_handle_t));
	
	bFullScreenVideo = 0;
	// Default Settings
	t->priority = 0;
	t->task_id = 0;
	t->timeout = 1000;
	test_handle.fcount = 50;
	test_handle.loop_cnt = 1;
	t->input.width = in_wid;
	t->input.height = in_height;
	//yv12:   yvu420
	t->input.format = v4l2_fourcc('Y', 'U', '1', '2');// v4l2_fourcc('Y', 'V', '1', '2');   // v4l2_fourcc('Y', 'U', 'Y', 'V');  //v4l2_fourcc('I', '4', '2', '0'); // v4l2_fourcc('Y', 'V', '1', '2');  //v4l2_fourcc('R','G','B','P');
	t->input.crop.pos.x = 0;
	t->input.crop.pos.y = 0;
	if( (in_wid >= 1024) && (in_height >= 600) ) {
		t->input.crop.w = 1024;
		t->input.crop.h = 600;
	}
	
	t->input.deinterlace.enable = 0;
	t->input.deinterlace.motion = 0;

	if( (out_wid >= 1024) && (out_height >= 600) ) {
		bFullScreenVideo = 1;
		t->output.width = 1024;
		t->output.height = 600;
	}
	else {
		t->output.width = out_wid ;  //out_wid;   //1024;
		t->output.height = out_height ;  //out_height;  //600;
	}
	
	t->output.format = IPU_PIX_FMT_BGRA32 ; //v4l2_fourcc('Y', 'U', '1', '2'); // IPU_PIX_FMT_BGRA32    v4l2_fourcc('R','G','B','P'); //v4l2_fourcc('Y', 'V', '1', '2');  // v4l2_fourcc('Y', 'U', 'Y', 'V'); //v4l2_fourcc('Y', 'V', '1', '2'); //v4l2_fourcc('R','G','B','P');
	t->output.rotate = 0;
	t->output.crop.pos.x = 0;
	t->output.crop.pos.y = 0;
	t->output.crop.w = 0; // 0;//1024;
	t->output.crop.h = 0;  //0;//600;

	test_handle.show_to_fb = 1;

	fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
	if (fd_ipu < 0) {
		printf("open ipu dev fail\n");
		ret = 0;
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
		ret = 0;
		goto done;
	}


	// Map the IPU input buffer
	inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd_ipu, t->input.paddr);
	if (!inbuf) {
		printf("mmap fail\n");
		ret = 0;
		goto done;
	}

	if ((fd_fb = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		ret = 0;
		goto done;
	}

	if ( ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		printf("Get FB fix info failed!\n");
		ret = 0;
		goto done;
	}
	if ( ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
			printf("Get FB var info failed!\n");
			ret = 0;
			goto done;
		}
	printf("ipu t->input.width:%d, t->output.width:%d!,isize:%d\n",t->input.width , t->output.width , isize);

	for (i=0; i<FB_BUFS; i++)
		fb_Paddr[i] = fb_fix.smem_start +
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

	if( bFullScreenVideo )
		t->output.paddr = fb_Paddr[0];  //ipu direct to framebuff ,   
	else{	
		t->output.paddr = fb_max_len = fb_fix.smem_len;
		ret = ioctl(fd_ipu, IPU_ALLOC, &t->output.paddr);//input size , output phyadd
		
		if (ret < 0) {
			printf("ioctl IPU_ALLOC fail\n");
			ret = 0;
			goto done;
		}

		ipuUserPaddr = mmap(0, fb_max_len, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd_ipu, t->output.paddr);
		if (!ipuUserPaddr) {
			printf(" ipu User Paddr mmap fail\n");
			ret = 0;
			goto done;
		}

		fd_buf = mmap(NULL, fb_max_len , PROT_READ | PROT_WRITE, MAP_SHARED,
				fd_fb, 0);
		if (fd_buf == MAP_FAILED) {
			printf("fd_buf mmap failed!\n");
			ret = 0;
			goto done;
		} 
	}

	return 1;

done:
	rot_ipu_uninit();
	return ret;


}

int rot_ipu(void * raw_image, int raw_len )
{
	int ret , i;
	static int autoChangeRotate = 0;
	static int rotateMode = 0;
	
#ifdef SHOW_FRAME_RATE
	static struct timeval tv_start, tv_current;
	int total_time;
	if(autoChangeRotate % 100 == 0)
		gettimeofday(&tv_start, NULL);
#endif

	//Copy the raw image to the IPU buffer
	memcpy(inbuf, raw_image, raw_len);

//	Yuv420p2Rgb32(inbuf,fd_buf,640,480);
//	return 0;
	
	ret = ioctl(fd_ipu, IPU_QUEUE_TASK, t);
	if (ret < 0) {
		return -1;
	}
	if( !bFullScreenVideo ) // used in none-full screen mode
		for(i =0; i < t->output.height; i++)
			memcpy( ((char *)fd_buf) + i*1024*4, ipuUserPaddr + i*t->output.width*4, t->output.width*4);
	
	if(autoChangeRotate ++ % 100 == 0){
		rotateMode = (rotateMode + 5 ) % 8;
		//printf("change rotate mode :%d\n", rotateMode);
	}
	
#ifdef SHOW_FRAME_RATE
	if(autoChangeRotate % 100 == 99){
		gettimeofday(&tv_current, 0);
		total_time = (tv_current.tv_sec - tv_start.tv_sec) * 1000000L;
		total_time += tv_current.tv_usec - tv_start.tv_usec;
		printf("total time for %u frames = %u us =	%lld fps\n", 100, total_time, (100 * 1000000ULL) / total_time);
	 	
	}
	if(autoChangeRotate < 100)
		printf("r i:%d\n",autoChangeRotate);
#endif


	return 0;
}

