#include "widget.h"
#include <QApplication>

void WriteSettings()
{
    //QSettings settings("Software Inc", "Spreadsheet"); // windows在注册表中建立建 Software Inc -> Spreadsheet
    QSettings settings(CFG_NAME, QSettings::IniFormat); // 当前目录的INI文件
    settings.beginGroup("DevOption");
    settings.setValue("mainFun", 25);
    settings.setValue("subFun", 40);
    settings.setValue("service", 1);
    settings.endGroup();
}

void ReadSettings( QString sItem, int * value)
{
    QSettings settings(CFG_NAME, QSettings::IniFormat);

    * value = settings.value( sItem ).toInt();

}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
