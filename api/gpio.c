#include "common.h"


void gpio_direction_set(int pin, char *dir)
{
    char path[256] = {0};
	memset(path,0,256);
    sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);
	system_echo(dir,path);
}
//bOutDir  : 1,    "out"
bool gpio_open(int pin, int bOutDir)
{
	printf("open pin:%d dir:%d \n",pin,bOutDir);
	char path[256] = {0};
	memset(path,0,256);
	sprintf(path, "echo %d > /sys/class/gpio/export",pin);
	system_cmd_exec(path);
	if( bOutDir )
		gpio_direction_set(pin,"out");
	else
		gpio_direction_set(pin,"in");
	return true;
}

bool gpio_close(int pin)
{
    char path[256] = {0};
	memset(path,0,256);
	sprintf(path, "echo %d > /sys/class/gpio/unexport",pin);
	system_cmd_exec(path);
	return true;
}

//gpio输出设置
int gpio_set(int pin,int val)
{
    char path[256] = {0};
    char buf[8] = {0};

	memset(buf,0,8);
	memset(path,0,256);
	sprintf(buf,"%d",(val>0) ? 1:0);

	sprintf(path, "/sys/class/gpio/gpio%d/value",pin);

	//printf("### echo %s > %s ------ \n",buf,path);

	system_echo(buf,path);
    return val;
}

//gpio输入读取
int gpio_get(int pin)
{
    char path[256] = {0};
    char buf[8] = {0};
    int val = 0;
    
	memset(buf,0,8);
	memset(path,0,256);
	sprintf(path, "/sys/class/gpio/gpio%d/value", pin);

    system_read(buf,8,path);
    val = atoi(buf);
    return val;
}

