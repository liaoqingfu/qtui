#include "application.h"
#include "video/main_api.h"
#include "QDateTime"

#define INTERVAL 50

extern int bVideoShow ;
Application::Application(int &argc, char **argv):
    QApplication(argc,argv)
{
}

bool Application::notify(QObject *obj, QEvent *e)
{
   static bool fullScreen = true;
   static qint64 startTime = QDateTime::currentMSecsSinceEpoch();

   //if( bVideoShow )
   // qDebug() <<e->type();
 
    if( e->type() == QEvent::MouseButtonPress ) {
		if (QDateTime::currentMSecsSinceEpoch() - startTime > INTERVAL) {
			startTime = QDateTime::currentMSecsSinceEpoch();
			fullScreen = !fullScreen;
			if( fullScreen ) {
				ipu_para_set(1024, 600, 0 , 0, 0, 0, 4);
				qDebug("**full screen*");
			}
			else {
				//ipu_para_set(output_w, output_h, crop_x , crop_y, crop_w, crop_h, rotate);
				ipu_para_set(1024, 600, 0 , 0, 512, 600, 4);
				qDebug("**half screen*");
			}
		}
 	}
    return QApplication::notify(obj,e);
}

/*
bool Application::qwsEventFilter(QWSEvent *event)
{
    //do something
    qDebug("**qwsEventFilter*");
    return QApplication::qwsEventFilter(event);
}*/
