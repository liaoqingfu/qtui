

#
# This make variable must be set before the demos or examples
# can be built. 
#
#CROSS_COMPILE:=/home/jack/Spon-Xserial/imx6/tools/arm-linux-cotex-a9-glibc/usr/bin/arm-linux-
CROSS_COMPILE = 
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CPP             = $(CROSS_COMPILE)g++ 
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
RM              = rm -f
MAKEDIR         = mkdir -p

#CFLAGS          += -Wall  -O2
CFLAGS          += -Wall  -g
AFLAGS         := -D__ASSEMBLY__






