#include "widget.h"
//#include <QApplication>
#include "application.h"



int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    Application appli(argc, argv );
    
    Widget wid;
	appli.setMyView( wid.view );
	
    QIcon ico(":/broadcast1.png");
    appli.setWindowIcon( ico );

    QFont font;
    font.setPointSize(26);
    font.setFamily( FONE_NAME );
    font.setBold(false);
    appli.setFont(font);


    wid.view->show();

    return appli.exec();
}
