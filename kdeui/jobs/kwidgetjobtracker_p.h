/*  This file is part of the KDE project
    Copyright (C) 2007 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KWIDGETJOBTRACKER_P_H
#define KWIDGETJOBTRACKER_P_H

#include <QWidget>
#include <QMap>
#include <QElapsedTimer>
#include <QLabel>
#include <QProgressBar>
#include <QQueue>
#include <QCheckBox>

#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>

class KPushButton;
class KSqueezedTextLabel;

class KWidgetJobTracker::Private
{
public:
    Private(QWidget *_parent, KWidgetJobTracker *tracker)
        : q(tracker)
        , parent(_parent)
    {
    }

    void _k_showProgressWidget();

    class ProgressWidget;

    KWidgetJobTracker *const q;
    QWidget *parent;
    QMap<KJob*, ProgressWidget*> progressWidget;
    QQueue<KJob*> progressWidgetsToBeShown;
};


class KWidgetJobTracker::Private::ProgressWidget
    : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KWidgetJobTracker *object, QWidget *parent)
        : QWidget(parent), tracker(object), job(job), totalSize(0), totalFiles(0), totalDirs(0),
          processedSize(0), processedDirs(0), processedFiles(0), totalSizeKnown(false),
          stopOnClose(true), jobRegistered(false), cancelClose(0), openFile(0),
          openLocation(0), keepOpenCheck(0), pauseButton(0), sourceEdit(0),
          destEdit(0), progressLabel(0), destInvite(0), speedLabel(0), sizeLabel(0),
          resumeLabel(0), progressBar(0), suspendedProperty(false), refCount(1)
    {
        init();
    }

    ~ProgressWidget()
    {
        if (keepOpenCheck->isChecked()) {
            KGlobal::deref();
        }
    }

    KWidgetJobTracker *const tracker;
    KJob *const job;

    qulonglong totalSize;
    qulonglong totalFiles;
    qulonglong totalDirs;
    qulonglong processedSize;
    qulonglong processedDirs;
    qulonglong processedFiles;

    bool totalSizeKnown;
    bool stopOnClose;
    bool jobRegistered;
    QString caption;

    KPushButton *cancelClose;
    KPushButton *openFile;
    KPushButton *openLocation;
    QCheckBox   *keepOpenCheck;
    KUrl        location;
    QElapsedTimer startTime;
    KPushButton *pauseButton;
    KSqueezedTextLabel *sourceEdit;
    KSqueezedTextLabel *destEdit;
    QLabel *progressLabel;
    QLabel *sourceInvite;
    QLabel *destInvite;
    QLabel *speedLabel;
    QLabel *sizeLabel;
    QLabel *resumeLabel;
    QProgressBar *progressBar;
    KPushButton *arrowButton;
    Qt::ArrowType arrowState;

    bool suspendedProperty;

    int refCount; // will not close the dialog if a modal menu is shown

    void init();
    void showTotals();
    void setDestVisible(bool visible);
    void checkDestination(const KUrl &dest);
    void ref();
    void deref();
    void closeNow();

    virtual bool eventFilter(QObject *watched, QEvent *event);

public Q_SLOTS:
    virtual void infoMessage(const QString &plain, const QString &rich);
    virtual void description(const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void processedAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();
    virtual void suspended();
    virtual void resumed();

    //TODO: Misses canResume()

protected:
    void closeEvent(QCloseEvent *event);

private Q_SLOTS:
    void _k_keepOpenToggled(bool);
    void _k_openFile();
    void _k_openLocation();
    void _k_pauseResumeClicked();
    void _k_stop();
    void _k_arrowToggled();
};

#endif // KWIDGETJOBTRACKER_P_H
