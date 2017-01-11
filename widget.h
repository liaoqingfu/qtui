#ifndef WIDGET_H
#define WIDGET_H


#include "public.h"
#include "myview.h"



namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(  QWidget *parent = 0);
    ~Widget();

    MyView *view;
};

#endif // WIDGET_H
