#ifndef PUBLIC_H
#define PUBLIC_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "QLabel"

#include "QPixmap"
#include "QPushButton"
#include "QIcon"
#include "QDebug"
#include "QPainter"
#include "QTime"
#include <QThread>
#include <QTextStream>
#include "QDesktopWidget"
#include "QRect"
#include "QPixmap"
#include "QToolButton"
#include "QGraphicsProxyWidget"
#include "QGraphicsView"
#include "QVBoxLayout"
#include "QHBoxLayout"


#include <QGraphicsSceneMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>

#include  "QToolButton"
#include "QGraphicsProxyWidget"
#include "QVBoxLayout"
#include "QIcon"
#include "QPixmap"

#include <QtWidgets>
#include <qtextcodec.h>
#include <qfont.h>
#include <qapplication.h>
#include "QToolTip"

#include <qmath.h>
#define FRAME_FPS     15
#define REF_TIME_MS  1000
#define REF_VIDEO_MS (1000/FRAME_FPS)

#define NUM_CALL 15   //num of call num button
#define MAX_NUM_LEN 10   //lengh of call num
#define MAX_CALL_TYPE  2

#define FONE_NAME "wenquanyi"
#ifdef SYS_WIN
    #define CFG_NAME "d:\sys_cfg.txt"
#else
    #define CFG_NAME "sys_cfg.txt"
#endif

#define NUM_X_WID       160
#define NUM_START_X     200   //1开始位置

#define NUM_Y_HEIGHT    200

#endif // PUBLIC_H

