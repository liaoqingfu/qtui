# Makefile
#
ROOTDIR = ../../
include $(ROOTDIR)/Rules.make 


SRC  = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRC))

CPPSRC  = $(wildcard *.cpp)
OBJS += $(patsubst %.cpp,%.o,$(CPPSRC))

OBJS_OUT_DIR = ../linux_lib/pc/
LINK_OBJS = $(patsubst %.o,$(OBJS_OUT_DIR)%.o,$(OBJS))

TARGET = $(OBJS_OUT_DIR)libua.a 
INCLUDE_DIR = -I../include -I../include/eXosip2  -I../include/liblame  \
-I../include/libmad  -I../include/libresample  -I../include/ortp  \
-I../include/osip2  -I../include/osipparser2 -I../include/portaudio \
-I../include/osipparser2/headers  -I./
LIB_DIR = -L../linux_lib/pc

all:$(TARGET)

%.o:%.c
	$(CC) $(INCLUDE_DIR) -pthread -static -c $< -o $(OBJS_OUT_DIR)$@ 

%.o:%.cpp
	$(CC) $(INCLUDE_DIR) -pthread -static -c $< -o $(OBJS_OUT_DIR)$@ 

$(TARGET):$(OBJS)  
	$(AR) -cr -o $@ $(LINK_OBJS)   -lc #-lpthread  -lrt -leXosip2 -losip2 -losipparser2 -lortp -lportaudio
clean:  
	rm -f $(LINK_OBJS) $(OBJS) 


