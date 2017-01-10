/* example for how to use the imxvpuapi decoder interface
 * Copyright (C) 2014 Carlos Rafael Giani
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imx_h264_utils.h"
#include <linux/mxcfb.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "vpu_lib.h"
#include "imxvpuapi.h"


#define  SIMULATE_DECODE
//#define  DECODE_DIRECT_TO_FB     

//#define  h264_decode_saveFile

#define FB_INIT   0
#define FB_FRESH  1
#define FB_CLOSE  2

extern int  rot_ipu_init( int in_wid, int in_height , int out_wid, int out_height );
extern int  rot_ipu(void * raw_image, int  );
extern void rot_ipu_uninit();


/* This is a simple example of how to decode with the imxvpuapi library.
 * It decodes h.264 byte stream data and dumps the decoded frames to a file.
 * Also look into imxvpuapi.h for documentation. */

static char * ph264Buf;
static int    h264Len;
#define FB_BUFS 3
void * outpaddr[FB_BUFS];
static int screensize = 0;
//static int ipu_input_crop_w = 0, ipu_input_crop_h = 0;   //lhg

#ifdef h264_decode_saveFile
	static  FILE * fd_y_file1 = 0;
	static 	FILE * fd_y_file2 = 0;
	static 	FILE * fd_y_file3 = 0;
#endif



#ifdef DECODE_DIRECT_TO_FB 
int fb_out( int operation)  //0:init   1:refresh   2 : close
{
	static int fd_fb0 = -1 ;
	static struct fb_var_screeninfo fb_var;
	
	int  ret = 0;
	
	struct fb_fix_screeninfo fb_fix;

	if( (operation == FB_INIT ) && (fd_fb0 == -1) )
	{
		
		
		if ((fd_fb0 = open("/dev/fb0", O_RDWR, 0)) < 0) {
			printf("Unable to open /dev/fb0\n");
			ret = -1;
			goto done;
		}

		if ( ioctl(fd_fb0, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
			printf("Get FB fix info failed!\n");
			ret = -1;
			goto done;
		}

		if ( ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb_var) < 0) {
			printf("Get FB var info failed!\n");
			ret = -1;
			goto done;
		}
		
		//fb_var.xres = 1024;
		fb_var.xres_virtual = fb_var.xres;
		//fb_var.yres = 600;
		fb_var.yres_virtual = fb_var.yres;
		/*fb_var.activate |= FB_ACTIVATE_FORCE;
		fb_var.vmode |= FB_VMODE_YWRAP;
		fb_var.nonstd = t->output.format;
		fb_var.bits_per_pixel = fmt_to_bpp(t->output.format);
		*/
		ret = ioctl(fd_fb0, FBIOPUT_VSCREENINFO, &fb_var);
		if (ret < 0) {
			printf("fb ioctl FBIOPUT_VSCREENINFO fail\n");
			goto done;
		}

		ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb_var);
		ioctl(fd_fb0, FBIOGET_FSCREENINFO, &fb_fix);

		printf("fb_var.xres:%d £¬ fb_var.yres:%d , fb_fix.line_length:%d", fb_var.xres, fb_var.yres, fb_fix.line_length);
		screensize = fb_var.xres * fb_var.yres *3/2;
		outpaddr[0] = mmap(NULL,screensize,PROT_READ|PROT_WRITE,MAP_SHARED,fd_fb0,0);
		/*
		for (i=0; i<FB_BUFS; i++)
			outpaddr[i] = fb_fix.smem_start +
				i * fb_var.yres * fb_fix.line_length;*/
		#ifdef h264_decode_saveFile
			if ((fd_y_file1 = fopen("0.yuv", "wb")) ==NULL)
	        {
	                printf("test program:Unable to create y frame recording file\n");
	                return -1;
	        }
			if ((fd_y_file2 = fopen("90.yuv", "wb")) ==NULL)
	        {
	                printf("test program:Unable to create y frame recording file\n");
	                return -1;
	        }
			if ((fd_y_file3 = fopen("180.yuv", "wb")) ==NULL)
			{
					printf("test program:Unable to create y frame recording file\n");
					return -1;
			}
		#endif
	}
	else if( operation == FB_FRESH){
	//display
		ret = ioctl(fd_fb0, FBIOPAN_DISPLAY, &fb_var);
		if (ret < 0) {
			printf("fb ioct FBIOPAN_DISPLAY fail\n");
		}
	}
	else if (operation == FB_CLOSE){
		if(fd_fb0 >= 0)
			close(fd_fb0);
		fd_fb0 = -1;
	}
	
	done:
		return ret;

}
#endif


#ifndef BOOL
#define BOOL int
#endif


typedef enum
{
	FrameMode_Free,
	FrameMode_ReservedForDecoding,
	FrameMode_ContainsDisplayableFrame
}
FrameMode;


typedef struct
{
	void *context;
	uint64_t pts, dts;
	ImxVpuFrameType frame_types[2];
	ImxVpuInterlacingMode interlacing_mode;
	FrameMode mode;
}
ImxVpuDecFrameEntry;


struct _ImxVpuDecoder
{
	DecHandle handle;

	ImxVpuDMABuffer *bitstream_buffer;
	uint8_t *bitstream_buffer_virtual_address;
	imx_vpu_phys_addr_t bitstream_buffer_physical_address;

	uint8_t const *codec_data;
	size_t codec_data_size;

	ImxVpuCodecFormat codec_format;
	unsigned int frame_width, frame_height;

	unsigned int old_jpeg_width, old_jpeg_height;
	ImxVpuColorFormat old_jpeg_color_format;

	unsigned int num_framebuffers, num_used_framebuffers;
	/* internal_framebuffers and framebuffers are separate from
	 * frame_entries: internal_framebuffers must be given directly
	 * to the vpu_DecRegisterFrameBuffer() function, and framebuffers
	 * is a user-supplied input value */
	FrameBuffer *internal_framebuffers;
	ImxVpuFramebuffer *framebuffers;
	ImxVpuDecFrameEntry *frame_entries;
	ImxVpuDecFrameEntry dropped_frame_entry;

	BOOL main_header_pushed;

	BOOL drain_mode_enabled;
	BOOL drain_eos_sent_to_vpu;

	DecInitialInfo initial_info;
	BOOL initial_info_available;

	DecOutputInfo dec_output_info;
	int available_decoded_frame_idx;

	imx_vpu_dec_new_initial_info_callback initial_info_callback;
	void *callback_user_data;
};

typedef struct _ImxVpuDecoder ImxVpuDecoder;

struct h264_decode_Context
{
	FILE *fin, *fout;

	h264_context h264_ctx;

	ImxVpuDecoder *vpudec;

	ImxVpuDMABuffer *bitstream_buffer;
	size_t bitstream_buffer_size;
	unsigned int bitstream_buffer_alignment;

	ImxVpuDecInitialInfo initial_info;

	ImxVpuFramebuffer *framebuffers;
	ImxVpuDMABuffer **fb_dmabuffers;
	unsigned int num_framebuffers;
	ImxVpuFramebufferSizes calculated_sizes;

	unsigned int frame_id_counter;

	ImxVpuDecOpenParams open_params;
};


ImxVpuFramebuffer *rotate_framebuffers;
ImxVpuDMABuffer *rotate_fb_dmabuffers;
FrameBuffer *rotate_internal_fb;

int initial_info_callback(ImxVpuDecoder *decoder, ImxVpuDecInitialInfo *new_initial_info, unsigned int output_code, void *user_data)
{
	unsigned int i;
	h264_dec_Context *ctx = (h264_dec_Context *)user_data;

	((void)(decoder));
	((void)(output_code));


	/* Keep a copy of the initial information around */
	ctx->initial_info = *new_initial_info;


	fprintf(
		stderr,
		"initial info:  size: %ux%u pixel  rate: %u/%u  min num required framebuffers: %u  interlacing: %d  framebuffer alignment: %u\n",
		ctx->initial_info.frame_width,
		ctx->initial_info.frame_height,
		ctx->initial_info.frame_rate_numerator,
		ctx->initial_info.frame_rate_denominator,
		ctx->initial_info.min_num_required_framebuffers,
		ctx->initial_info.interlacing,
		ctx->initial_info.framebuffer_alignment
	);

#ifdef DECODE_DIRECT_TO_FB 
	fb_out( FB_INIT );  //lhg direct output 
#else
	rot_ipu_init(ctx->initial_info.frame_width,	ctx->initial_info.frame_height,ctx->initial_info.frame_width,	ctx->initial_info.frame_height);     //ipu rotate, then output
#endif
	ctx->num_framebuffers = ctx->initial_info.min_num_required_framebuffers;

	/* Using the initial information, calculate appropriate framebuffer sizes */
	imx_vpu_calc_framebuffer_sizes(
		ctx->initial_info.color_format,
		ctx->initial_info.frame_width,
		ctx->initial_info.frame_height,
		ctx->initial_info.framebuffer_alignment,
		ctx->initial_info.interlacing,
		0,
		&(ctx->calculated_sizes)
	);

	fprintf(
		stderr,
		"calculated sizes:  frame width&height: %dx%d  Y stride: %u  CbCr stride: %u  Y size: %u  CbCr size: %u  MvCol size: %u  total size: %u,ctx->initial_info.color_format:%d\n",
		ctx->calculated_sizes.aligned_frame_width, ctx->calculated_sizes.aligned_frame_height,
		ctx->calculated_sizes.y_stride, ctx->calculated_sizes.cbcr_stride,
		ctx->calculated_sizes.y_size, ctx->calculated_sizes.cbcr_size, ctx->calculated_sizes.mvcol_size,
		ctx->calculated_sizes.total_size,ctx->initial_info.color_format
	);


	/* If any framebuffers were allocated previously, deallocate them now.
	 * This can happen when video sequence parameters change, for example. */

	if (ctx->framebuffers != NULL)
		free(ctx->framebuffers);

	if (ctx->fb_dmabuffers != NULL)
	{
		for (i = 0; i < ctx->num_framebuffers; ++i)
			imx_vpu_dma_buffer_deallocate(ctx->fb_dmabuffers[i]);
		free(ctx->fb_dmabuffers);
	}


	/* Allocate memory blocks for the framebuffer and DMA buffer structures,
	 * and allocate the DMA buffers themselves */

	ctx->framebuffers = malloc(sizeof(ImxVpuFramebuffer) * ctx->num_framebuffers);
	ctx->fb_dmabuffers = malloc(sizeof(ImxVpuDMABuffer*) * ctx->num_framebuffers);

//lhg add extra rotate buffer

/*
	rotate_framebuffers = malloc(sizeof(ImxVpuFramebuffer) );
	rotate_fb_dmabuffers = imx_vpu_dma_buffer_allocate(
			imx_vpu_dec_get_default_allocator(),
			ctx->calculated_sizes.total_size,
			ctx->initial_info.framebuffer_alignment,
			0
		);
		imx_vpu_fill_framebuffer_params(
			rotate_framebuffers,
			&(ctx->calculated_sizes),
			rotate_fb_dmabuffers,
			(void*)((uintptr_t)(0x2000 + 10))
		);
		imx_vpu_phys_addr_t phys_addr;
		rotate_internal_fb = IMX_VPU_ALLOC(sizeof(FrameBuffer) );
		memset(rotate_internal_fb, 0, sizeof(FrameBuffer) );

		phys_addr = imx_vpu_dma_buffer_get_physical_address(rotate_framebuffers->dma_buffer);
		if (phys_addr == 0)
		{
			IMX_VPU_ERROR("could not map buffer %u", i );
		}

		rotate_framebuffers->already_marked = 1;
		rotate_framebuffers->internal = (void*)i; 

		rotate_internal_fb->strideY = rotate_framebuffers->y_stride;
		rotate_internal_fb->strideC = rotate_framebuffers->cbcr_stride;
		rotate_internal_fb->myIndex = 10;
		rotate_internal_fb->bufY = (PhysicalAddress)(phys_addr + rotate_framebuffers->y_offset);
		rotate_internal_fb->bufCb = (PhysicalAddress)(phys_addr + rotate_framebuffers->cb_offset);
		rotate_internal_fb->bufCr = (PhysicalAddress)(phys_addr + rotate_framebuffers->cr_offset);
		rotate_internal_fb->bufMvCol = (PhysicalAddress)(phys_addr + rotate_framebuffers->mvcol_offset);*/
//lhg add extra rotate buffer end

		

	for (i = 0; i < ctx->num_framebuffers; ++i)
	{
		/* Allocate a DMA buffer for each framebuffer. It is possible to specify alternate allocators;
		 * all that is required is that the allocator provides physically contiguous memory
		 * (necessary for DMA transfers) and respecs the alignment value. */
		ctx->fb_dmabuffers[i] = imx_vpu_dma_buffer_allocate(
			imx_vpu_dec_get_default_allocator(),
			ctx->calculated_sizes.total_size,
			ctx->initial_info.framebuffer_alignment,
			0
		);

		/* The last parameter (the one with 0x2000 + i) is the context data for the framebuffers in the pool.
		 * It is possible to attach user-defined context data to them. Note that it is not related to the
		 * context data in en- and decoded frames. For purposes of demonstrations, the context pointer
		 * is just a simple monotonically increasing integer. First framebuffer has context 0x2000, second 0x2001 etc. */
		imx_vpu_fill_framebuffer_params(
			&(ctx->framebuffers[i]),
			&(ctx->calculated_sizes),
			ctx->fb_dmabuffers[i],
			(void*)((uintptr_t)(0x2000 + i))
		);
	}


	/* Actual registration is done here. From this moment on, the VPU knows which buffers to use for
	 * storing decoded raw frames into. This call must not be done again until decoding is shut down or
	 * IMX_VPU_DEC_OUTPUT_CODE_INITIAL_INFO_AVAILABLE is set again. */
	imx_vpu_dec_register_framebuffers(ctx->vpudec, ctx->framebuffers, ctx->num_framebuffers);


	return 1;
}


h264_dec_Context* h264_dec_init( )
{
	h264_dec_Context *ctx;

	ctx = calloc(1, sizeof(h264_dec_Context));
	ctx->fin = NULL;
	ctx->fout = NULL;
	ctx->frame_id_counter = 100;
	ph264Buf = NULL;
	h264Len  = 0;
	
	h264_ctx_init(&(ctx->h264_ctx) );

	/* Set the open params. Enable frame reordering, use h.264 as the codec format.
	 * The memset() call ensures the other values are set to their default. */
	memset(&(ctx->open_params), 0, sizeof(ImxVpuDecOpenParams));
	ctx->open_params.codec_format = IMX_VPU_CODEC_FORMAT_H264;
	ctx->open_params.enable_frame_reordering = 1;

	/* Load the VPU firmware */
	imx_vpu_dec_load();

	/* Retrieve information about the required bitstream buffer and allocate one based on this */
	imx_vpu_dec_get_bitstream_buffer_info(&(ctx->bitstream_buffer_size), &(ctx->bitstream_buffer_alignment));
	ctx->bitstream_buffer = imx_vpu_dma_buffer_allocate(
		imx_vpu_dec_get_default_allocator(),
		ctx->bitstream_buffer_size,
		ctx->bitstream_buffer_alignment,
		0
	);

	/* Open a decoder instance, using the previously allocated bitstream buffer */
	imx_vpu_dec_open(&(ctx->vpudec), &(ctx->open_params), ctx->bitstream_buffer, initial_info_callback, ctx);

	return ctx;
}


static int h264_decode_frame(h264_dec_Context *ctx)
{
	ImxVpuEncodedFrame encoded_frame;
	int ok;
	unsigned int output_code;
	ImxVpuDecReturnCodes ret;
	char decodeFileName[128];
	

	if (imx_vpu_dec_is_drain_mode_enabled(ctx->vpudec))
	{
		/* In drain mode there is no input data */
		encoded_frame.data = NULL;
		encoded_frame.data_size = 0;
		encoded_frame.context = NULL;

		imx_vpu_dec_set_codec_data(ctx->vpudec, NULL, 0);

		ok = 1;
	}
	else
	{
		/* Regular mode; read input data and feed it to the decoder */
		#ifdef  SIMULATE_DECODE
			encoded_frame.data = (uint8_t *)ph264Buf;
			encoded_frame.data_size = h264Len;
			
			/*
			static int decodeFrameNum = 0;
			static int rot_angle = 0 , rot_stride;

			if( (decodeFrameNum++ % 100)  >=  3){

				vpu_DecGiveCommand(ctx->vpudec->handle, SET_ROTATION_ANGLE,	&rot_angle);


				rot_stride = (rot_angle == 90 || rot_angle == 270) ? ctx->initial_info.frame_height: ctx->initial_info.frame_width;

				vpu_DecGiveCommand(ctx->vpudec->handle, SET_ROTATOR_OUTPUT, (void *)ctx->vpudec->internal_framebuffers );  //(void *)(rotate_internal_fb)
				
				vpu_DecGiveCommand( ctx->vpudec->handle, SET_ROTATOR_STRIDE, &rot_stride);

				vpu_DecGiveCommand(ctx->vpudec->handle,	ENABLE_ROTATION, 0);



				vpu_DecGiveCommand(ctx->vpudec->handle, SET_ROTATION_ANGLE,	&rot_angle);


				rot_stride = (rot_angle == 90 || rot_angle == 270) ? ctx->initial_info.frame_height: ctx->initial_info.frame_width;

				vpu_DecGiveCommand( ctx->vpudec->handle, SET_ROTATOR_STRIDE, &rot_stride);

				vpu_DecGiveCommand(ctx->vpudec->handle,	ENABLE_ROTATION, 0);

				rot_angle = (rot_angle +90) %360;
			}*/
		#else
			ok = h264_ctx_read_access_unit(&(ctx->h264_ctx) , ph264Buf , h264Len);
			if(ok == 0)
				return RETVAL_ERROR;

			if ((ctx->h264_ctx.au_end_offset <= ctx->h264_ctx.au_start_offset) )
				return RETVAL_EOS;

			encoded_frame.data = ctx->h264_ctx.in_buffer + ctx->h264_ctx.au_start_offset;
			encoded_frame.data_size = ctx->h264_ctx.au_end_offset - ctx->h264_ctx.au_start_offset;
		#endif
		/* Codec data is out-of-band data that is typically stored in a separate space in
		 * containers for each elementary stream; h.264 byte-stream does not need it */
		imx_vpu_dec_set_codec_data(ctx->vpudec, NULL, 0);
		/* The frame id counter is used to give the encoded frames an example context.
		 * The context of an encoded frame is a user-defined pointer that is passed along
		 * to the corresponding decoded raw frame. This way, it can be determined which
		 * decoded raw frame is the result of what encoded frame.
		 * For example purposes (to be able to print some log output), the context
		 * is just a monotonically increasing integer. */
		encoded_frame.context = (void *)((uintptr_t)(ctx->frame_id_counter));

		/* Not using the PTS/DTS values in this example */
		encoded_frame.pts = 0;
		encoded_frame.dts = 0;

		//fprintf(stderr, "encode id: 0x%x,%u b\n", ctx->frame_id_counter, encoded_frame.data_size);
	}

	/* Perform the actual decoding */
	if ((ret = imx_vpu_dec_decode(ctx->vpudec, &encoded_frame, &output_code)) != IMX_VPU_DEC_RETURN_CODE_OK)
	{
		fprintf(stderr, "imx_vpu_dec_decode() failed: %s\n", imx_vpu_dec_error_string(ret));
		return RETVAL_ERROR;
	}

	if (output_code & IMX_VPU_DEC_OUTPUT_CODE_VIDEO_PARAMS_CHANGED)
	{
		/* Video sequence parameters changed. Decoding cannot continue with the
		 * existing decoder. Drain it, and open a new one to resume decoding. */

		imx_vpu_dec_enable_drain_mode(ctx->vpudec, 1);

		for (;;)
		{
			Retval rret = h264_decode_frame(ctx);
			if (rret == RETVAL_EOS)
				break;
			else if (rret == RETVAL_ERROR)
				return RETVAL_ERROR;
		}

		imx_vpu_dec_enable_drain_mode(ctx->vpudec, 0);

		imx_vpu_dec_close(ctx->vpudec);

		imx_vpu_dec_open(&(ctx->vpudec), &(ctx->open_params), ctx->bitstream_buffer, initial_info_callback, ctx);

		/* Feed the data that caused the IMX_VPU_DEC_OUTPUT_CODE_VIDEO_PARAMS_CHANGED
		 * output code flag again, but this time into the new decoder */
		if ((ret = imx_vpu_dec_decode(ctx->vpudec, &encoded_frame, &output_code)) != IMX_VPU_DEC_RETURN_CODE_OK)
		{
			fprintf(stderr, "imx_vpu_dec_decode() failed: %s\n", imx_vpu_dec_error_string(ret));
			return RETVAL_ERROR;
		}
	}

	if (output_code & IMX_VPU_DEC_OUTPUT_CODE_DECODED_FRAME_AVAILABLE)
	{
		/* A decoded raw frame is available for further processing. Retrieve it, do something
		 * with it, and once the raw frame is no longer needed, mark it as displayed. This
		 * marks it internally as available for further decoding by the VPU. */

		ImxVpuRawFrame decoded_frame;
		//unsigned int frame_id;
		uint8_t *mapped_virtual_address;
		size_t num_out_byte = ctx->calculated_sizes.y_size + ctx->calculated_sizes.cbcr_size * 2;

		/* This call retrieves information about the decoded raw frame, including
		 * a pointer to the corresponding framebuffer structure. This must not be called more
		 * than once after IMX_VPU_DEC_OUTPUT_CODE_DECODED_FRAME_AVAILABLE was set. */
		imx_vpu_dec_get_decoded_frame(ctx->vpudec, &decoded_frame);
		//frame_id = (unsigned int)((uintptr_t)(decoded_frame.context));
		//fprintf(stderr, "decoded id: 0x%x  %u b\n", frame_id, num_out_byte);

		/* Map buffer to the local address space, dump the decoded frame to file,
		 * and unmap again. The decoded frame uses the I420 color format for all
		 * bitstream formats (h.264, MPEG2 etc.), with one exception; with motion JPEG data,
		 * the format can be different. See imxvpuapi.h for details. */
		mapped_virtual_address = imx_vpu_dma_buffer_map(decoded_frame.framebuffer->dma_buffer, IMX_VPU_MAPPING_FLAG_READ);

#ifdef DECODE_DIRECT_TO_FB 
		memcpy( outpaddr[0] , mapped_virtual_address , num_out_byte < screensize ? num_out_byte : screensize);
		fb_out( FB_FRESH );
#else
		rot_ipu( (void *)mapped_virtual_address , num_out_byte);
#endif
				
		#ifdef h264_decode_saveFile
			if( (decodeFrameNum == 3) && (fd_y_file1 != NULL) ){
				fwrite( mapped_virtual_address, num_out_byte, 1, fd_y_file1);
				fclose(fd_y_file1);
				fd_y_file1 = NULL;
			}
			else if( (decodeFrameNum == 4) && (fd_y_file2 != NULL) ){
				fwrite( mapped_virtual_address, num_out_byte, 1, fd_y_file2);
				fclose(fd_y_file2);
				fd_y_file2 = NULL;
			}
			else if( (decodeFrameNum == 5) && (fd_y_file3 != NULL) ){
				fwrite( mapped_virtual_address, num_out_byte, 1, fd_y_file3);
				fclose(fd_y_file3);
				fd_y_file3 = NULL;
			}

			if( frame_id < 0x70  && frame_id > 0x62) {
			sprintf(decodeFileName,"%d.yuv",frame_id);
			ctx->fout = fopen(decodeFileName, "wb");
			if(ctx->fout!= NULL){
				fwrite(mapped_virtual_address, 1, num_out_byte, ctx->fout);
				fclose(ctx->fout);
			}
		}
			
		#endif
		
		imx_vpu_dma_buffer_unmap(decoded_frame.framebuffer->dma_buffer);

		/* Mark the framebuffer as displayed, thus returning it to the list of
		 *framebuffers available for decoding. */
		imx_vpu_dec_mark_framebuffer_as_displayed(ctx->vpudec, decoded_frame.framebuffer);
	}
	else if (output_code & IMX_VPU_DEC_OUTPUT_CODE_DROPPED)
	{
		/* A frame was dropped. The context of the dropped frame can be retrieved
		 * if this is necessary for timestamping etc. */
		void* dropped_frame_id;
		imx_vpu_dec_get_dropped_frame_info(ctx->vpudec, &dropped_frame_id, NULL, NULL);
		fprintf(stderr, "dropped frame:  frame id: 0x%x\n", (unsigned int)((uintptr_t)dropped_frame_id));
	}

	if (output_code & IMX_VPU_DEC_OUTPUT_CODE_EOS)
	{
		fprintf(stderr, "VPU reports EOS; no more decoded frames available\n");
		ok = 0;
	}

	ctx->frame_id_counter++;

	return ok ? RETVAL_OK : RETVAL_EOS;
}


Retval h264_dec_run(h264_dec_Context *ctx, char * buf, int length)
{
	/* Feed frames to decoder & decode & output, until we run out of input data */
	ph264Buf = buf;
	h264Len  = length;
	#ifdef  SIMULATE_DECODE
		h264_decode_frame(ctx);
		return 0;
	#endif
	
	for (;;)
	{
		Retval ret = h264_decode_frame(ctx);
		if (ret == RETVAL_EOS)
			break;
		else if (ret == RETVAL_ERROR)
			return RETVAL_ERROR;
	}

	/* Enable drain mode; in this mode, any decoded frames that are still in the
	 * decoder are output; no input data is given (since there isn't any input data anymore) */

	fprintf(stderr, "draining decoder\n");
	imx_vpu_dec_enable_drain_mode(ctx->vpudec, 1);

	for (;;)
	{
		Retval ret = h264_decode_frame(ctx);
		if (ret == RETVAL_EOS)
			break;
		else if (ret == RETVAL_ERROR)
			return RETVAL_ERROR;
	}

	return RETVAL_OK;
}


void h264_dec_uninit(h264_dec_Context *ctx)
{
	unsigned int i;

	/* Close the previously opened decoder instance */
	imx_vpu_dec_close(ctx->vpudec);

	/* Free all allocated memory (both regular and DMA memory) */
	free(ctx->framebuffers);
	for (i = 0; i < ctx->num_framebuffers; ++i)
		imx_vpu_dma_buffer_deallocate(ctx->fb_dmabuffers[i]);
	free(ctx->fb_dmabuffers);
	imx_vpu_dma_buffer_deallocate(ctx->bitstream_buffer);

	/* Unload the VPU firmware */
	imx_vpu_dec_unload();
#ifdef DECODE_DIRECT_TO_FB 
	fb_out( FB_CLOSE);
#else
	rot_ipu_uninit();
#endif

	h264_ctx_cleanup(&(ctx->h264_ctx));

	free(ctx);
}
