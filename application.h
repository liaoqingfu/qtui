#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDebug>

class Application : public QApplication
{
public:
    Application(int & argc, char ** argv);
    bool notify(QObject *, QEvent *);
    //bool qwsEventFilter(QWSEvent * event);

};

#endif // APPLICATION_H


