#include "application.h"
#include "video/main_api.h"
#include "QDateTime"

#define TIME_INTERVAL  50    //MS
#define XDIS_INTERVAL  50    //PIX


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
   static int x_pos, x_pos_dis;
   int wt = pview->WindowType;

	if( (e->type() == QEvent::MouseButtonPress) ) {
		if ( (wt < WINDOW_TYPE_VIDEO_HALF)  )
			x_pos = QCursor::pos().rx();
	 	else   //video :  half window ,  full window switch
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
					ipu_para_set(1024, 600, 0 , 0, 512, 600, 4);
					qDebug("**half screen*");
				}

				pview->changeWindowType( wt );
				
			}
	 	}
	}
	else if(  e->type() == QEvent::MouseButtonRelease) {
		x_pos_dis = QCursor::pos().rx() - x_pos;
		qDebug() << e->type() << x_pos_dis;
		if( x_pos_dis > XDIS_INTERVAL)
			wt = (wt + 1) % WINDOW_TYPE_MAX_UI;
		else if( x_pos_dis < -XDIS_INTERVAL)
			wt = (wt + WINDOW_TYPE_MAX_UI - 1) % WINDOW_TYPE_MAX_UI;

		pview->changeWindowType( wt );
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
