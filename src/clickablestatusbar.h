#ifndef CLICKABLESTATUSBAR_H
#define CLICKABLESTATUSBAR_H

#include <QStatusBar>

#include <iostream>
class ClickableStatusBar : public QStatusBar
{
    Q_OBJECT
public:
    ClickableStatusBar(QWidget* parent):QStatusBar(parent) {}

    ClickableStatusBar()
    {

    }

    ~ClickableStatusBar()
    {

    }

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        emit mouseClick();
    }

signals:
    void mouseClick();
};

#endif // CLICKABLESTATUSBAR_H
