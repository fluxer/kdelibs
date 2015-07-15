#ifndef MAIN_H
#define MAIN_H

#include <QObject>
#include <KMainWindow>

class Window : public KMainWindow
{
    Q_OBJECT
public:
    Window();
};

#endif
