# Makefile
#
#CFLAGS          = -Wall  -O2
ROOTDIR = ../../../
include $(ROOTDIR)/Rules.make 


SRC  = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))

CPPSRC = $(wildcard *.cpp)
CPPSRC += $(wildcard ./test/*.cpp)
OBJS += $(patsubst %.cpp,%.o,$(CPPSRC))


LINK_LIB = ../../linux_lib/arm/
LDFLAGS =  -pthread  -lrt $(LINK_LIB)libeXosip2.so $(LINK_LIB)libosip2.so $(LINK_LIB)libosipparser2.so \
		$(LINK_LIB)libortp.so $(LINK_LIB)libasound.so.2 $(LINK_LIB)libportaudio.so $(LINK_LIB)libmad.so \
		$(LINK_LIB)liblame.so  $(LINK_LIB)libresample.so  $(LINK_LIB)libua.so #../linux_lib/pc/libua.so #-losip2 -losipparser2 -lortp -lportaudio -lua
#LDFLAGS = ../Release/libresample.a 
INCLUDE_PATH = -I../../include -I../../include/eXosip2  -I../../include/liblame  \
-I../../include/libmad  -I../../include/libresample  -I../../include/ortp  \
-I../../include/osip2  -I../../include/osipparser2 -I../../include/portaudio \
-I../../include/osipparser2/headers  -I../

.PHONY:main

main:$(OBJS)
	$(ARM_CC) -o $@ $(OBJS)  $(LDFLAGS) -lstdc++    -lm

%.o:%.cpp
	$(ARM_CC) $(INCLUDE_PATH)  -c $< -o $@ 
%.o:%.c
	$(ARM_CC) $(INCLUDE_PATH)  -c $< -o $@ 

clean:  
	rm -f $(OBJS) 
 



