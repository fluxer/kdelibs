#ifndef KTREEWIDGETSEARCHLINETEST_H
#define KTREEWIDGETSEARCHLINETEST_H

#include <kdialog.h>
#include <QTreeWidget>

class KTreeWidgetSearchLineTest : public KDialog
{
    Q_OBJECT

public:
    KTreeWidgetSearchLineTest();

    void create2ndLevel(class QTreeWidgetItem* item);
    void create3rdLevel(QTreeWidgetItem* item);

public Q_SLOTS:
    void switchCaseSensitivity(bool cs);

protected:
    virtual void showEvent(QShowEvent* event);

private:
    class KTreeWidgetSearchLine* m_searchLine;
    QTreeWidget* tw;
};

#endif
