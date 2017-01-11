#include "widget.h"




Widget::Widget(QWidget *parent) :
    QWidget(parent)
{

    
   // scene->setBackgroundBrush(QPixmap(":/othermainwindowbg.jpg"));


    view = new MyView(parent);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setFixedSize(1024,600);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}


Widget::~Widget()
{
    //delete ui;
}

