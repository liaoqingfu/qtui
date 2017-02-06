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
#INCLUDEPATH +=  /home/jack/shareFile/qtApp/vertical_9171/sipua/include /home/jack/shareFile/qtApp/vertical_9171/sipua/include/eXosip2  /home/jack/shareFile/qtApp/vertical_9171/sipua/include/liblame  \
#/home/jack/shareFile/qtApp/vertical_9171/sipua/include/libmad  /home/jack/shareFile/qtApp/vertical_9171/sipua/include/libresample  /home/jack/shareFile/qtApp/vertical_9171/sipua/include/ortp  \
#/home/jack/shareFile/qtApp/vertical_9171/sipua/include/osip2  /home/jack/shareFile/qtApp/vertical_9171/sipua/include/osipparser2 /home/jack/shareFile/qtApp/vertical_9171/sipua/include/portaudio \
#/home/jack/shareFile/qtApp/vertical_9171/sipua/include/osipparser2/headers

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
    myscene_video.cpp \
    myscene_num_call.cpp \
    myscene_main.cpp \
    api/gpio.c \
    myscene_piccall.cpp \
    myscene_listcall.cpp \
  #  sipua/XC9000UATest1.cpp \
    myscene_calling.cpp \
    sipua/src/AudioFile.cpp \
    sipua/src/AudioStream.cpp \
    sipua/src/EventList.cpp \
    sipua/src/H264Pack.cpp \
    sipua/src/log.cpp \
    sipua/src/PlainParse.cpp \
    sipua/src/SipSponServer.cpp \
    sipua/src/SipUA.cpp \
    sipua/src/socket.cpp \
    sipua/src/Terminal.cpp \
    sipua/src/ua_port.cpp \
    sipua/src/VideoStream.cpp \
    sipua/src/XC9000UATest1.cpp \
    sipua/src/CodecAlawUlaw.c \
    sipua/src/ua_global.c







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
    api/common.h \
    myscene_piccall.h \
    myscene_listcall.h \
    myscene_calling.h \
    sipua/src/CodecAlawUlaw.h \
    sipua/src/defines.h \
    sipua/src/SipSponServer.h \
    sipua/src/SqQueue.h \
    sipua/src/Terminal.h \
    sipua/src/version.h



FORMS    +=

RESOURCES += \
    pic.qrc


INCLUDEPATH += $$PWD/sipua/lib
DEPENDPATH += $$PWD/sipua/lib



INCLUDEPATH += $$PWD/sipua/include
DEPENDPATH += $$PWD/sipua/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lua
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lua
#else:unix: LIBS += -L$$PWD/sipua/lib/ -lua


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lresample
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lresample
else:unix: LIBS += -L$$PWD/sipua/lib/ -lresample


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lportaudio
else:unix: LIBS += -L$$PWD/sipua/lib/ -lportaudio


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -losipparser2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -losipparser2
else:unix: LIBS += -L$$PWD/sipua/lib/ -losipparser2


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -losip2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -losip2
else:unix: LIBS += -L$$PWD/sipua/lib/ -losip2


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lortp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lortp
else:unix: LIBS += -L$$PWD/sipua/lib/ -lortp


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lmad
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lmad
else:unix: LIBS += -L$$PWD/sipua/lib/ -lmad


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -llame
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -llame
else:unix: LIBS += -L$$PWD/sipua/lib/ -llame


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -leXosip2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -leXosip2
else:unix: LIBS += -L$$PWD/sipua/lib/ -leXosip2


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/sipua/lib/release/ -lasound
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/sipua/lib/debug/ -lasound
else:unix: LIBS += -L$$PWD/sipua/lib/ -lasound

DISTFILES +=

