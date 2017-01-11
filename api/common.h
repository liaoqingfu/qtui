#ifndef __COMMON_LIB_H__
#define __COMMON_LIB_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <stdarg.h>
#include <errno.h> 
#include <stdbool.h>
#include <string.h>

static inline int  system_cmd_exec(const char *fmt, ...)
{
	int ret = 0;
	char cmd[256] = {0}; 
	memset(cmd,0,256);

	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);

//	printf("***  system exec : %s \n",cmd);
	ret = system(cmd);
	va_end(ap);

	return ret;
}

static inline int system_echo(char *value,char *dev) 
{
	if (!value || !dev) return -1;
	
    FILE *fp_val = fopen(dev, "rb+");
    if (!fp_val) return -1;

    int len = fwrite(value, sizeof(char), strlen(value), fp_val);	
	fclose(fp_val);

	if (len < 0) {
		printf("*** system_echo err:-1 ,%s \n",strerror(errno));
	}
	return len;
}

static inline int system_read(char *buf,int buf_len,char *dev) 
{
	if (!dev) return -1;
	
    FILE *fp_val = fopen(dev, "rb+");
    if (!fp_val) return -1;

	int len = fread(buf, 1,buf_len, fp_val);
	fclose(fp_val);
    
	if (len < 0) {
		printf("*** system_read err:-1 ,%s \n",strerror(errno));
	}
	return len;
}


#endif