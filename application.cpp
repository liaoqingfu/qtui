#include "application.h"
#include "video/main_api.h"
#include "QDateTime"
//#include "public.h"

#define TIME_INTERVAL  50    //MS
#define YDIS_INTERVAL  50    //PIX

#define DBCLK_INTERVAL  10    //MS



extern ncs_cfg_t cfg;
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
   static int video_halfscreen = cfg.display_halfScrenn;
   QMouseEvent *mouseEvent = (QMouseEvent *)e;

	if( (e->type() == QEvent::MouseButtonPress) ) {
		if( pview->b_screenSaver )  //����
			pview->setScreenSaver(0);
		pview->screenSaverStartTime = QDateTime::currentMSecsSinceEpoch();
		
		if ( (wt < WINDOW_TYPE_VIDEO_HALF)  ){
			pos = mouseEvent->globalPos();
			//qDebug() << "P:" << pos.x() << pos.y();
		}
	 	else  if ( (wt <= WINDOW_TYPE_CALLING)  ) //video :  half window ,  full window switch
	 	{
	 		if( startTime == 0 )
				startTime = QDateTime::currentMSecsSinceEpoch();
			
			if (QDateTime::currentMSecsSinceEpoch() - startTime < DBCLK_INTERVAL) {   
				startTime = QDateTime::currentMSecsSinceEpoch();
				if( !video_halfscreen  )
				{
					ipu_para_set(1024, 600, 0 , 0, 0, 0, 4);
					pview->scene_calling->bt_hangup->setVisible(false);
					pview->scene_calling->bt_hangup->update();
					qDebug("**full screen*");
				}
				else {
					//ipu_para_set(output_w, output_h, crop_x , crop_y, crop_w, crop_h, rotate);
					ipu_para_set(512, 600, 100 , 0, 0, 0, 4);
					pview->scene_calling->bt_hangup->setVisible(true);
					pview->scene_calling->bt_hangup->update();
					qDebug("**half screen*");
				}
				video_halfscreen = !video_halfscreen;
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
			qDebug() << "R:" << pos.x()<< pos.y()<< wt;

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
