#include "application.h"
#include "video/main_api.h"
#include "QDateTime"

#define TIME_INTERVAL  50    //MS
#define YDIS_INTERVAL  50    //PIX


Application::Application(int &argc, char **argv ):
    QApplication(argc,argv)
{
	
}
void Application::setMyView( class  MyView *pv )
{
	pview = pv;
}

bool Application::notify(QObject *obj, QEvent *e)
{
   static qint64 startTime = 0;//filter closely event
   static QPoint   pos, pos_dis;
   int wt = pview->WindowType;
   QMouseEvent *mouseEvent = (QMouseEvent *)e;

	if( (e->type() == QEvent::MouseButtonPress) ) {
		if ( (wt < WINDOW_TYPE_VIDEO_HALF)  ){
			pos = mouseEvent->globalPos();
			qDebug() << "Press" << pos.x() << pos.y();
		}
	 	else  if ( (wt <= WINDOW_TYPE_VIDEO_FUL)  ) //video :  half window ,  full window switch
	 	{
	 		if( startTime == 0 )
				startTime = QDateTime::currentMSecsSinceEpoch();
			
			if (QDateTime::currentMSecsSinceEpoch() - startTime > TIME_INTERVAL) {
				startTime = QDateTime::currentMSecsSinceEpoch();

				if( wt == WINDOW_TYPE_VIDEO_HALF)
				{
					wt = WINDOW_TYPE_VIDEO_FUL;
					ipu_para_set(1024, 600, 0 , 0, 0, 0, 4);
					qDebug("**full screen*");
				}
				else {
					//ipu_para_set(output_w, output_h, crop_x , crop_y, crop_w, crop_h, rotate);
					wt = WINDOW_TYPE_VIDEO_HALF;
					ipu_para_set(1024, 600, 100 , 0, 512, 600, 4);
					qDebug("**half screen*");
				}

				pview->changeWindowType( wt );
				
			}
	 	}
	}
	else if(  e->type() == QEvent::MouseButtonRelease) {
		if ( (wt < WINDOW_TYPE_VIDEO_HALF)  ){
			pos_dis = mouseEvent->globalPos()  - pos;
			pos = mouseEvent->globalPos();
			if( pos_dis.y() > YDIS_INTERVAL)
				wt = (wt + 1) % WINDOW_TYPE_MAX_UI;
			else if( pos_dis.y() < -YDIS_INTERVAL)
				wt = (wt + WINDOW_TYPE_MAX_UI - 1) % WINDOW_TYPE_MAX_UI;
			qDebug() << "Release" << pos.x()<< pos.y()<< wt;

			pview->changeWindowType( wt );
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
