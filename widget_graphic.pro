#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T14:41:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT  += network

#QT += multimedia
#QT += multimediawidgets
TARGET = widget_graphic
TEMPLATE = app
INCLUDEPATH += /home/jack/Spon-Xserial/imx6/kernel/linux-imx/include/uapi/
INCLUDEPATH += /home/jack/Spon-Xserial/imx6/kernel/linux-imx/include/

SOURCES += main.cpp\
        widget.cpp \
    myview.cpp \
    application.cpp \
    video/imx_h264_dec.c \
    video/rot_ipu.c \
    video/imx_h264_enc.c \
    video/imx_v4l2_capture.c \
    video/imxvpuapi_vpulib.c \
    video/imx_h264_utils.c \
    video/vpu_util.c \
    video/vpu_lib.c \
    video/imxvpuapi.c \
    video/imxvpuapi_parse_jpeg.c \
    video/imxvpuapi_jpeg.c \
    video/vpu_io.c \
    video/vpu_gdi.c \
    video/vpu_debug.c \
    video/main_api.c \
    myscene_video.cpp \
    myscene_num_call.cpp \
    myscene_main.cpp \
    api/gpio.c

HEADERS  += widget.h \
    myview.h \
    public.h \
    application.h \
    video/imx_h264_utils.h \
    video/vpu_reg.h \
    video/imx_v4l2_capture.h \
    video/imxvpuapi.h \
    video/config.h \
    video/imx_h264_enc.h \
    video/imxvpuapi_priv.h \
    video/imxvpuapi_parse_jpeg.h \
    video/imxvpuapi_jpeg.h \
    video/vpu_lib.h \
    video/vpu_io.h \
    video/vpu_gdi.h \
    video/vpu_debug.h \
    video/vpu_util.h \
    video/main_api.h \
    myscene_video.h \
    myscene_num_call.h \
    myscene_main.h \
    api/common.h

FORMS    +=

RESOURCES += \
    pic.qrc
