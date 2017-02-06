# Makefile
#
#CFLAGS          = -Wall  -O2
ROOTDIR = ./
include $(ROOTDIR)Rules.make 


SRC  = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))

CPPSRC  = $(wildcard *.cpp  test/*.cpp)
OBJS += $(patsubst %.cpp,%.o,$(CPPSRC))


LINK_LIB = ../linux_lib/arm/
#LDFLAGS =  -pthread  -lrt #$(LINK_LIB)libeXosip2.so $(LINK_LIB)libosip2.so $(LINK_LIB)libosipparser2.so \
#LDFLAGS = ../Release/libresample.a 
INCLUDE_PATH = -I /home/jack/Spon-Xserial/imx6/kernel/linux-imx/include/uapi  \
-I/home/jack/Spon-Xserial/imx6/kernel/linux-imx/include #\
-I/home/jack/Spon-Xserial/imx6/kernel/linux-imx/include\
-I/home/jack/Spon-Xserial/imx6/kernel/linux-imx/arch/arm/include  
#-I/home/jac/Spon-Xserial/buildroot/output/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include 
#-I../include -I../include/eXosip2  -I../include/liblame  \
-I../include/libmad  -I../include/libresample  -I../include/ortp  \
-I../include/osip2  -I../include/osipparser2 -I../include/portaudio \
-I../include/osipparser2/headers  -I../ -I./

.PHONY:main

main:$(OBJS)
	$(ARM_CC) -o $@ $(OBJS)  -lpthread #$(LDFLAGS) -lstdc++    -lm 

%.o:%.cpp
	$(ARM_CC) $(INCLUDE_PATH) $(CFLAGS)  -c $< -o $@ 
%.o:%.c
	$(ARM_CC) $(INCLUDE_PATH) $(CFLAGS)  -c $< -o $@ 

clean:  
	rm -f $(OBJS) 
 



