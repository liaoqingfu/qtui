#ifndef __MISC_H__
#define __MISC_H__

#define VIDEO_PALETTE_YUV420P_MACRO		50		/* YUV 420 Planar Macro */

//#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG_PRINT(x, ...) fprintf(stdout, x, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(x, ...)
#endif
#define ERR_PRINT(x, ...) fprintf(stderr, x, ##__VA_ARGS__)

#ifndef BOOL
#define BOOL int32_t
#define FALSE 0
#define TRUE 1
#endif

typedef enum
{
	eIMAGE_CIF,
	eIMAGE_VGA,
	eIMAGE_D1,
	eIMAGE_720P,
	eIMAGE_SVGA,
	eIMAGE_QVGA,
}E_IMAGE_RESOL;

static inline void video_image_resolution_get(int image_type,unsigned int* width,unsigned int* height)
{
	switch (image_type) {
		case eIMAGE_720P:
		* width = 1280;
		* height = 720;
		break;
		case eIMAGE_SVGA:
		* width = 800;
		* height = 600;
		break;
		case eIMAGE_D1:
		* width = 720;
		* height = 576;
		break;
		case eIMAGE_VGA:
		* width = 640;
		* height = 480;
		break;
		case eIMAGE_QVGA:
		* width = 320;
		* height = 240;
		break;
		case eIMAGE_CIF:
		* width = 352;
		* height = 288;
		break;
		default:
		* width = 640;
		* height = 480;		
		break;
	}
}

static inline const char* video_image_resolution_name_get(int image_type)
{
	switch (image_type) {
		case eIMAGE_720P:
			return "720p";
		break;
		case eIMAGE_SVGA:
			return "svga";
		break;
		case eIMAGE_D1:
			return "d1";
		break;
		case eIMAGE_VGA:
			return "vga";
		break;
		case eIMAGE_QVGA:
			return "qvga";
		break;
		case eIMAGE_CIF:
			return "cif";
		break;
		default:
			return "vga";
		break;
	}
}

#endif

