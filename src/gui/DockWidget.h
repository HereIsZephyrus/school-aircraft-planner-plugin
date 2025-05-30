#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>

class LeftDockWidget : public QDockWidget{
    Q_OBJECT

public:
    LeftDockWidget(QWidget *parent = nullptr);
    ~LeftDockWidget();
};

class RightDockWidget : public QDockWidget{
    Q_OBJECT

public:
    RightDockWidget(QWidget *parent = nullptr);
    ~RightDockWidget();
};

#endif // DOCKWIDGET_H