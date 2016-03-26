#ifndef TEST_KSTATUSBAR_H
#define TEST_KSTATUSBAR_H

#include <kstatusbar.h>
#include <kxmlguiwindow.h>
#include <QMenuBar>

class QTextEdit;

class testWindow  : public KXmlGuiWindow
{
    Q_OBJECT

public:
    testWindow (QWidget *parent=0);
    ~testWindow ();

public Q_SLOTS:
    void slotPress(int i);
    void slotClick(int i);
    void slotMenu(QAction*);

protected:
    QMenu *fileMenu;
    QMenu *smenu;
    QMenuBar *menuBar;
    KStatusBar *statusbar;
    bool insert;
    QTextEdit *widget;
};
#endif
