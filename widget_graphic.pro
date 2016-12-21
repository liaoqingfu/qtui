#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T14:41:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QT += multimedia
#QT += multimediawidgets
TARGET = widget_graphic
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    myscene.cpp \
    myview.cpp

HEADERS  += widget.h \
    myscene.h \
    myview.h \
    public.h

FORMS    +=

RESOURCES += \
    pic.qrc
