#ifndef IMX_H264_UTILS_H____
#define IMX_H264_UTILS_H____

#include <stdio.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	FILE *fin;
	uint8_t *in_buffer;
	size_t in_buffer_allocated_size, in_buffer_data_size;
	size_t au_start_offset, au_end_offset;
	int au_finished, first_au;
}
h264_context;


void h264_ctx_init(h264_context *ctx );
void h264_ctx_cleanup(h264_context *ctx);
int h264_ctx_read_access_unit(h264_context *ctx,char * buf, int length);

//lhg add start
#include "imxvpuapi.h"
typedef enum
{
	RETVAL_OK = 0,
	RETVAL_ERROR = 1,
	RETVAL_EOS = 2
}
Retval;
typedef struct h264_decode_Context h264_dec_Context;
h264_dec_Context* h264_dec_init( );
Retval h264_dec_run(h264_dec_Context *ctx , char * , int);
void h264_dec_uninit(h264_dec_Context *ctx);

//lhg

#define H264_DECODE_LOCAL
#define H264_DECODE_LOCAL_ON_OPEN
//#define H264_DECODE_LOCAL_FILE


//lhg add end

#ifdef __cplusplus
}
#endif


#endif
