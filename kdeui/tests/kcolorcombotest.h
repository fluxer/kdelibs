#ifndef KCOLORCOMBOTEST_H
#define KCOLORCOMBOTEST_H

#include <QtGui/QWidget>

class QPushButton;
class KColorCombo;

class KColorComboTest : public QWidget
{
    Q_OBJECT

public:
    KColorComboTest(QWidget *parent = 0);
    ~KColorComboTest();

private Q_SLOTS:
    void quitApp();

protected:
    KColorCombo* mStandard;
    KColorCombo* mCustom;
    QPushButton* mExit;
};

#endif
