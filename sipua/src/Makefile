# Makefile
#
ROOTDIR = ./
include $(ROOTDIR)/Rules.make 


SRC  = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))



CPPSRC  = $(wildcard *.cpp)
#CPPSRC += $(wildcard ./test/*.cpp)
OBJS += $(patsubst %.cpp,%.o,$(CPPSRC))

OBJS_OUT_DIR = ../../../sipuaobj/
SO_OUT_DIR = ../lib/
LINK_OBJS = $(patsubst %.o,$(OBJS_OUT_DIR)%.o,$(OBJS))

TARGET = $(SO_OUT_DIR)libua.so 
INCLUDE_DIR = -I../include -I../include/eXosip2  -I../include/liblame  \
-I../include/libmad  -I../include/libresample  -I../include/ortp  \
-I../include/osip2  -I../include/osipparser2 -I../include/portaudio \
-I../include/osipparser2/headers  -I./


all:$(TARGET)

%.o:%.c
	$(ARM_CC) $(INCLUDE_DIR) $(CFLAGS) $(CFLAGSO) -c $< -o $(OBJS_OUT_DIR)$@ 

%.o:%.cpp
	$(ARM_CC) $(INCLUDE_DIR) $(CFLAGS) $(CFLAGSO)  -c $< -o $(OBJS_OUT_DIR)$@ 

$(TARGET):$(OBJS)  
	$(ARM_CC) -shared  -o $@ $(LINK_OBJS) -lc #-lpthread  -lrt -leXosip2 -losip2 -losipparser2 -lortp -lportaudio
clean:  
	rm -f $(LINK_OBJS)
	rm -f $(OBJS) 


