/* example for how to use the imxvpuapi encoder interface
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imx_h264_enc.h"
#include "imxvpuapi.h"

#define IMXVPUAPI_DEDUG		0

#if IMXVPUAPI_DEDUG
#include <stdarg.h> 
void my_ImxVpuLoggingFunc(ImxVpuLogLevel level, char const *file, int const line, char const *fn, const char *format, ...)
{
	char cmd[256] = {0}; 
	memset(cmd,0,512);

	printf("DUG_[%s]_[%d]_[%s] ",file,line,fn);

	va_list ap;
	va_start(ap, format);
	vsprintf(cmd, format, ap);
	printf("\t : %s \n",cmd);
	va_end(ap);
}

//imx_vpu_set_logging_function(my_ImxVpuLoggingFunc);
//imx_vpu_set_logging_threshold(IMX_VPU_LOG_LEVEL_TRACE);
#endif

typedef enum
{
	RETVAL_OK = 0,
	RETVAL_ERROR = 1,
	RETVAL_EOS = 2
}
Retval;


typedef struct _Context Context;

/* This is a simple example of how to encode with the imxvpuapi library.
 * It encodes procedurally generated frames to h.264 and dumps the encoded
 * frames to a file. Also look into imxvpuapi.h for documentation.
 *
 * This expects as input a file with uncompressed 320x240 i420 frames and
 * 25 fps. The encoder outputs a byte-stream formatted h.264 stream, which
 * can be played with VLC or mplayer for example. */



#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
#define FRAME_SIZE ((FRAME_WIDTH) * (FRAME_HEIGHT) * 12 / 8)
#define COLOR_FORMAT IMX_VPU_COLOR_FORMAT_YUV420
#define FPS_N 25
#define FPS_D 1


struct _Context
{
	FILE *fin, *fout;

	ImxVpuEncoder *vpuenc;

	ImxVpuDMABuffer *bitstream_buffer;
	size_t bitstream_buffer_size;
	unsigned int bitstream_buffer_alignment;

	ImxVpuEncInitialInfo initial_info;

	ImxVpuFramebuffer input_framebuffer;
	ImxVpuDMABuffer *input_fb_dmabuffer;

	ImxVpuFramebuffer *framebuffers;
	ImxVpuDMABuffer **fb_dmabuffers;
	unsigned int num_framebuffers;
	ImxVpuFramebufferSizes calculated_sizes;

	ImxVpuRawFrame input_frame;
	ImxVpuEncodedFrame output_frame;
	ImxVpuEncParams enc_params;
	void *encoded_mem;
	int encoded_size;

	int inited;
};

Context * h264_enc_ctx_get(void)
{
	static Context *h264_enc_ctx = NULL;
	if (!h264_enc_ctx) {
	
#if IMXVPUAPI_DEDUG
imx_vpu_set_logging_function(my_ImxVpuLoggingFunc);
imx_vpu_set_logging_threshold(IMX_VPU_LOG_LEVEL_TRACE);
#endif

		h264_enc_ctx = calloc(1, sizeof(Context));
	}
	return h264_enc_ctx;
}

void* acquire_output_buffer(void *context, size_t size, void **acquired_handle)
{
	static int latest_max_len = 0;

	Context *ctx = h264_enc_ctx_get();
	if (ctx->encoded_mem && latest_max_len >= size) {
		*acquired_handle = ctx->encoded_mem;
		ctx->encoded_size = size;
		return ctx->encoded_mem;
	}
/*	if (ctx->encoded_mem && ctx->encoded_size == size) {
		*acquired_handle = ctx->encoded_mem;
		return ctx->encoded_mem;
	} */
	if (ctx->encoded_mem) {
		free (ctx->encoded_mem);
		ctx->encoded_mem = NULL;
		ctx->encoded_size = 0;
	}
	ctx->encoded_mem = malloc(size);
	ctx->encoded_size = size;
	latest_max_len = size;
	*acquired_handle = ctx->encoded_mem;
	//fprintf(stderr, "acquired output buffer, encoded_mem %p size:%d\n", ctx->encoded_mem,ctx->encoded_size);
	return ctx->encoded_mem;
}


void finish_output_buffer(void *context, void *acquired_handle)
{
	((void)(context));

	/* Nothing needs to be done here in this example. Just log this call. */
	//fprintf(stderr, "finished output buffer, handle %p", acquired_handle);
}


Context* h264_enc_ctx_init(Context *ctx,int frame_width,int frame_height,int fps)
{
	ImxVpuEncOpenParams open_params;
	unsigned int i;

	//ctx->fin = input_file;
	//ctx->fout = output_file;


	/* Set the open params. Use the default values (note that memset must still
	 * be called to ensure all values are set to 0 initially; the
	 * imx_vpu_enc_set_default_open_params() function does not do this!).
	 * Then, set a bitrate of 0 kbps, which tells the VPU to use constant quality
	 * mode instead (controlled by the quant_param field in ImxVpuEncParams).
	 * Frame width & height are also necessary, as are the frame rate numerator
	 * and denominator. */
	memset(&open_params, 0, sizeof(open_params));
	imx_vpu_enc_set_default_open_params(IMX_VPU_CODEC_FORMAT_H264, &open_params);
	open_params.bitrate = 0;
	open_params.frame_width = frame_width;
	open_params.frame_height = frame_height;
	open_params.frame_rate_numerator = fps;
	open_params.frame_rate_denominator = FPS_D;
	//4M bps - 20 q
	open_params.bitrate = 	4*1024; //2Mbps
	open_params.intra_qp = 	30;	//quilty 0-51 
	printf("--------------H264 bitrate:%d intra_qp:%d------------ \n",open_params.bitrate,open_params.intra_qp);
	/* Load the VPU firmware */
	imx_vpu_enc_load();

	/* Retrieve information about the required bitstream buffer and allocate one based on this */
	imx_vpu_enc_get_bitstream_buffer_info(&(ctx->bitstream_buffer_size), &(ctx->bitstream_buffer_alignment));
	ctx->bitstream_buffer = imx_vpu_dma_buffer_allocate(
		imx_vpu_enc_get_default_allocator(),
		ctx->bitstream_buffer_size,
		ctx->bitstream_buffer_alignment,
		0
	);

	/* Open an encoder instance, using the previously allocated bitstream buffer */
	imx_vpu_enc_open(&(ctx->vpuenc), &open_params, ctx->bitstream_buffer);


	/* Retrieve the initial information to allocate framebuffers for the
	 * encoding process (unlike with decoding, these framebuffers are used
	 * only internally by the encoder as temporary storage; encoded data
	 * doesn't go in there, nor do raw input frames) */
	imx_vpu_enc_get_initial_info(ctx->vpuenc, &(ctx->initial_info));

	ctx->num_framebuffers = ctx->initial_info.min_num_required_framebuffers;
	fprintf(stderr, "num framebuffers: %u\n", ctx->num_framebuffers);

	/* Using the initial information, calculate appropriate framebuffer sizes */
	imx_vpu_calc_framebuffer_sizes(IMX_VPU_COLOR_FORMAT_YUV420, frame_width, frame_height, ctx->initial_info.framebuffer_alignment, 0, 0, &(ctx->calculated_sizes));
	fprintf(
		stderr,
		"enc calculated sizes:  frame width&height: %dx%d  Y stride: %u  CbCr stride: %u  Y size: %u  CbCr size: %u  MvCol size: %u  total size: %u\n",
		ctx->calculated_sizes.aligned_frame_width, ctx->calculated_sizes.aligned_frame_height,
		ctx->calculated_sizes.y_stride, ctx->calculated_sizes.cbcr_stride,
		ctx->calculated_sizes.y_size, ctx->calculated_sizes.cbcr_size, ctx->calculated_sizes.mvcol_size,
		ctx->calculated_sizes.total_size
	);


	/* Allocate memory blocks for the framebuffer and DMA buffer structures,
	 * and allocate the DMA buffers themselves */

	ctx->framebuffers = malloc(sizeof(ImxVpuFramebuffer) * ctx->num_framebuffers);
	ctx->fb_dmabuffers = malloc(sizeof(ImxVpuDMABuffer*) * ctx->num_framebuffers);

	for (i = 0; i < ctx->num_framebuffers; ++i)
	{
		/* Allocate a DMA buffer for each framebuffer. It is possible to specify alternate allocators;
		 * all that is required is that the allocator provides physically contiguous memory
		 * (necessary for DMA transfers) and respecs the alignment value. */
		ctx->fb_dmabuffers[i] = imx_vpu_dma_buffer_allocate(imx_vpu_dec_get_default_allocator(), ctx->calculated_sizes.total_size, ctx->initial_info.framebuffer_alignment, 0);

		imx_vpu_fill_framebuffer_params(&(ctx->framebuffers[i]), &(ctx->calculated_sizes), ctx->fb_dmabuffers[i], 0);
	}

	/* allocate DMA buffers for the raw input frames. Since the encoder can only read
	 * raw input pixels from a DMA memory region, it is necessary to allocate one,
	 * and later copy the pixels into it. In production, it is generally a better
	 * idea to make sure that the raw input frames are already placed in DMA memory
	 * (either allocated by imx_vpu_dma_buffer_allocate() or by some other means of
	 * getting DMA / physically contiguous memory with known physical addresses). */
	ctx->input_fb_dmabuffer = imx_vpu_dma_buffer_allocate(imx_vpu_dec_get_default_allocator(), ctx->calculated_sizes.total_size, ctx->initial_info.framebuffer_alignment, 0);
	imx_vpu_fill_framebuffer_params(&(ctx->input_framebuffer), &(ctx->calculated_sizes), ctx->input_fb_dmabuffer, 0);

	/* Actual registration is done here. From this moment on, the VPU knows which buffers to use for
	 * storing temporary frames into. This call must not be done again until encoding is shut down. */
	imx_vpu_enc_register_framebuffers(ctx->vpuenc, ctx->framebuffers, ctx->num_framebuffers);

	/* Set up the input frame. The only field that needs to be
	 * set is the input framebuffer. The encoder will read from it.
	 * The rest can remain zero/NULL. */
	ctx->input_frame.framebuffer = &(ctx->input_framebuffer);

	/* Set the encoding parameters for this frame. quant_param 0 is
	 * the highest quality in h.264 constant quality encoding mode.
	 * (The range in h.264 is 0-51, where 0 is best quality and worst
	 * compression, and 51 vice versa.) */
	ctx->enc_params.quant_param = 0;
	ctx->enc_params.acquire_output_buffer = acquire_output_buffer;
	ctx->enc_params.finish_output_buffer = finish_output_buffer;
	ctx->enc_params.output_buffer_context = NULL;

	/* Set up the output frame. Simply setting all fields to zero/NULL
	 * is enough. The encoder will fill in data. */
	memset(&ctx->output_frame, 0, sizeof(ctx->output_frame));
	ctx->inited = 1;
	return ctx;
}


void h264_enc_ctx_uninit(Context *ctx)
{
	unsigned int i;

	/* Close the previously opened encoder instance */
	imx_vpu_enc_close(ctx->vpuenc);

	/* Free all allocated memory (both regular and DMA memory) */
	imx_vpu_dma_buffer_deallocate(ctx->input_fb_dmabuffer);
	free(ctx->framebuffers);
	for (i = 0; i < ctx->num_framebuffers; ++i)
		imx_vpu_dma_buffer_deallocate(ctx->fb_dmabuffers[i]);
	free(ctx->fb_dmabuffers);
	imx_vpu_dma_buffer_deallocate(ctx->bitstream_buffer);

	/* Unload the VPU firmware */
	imx_vpu_enc_unload();

	if (ctx->encoded_mem) {
		free (ctx->encoded_mem);
		ctx->encoded_mem = NULL;
		ctx->encoded_size = 0;
	}

	ctx->inited = 0;
}



int h264_enc_process(char* buf,int nsize,char** encoded_buf)
{
	Context *ctx = h264_enc_ctx_get();
	/* Read input i420 frames and encode them until the end of the input file is reached */
	if (ctx->inited != 1) {
		fprintf(stderr, "Context not init\n");
		return -1;
	}
	
	if (!buf || nsize < 0) {
		fprintf(stderr, "input param error, nsize=%d  total_size=%d \n", nsize,ctx->calculated_sizes.total_size);
		return -1;
	}


	uint8_t *mapped_virtual_address;
	unsigned int output_code;
	//void *output_block;

	/* Read uncompressed pixels into the input DMA buffer */
	mapped_virtual_address = imx_vpu_dma_buffer_map(ctx->input_fb_dmabuffer, IMX_VPU_MAPPING_FLAG_WRITE);
	memcpy(mapped_virtual_address, buf, nsize);
	
	imx_vpu_dma_buffer_unmap(ctx->input_fb_dmabuffer);
	
	/* The actual encoding */
	imx_vpu_enc_encode(ctx->vpuenc, &ctx->input_frame, &ctx->output_frame, &ctx->enc_params, &output_code);
	/* Write out the encoded frame to the output file. The encoder
	 * will have called acquire_output_buffer(), which acquires a
	 * buffer by malloc'ing it. The "handle" in this example is
	 * just the pointer to the allocated memory. This means that
	 * here, acquired_handle is the pointer to the encoded frame
	 * data. Write it to file, and then free the previously
	 * allocated block. In production, the acquire function could
	 * retrieve an output memory block from a buffer pool for
	 * example. */
	//output_block = ctx->output_frame.acquired_handle;
	output_code =  ctx->output_frame.data_size;
	
	* encoded_buf = ctx->output_frame.acquired_handle;

	return output_code;
}

int h264_enc_init(int frame_width,int frame_height,int fps)
{
	Context *ctx = h264_enc_ctx_get();
	h264_enc_ctx_init(ctx,frame_width,frame_height,fps);
	return 1;
}

int h264_enc_uninit(void)
{
	Context *ctx = h264_enc_ctx_get();
	h264_enc_ctx_uninit(ctx);
	return 1;
}
