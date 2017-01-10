

#
# This make variable must be set before the demos or examples
# can be built. 
#
CROSS_COMPILE:=/home/jack/Spon-Xserial/imx6/tools/arm-linux-cotex-a9-glibc/usr/bin/arm-linux-
AS              = as
LD              = ld
CC              = gcc
CPP             = $(CC) 
AR              = ar
NM              = nm
STRIP           = strip
OBJCOPY         = objcopy
OBJDUMP         = objdump
RM              = rm -f
MAKEDIR         = mkdir -p

ARM_AS              = $(CROSS_COMPILE)as
ARM_LD              = $(CROSS_COMPILE)ld
ARM_CC              = $(CROSS_COMPILE)gcc
ARM_CPP             = $(CC) 
ARM_AR              = $(CROSS_COMPILE)ar
ARM_NM              = $(CROSS_COMPILE)nm
ARM_STRIP           = $(CROSS_COMPILE)strip
ARM_OBJCOPY         = $(CROSS_COMPILE)objcopy
ARM_OBJDUMP         = $(CROSS_COMPILE)objdump

CFLAGS          += -Wall 
#CFLAGS          += -fPIC -shared
#LDFLAGS += -fPIC -shared
LDFLAGS = cr
AFLAGS         := -D__ASSEMBLY__






