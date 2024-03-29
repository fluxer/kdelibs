/*  This file is part of the KDE project
    Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#include "kuiserverjobtracker.h"
#include "kuiserverjobtracker_p.h"

#include "jobviewiface.h"

#include <klocale.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kjob.h>

K_GLOBAL_STATIC(KSharedUiServerProxy, serverProxy)

class KUiServerJobTracker::Private
{
public:
    Private(KUiServerJobTracker *parent)
        : q(parent)
    {
    }

    KUiServerJobTracker *const q;

    void _k_killJob();

    QHash<KJob*, org::kde::JobView*> progressJobView;
};

void KUiServerJobTracker::Private::_k_killJob()
{
    org::kde::JobView *jobView = qobject_cast<org::kde::JobView*>(q->sender());

    if (jobView) {
        KJob *job = progressJobView.key(jobView);

        if (job)
            job->kill(KJob::EmitResult);
    }
}

KUiServerJobTracker::KUiServerJobTracker(QObject *parent)
    : KJobTrackerInterface(parent),
    d(new Private(this))
{
}

KUiServerJobTracker::~KUiServerJobTracker()
{
    if (!d->progressJobView.isEmpty()) {
        qWarning() << "A KUiServerJobTracker instance contains"
                   << d->progressJobView.size() << "stalled jobs";
    }

    delete d;
}

void KUiServerJobTracker::registerJob(KJob *job)
{
    // Already registered job?
    if (d->progressJobView.contains(job)) {
        return;
    }

    KComponentData componentData = KGlobal::mainComponent();
    QString appName = job->property("appName").toString();
    if (appName.isEmpty()) {
        appName = componentData.aboutData()->programName();
    }
    QString appIconName = job->property("appIconName").toString();
    if (appIconName.isEmpty()) {
        appIconName = componentData.aboutData()->programIconName();
    }
    if (appIconName.isEmpty()) {
        appIconName = componentData.aboutData()->appName();
    }

    QWeakPointer<KJob> jobWatch = job;
    QDBusReply<QDBusObjectPath> reply = serverProxy->uiserver().requestView(appName,
                                                                            appIconName,
                                                                            job->capabilities());

    // If we got a valid reply, register the interface for later usage.
    if (reply.isValid()) {
        org::kde::JobView *jobView = new org::kde::JobView("org.kde.JobViewServer",
                                                           reply.value().path(),
                                                           QDBusConnection::sessionBus());
        if (!jobWatch) {
            //kDebug() << "deleted out from under us when asking the server proxy for the view";
            jobView->terminate(QString());
            delete jobView;
            return;
        }

        QObject::connect(jobView, SIGNAL(cancelRequested()), this,
                         SLOT(_k_killJob()));
        QObject::connect(jobView, SIGNAL(suspendRequested()), job,
                         SLOT(suspend()));
        QObject::connect(jobView, SIGNAL(resumeRequested()), job,
                         SLOT(resume()));

        QVariant destUrl = job->property("destUrl");
        if (destUrl.isValid()) {
            jobView->setDestUrl(destUrl.toString());
        }

        if (!jobWatch) {
            //kDebug() << "deleted out from under us when creating the dbus interface";
            jobView->terminate(QString());
            delete jobView;
            return;
        }

        d->progressJobView.insert(job, jobView);
    } else if (!jobWatch) {
        qWarning() << "Uh-oh...KUiServerJobTracker was trying to forward a job, but it was deleted from under us."
        << "kuiserver *may* have a stranded job. we can't do anything about it because the returned objectPath is invalid.";
        return;
    }

    KJobTrackerInterface::registerJob(job);
}

void KUiServerJobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);

    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView.take(job);

    if (job->error()) {
        jobView->terminate(job->errorText());
    } else {
        jobView->terminate(QString());
    }

    delete jobView;
}

void KUiServerJobTracker::finished(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView.take(job);

    if (job->error()) {
        jobView->terminate(job->errorText());
    } else {
        jobView->terminate(QString());
    }
}

void KUiServerJobTracker::suspended(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setSuspended(true);
}

void KUiServerJobTracker::resumed(KJob *job)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setSuspended(false);
}

void KUiServerJobTracker::description(KJob *job, const QString &title,
                                      const QPair<QString, QString> &field1,
                                      const QPair<QString, QString> &field2)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setInfoMessage(title);

    if (field1.first.isNull() || field1.second.isNull()) {
        jobView->clearDescriptionField(0);
    } else {
        jobView->setDescriptionField(0, field1.first, field1.second);
    }

    if (field2.first.isNull() || field2.second.isNull()) {
        jobView->clearDescriptionField(1);
    } else {
        jobView->setDescriptionField(1, field2.first, field2.second);
    }
}

void KUiServerJobTracker::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(rich)

    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setInfoMessage(plain);
}

void KUiServerJobTracker::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    switch (unit) {
    case KJob::Bytes:
        jobView->setTotalAmount(amount, "bytes");
        break;
    case KJob::Files:
        jobView->setTotalAmount(amount, "files");
        break;
    case KJob::Directories:
        jobView->setTotalAmount(amount, "dirs");
        break;
    default:
        break;
    }
}

void KUiServerJobTracker::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    switch (unit) {
    case KJob::Bytes:
        jobView->setProcessedAmount(amount, "bytes");
        break;
    case KJob::Files:
        jobView->setProcessedAmount(amount, "files");
        break;
    case KJob::Directories:
        jobView->setProcessedAmount(amount, "dirs");
        break;
    default:
        break;
    }
}

void KUiServerJobTracker::percent(KJob *job, unsigned long percent)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setPercent(percent);
}

void KUiServerJobTracker::speed(KJob *job, unsigned long value)
{
    if (!d->progressJobView.contains(job)) {
        return;
    }

    org::kde::JobView *jobView = d->progressJobView[job];

    jobView->setSpeed(value);
}

KSharedUiServerProxy::KSharedUiServerProxy()
    : m_uiserver("org.kde.JobViewServer", "/JobViewServer", QDBusConnection::sessionBus())
{
    QDBusConnectionInterface* sessionIface = QDBusConnection::sessionBus().interface();
    if (!sessionIface->isServiceRegistered("org.kde.JobViewServer")) {
        QDBusReply<void> sessionReply = sessionIface->startService("org.kde.kuiserver");
        if (!sessionReply.isValid()) {
            kError() << "Couldn't start kuiserver service" << sessionReply.error();
        }
    }

    if (!sessionIface->isServiceRegistered("org.kde.JobViewServer"))
        kError() << "The service org.kde.JobViewServer is still not registered";
    else
        kDebug() << "kuiserver registered";
}

KSharedUiServerProxy::~KSharedUiServerProxy()
{
}

org::kde::JobViewServer &KSharedUiServerProxy::uiserver()
{
    return m_uiserver;
}

#include "moc_kuiserverjobtracker.cpp"
#include "moc_kuiserverjobtracker_p.cpp"
