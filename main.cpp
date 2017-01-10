#include "widget.h"
//#include <QApplication>
#include "application.h"



int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
   Application a(argc, argv);

    QIcon ico(":/broadcast1.png");
    a.setWindowIcon( ico );

    QFont font;
    font.setPointSize(26);
    font.setFamily( FONE_NAME );
    font.setBold(false);
    a.setFont(font);

    Widget w;
   // w.show();

    return a.exec();
}
