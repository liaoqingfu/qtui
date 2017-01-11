#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDebug>
#include "public.h"
#include "myview.h"  


class Application : public QApplication
{
Q_OBJECT

public:
    Application(int & argc, char ** argv);
	void setMyView( class  MyView *pv );
    bool notify(QObject *, QEvent *);

private:
    class  MyView *pview;

};

#endif // APPLICATION_H


