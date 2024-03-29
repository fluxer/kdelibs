/*  This file is part of the KDE project
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "kjob.h"
#include "kjob_p.h"

#include "kjobuidelegate.h"

#include <kglobal.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMap>
#include <QMetaType>
#include <QTimer>

class KJobLoop : public QEventLoop
{
    Q_OBJECT
public:
    KJobLoop(QObject *parent);

    int error;

public Q_SLOTS:
    void slotFinished(KJob* job);

private:
    Q_DISABLE_COPY(KJobLoop);
};

KJobLoop::KJobLoop(QObject *parent)
    : QEventLoop(parent),
    error(0)
{
}

void KJobLoop::slotFinished(KJob* job)
{
    error = job->error();
    quit();
}

KJobPrivate::KJobPrivate()
    : q_ptr(0), uiDelegate(0), error(KJob::NoError),
      progressUnit(KJob::Bytes), percentage(0),
      suspended(false), capabilities(KJob::NoCapabilities),
      speedTimer(0), isAutoDelete(true), isFinished(false)
{
    qRegisterMetaType<KJob::Unit>("KJob::Unit");

    processedAmount.fill(0);
    totalAmount.fill(0);
}

KJobPrivate::~KJobPrivate()
{
}

KJob::KJob(QObject *parent)
    : QObject(parent), d_ptr(new KJobPrivate)
{
    d_ptr->q_ptr = this;
    // Don't exit while this job is running
    KGlobal::ref();
}

KJob::KJob(KJobPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    // Don't exit while this job is running
    KGlobal::ref();
}

KJob::~KJob()
{
    if (!d_ptr->isFinished) {
        emit finished(this);
    }

    delete d_ptr->speedTimer;
    delete d_ptr->uiDelegate;
    delete d_ptr;

    KGlobal::deref();
}

void KJob::setUiDelegate( KJobUiDelegate *delegate )
{
    Q_D(KJob);
    if ( delegate == 0 || delegate->setJob( this ) )
    {
        delete d->uiDelegate;
        d->uiDelegate = delegate;

        if ( d->uiDelegate )
        {
            d->uiDelegate->connectJob( this );
        }
    }
}

KJobUiDelegate *KJob::uiDelegate() const
{
    return d_func()->uiDelegate;
}

KJob::Capabilities KJob::capabilities() const
{
    return d_func()->capabilities;
}

bool KJob::isSuspended() const
{
    return d_func()->suspended;
}

bool KJob::kill( KillVerbosity verbosity )
{
    Q_D(KJob);
    if ( doKill() )
    {
        setError( KilledJobError );

        if ( verbosity!=Quietly )
        {
            emitResult();
        }
        else
        {
            // If we are displaying a progress dialog, remove it first.
            d->isFinished = true;
            emit finished(this);

            if ( isAutoDelete() )
                deleteLater();
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool KJob::suspend()
{
    Q_D(KJob);
    if ( !d->suspended )
    {
        if ( doSuspend() )
        {
            d->suspended = true;
            emit suspended(this);

            return true;
        }
    }

    return false;
}

bool KJob::resume()
{
    Q_D(KJob);
    if ( d->suspended )
    {
        if ( doResume() )
        {
            d->suspended = false;
            emit resumed(this);

            return true;
        }
    }

    return false;
}

bool KJob::doKill()
{
    return false;
}

bool KJob::doSuspend()
{
    return false;
}

bool KJob::doResume()
{
    return false;
}

void KJob::setCapabilities( KJob::Capabilities capabilities )
{
    Q_D(KJob);
    d->capabilities = capabilities;
}

bool KJob::exec()
{
    KJobLoop kjobloop(qApp);
    connect(
        this, SIGNAL(finished(KJob*)),
        &kjobloop, SLOT(slotFinished(KJob*)),
        Qt::DirectConnection
    );
    start();
    kjobloop.exec();
    return (kjobloop.error == NoError);
}

int KJob::error() const
{
    return d_func()->error;
}

QString KJob::errorText() const
{
    return d_func()->errorText;
}

QString KJob::errorString() const
{
    return d_func()->errorText;
}

qulonglong KJob::processedAmount(Unit unit) const
{
    return d_func()->processedAmount[unit];
}

qulonglong KJob::totalAmount(Unit unit) const
{
    return d_func()->totalAmount[unit];
}

unsigned long KJob::percent() const
{
    return d_func()->percentage;
}

void KJob::setError( int errorCode )
{
    Q_D(KJob);
    d->error = errorCode;
}

void KJob::setErrorText( const QString &errorText )
{
    Q_D(KJob);
    d->errorText = errorText;
}

void KJob::setProcessedAmount(Unit unit, qulonglong amount)
{
    Q_D(KJob);
    if ( d->processedAmount[unit] != amount )
    {
        d->processedAmount[unit] = amount;
        emit processedAmount(this, unit, amount);
        if (unit==d->progressUnit) {
            emit processedSize(this, amount);
            emitPercent(amount, d->totalAmount[unit]);
        }
    }
}

void KJob::setTotalAmount(Unit unit, qulonglong amount)
{
    Q_D(KJob);
    if ( d->totalAmount[unit] != amount )
    {
        d->totalAmount[unit] = amount;
        emit totalAmount(this, unit, amount);
        if (unit==d->progressUnit) {
            emit totalSize(this, amount);
            emitPercent(d->processedAmount[unit], amount);
        }
    }
}

void KJob::setPercent( unsigned long percentage )
{
    Q_D(KJob);
    if ( d->percentage != percentage )
    {
        d->percentage = percentage;
        emit percent( this, percentage );
    }
}

void KJob::emitResult()
{
    Q_D(KJob);
    d->isFinished = true;

    // If we are displaying a progress dialog, remove it first.
    emit finished( this );

    emit result( this );

    if ( isAutoDelete() )
        deleteLater();
}

void KJob::emitPercent( qulonglong processedAmount, qulonglong totalAmount )
{
    Q_D(KJob);
    // calculate percents
    if (totalAmount) {
        unsigned long oldPercentage = d->percentage;
        d->percentage = (unsigned long)(( (float)(processedAmount) / (float)(totalAmount) ) * 100.0);
        if ( d->percentage != oldPercentage ) {
            emit percent( this, d->percentage );
        }
    }
}

void KJob::emitSpeed(unsigned long value)
{
    Q_D(KJob);
    if (!d->speedTimer) {
        d->speedTimer = new QTimer(this);
        connect(d->speedTimer, SIGNAL(timeout()), SLOT(_k_speedTimeout()));
    }

    emit speed(this, value);
    d->speedTimer->start(5000);   // 5 seconds interval should be enough
}

void KJobPrivate::_k_speedTimeout()
{
    Q_Q(KJob);
    // send 0 and stop the timer
    // timer will be restarted only when we receive another speed event
    emit q->speed(q, 0);
    speedTimer->stop();
}

bool KJob::isAutoDelete() const
{
    Q_D(const KJob);
    return d->isAutoDelete;
}

void KJob::setAutoDelete( bool autodelete )
{
    Q_D(KJob);
    d->isAutoDelete = autodelete;
}

#include "kjob.moc"
#include "moc_kjob.cpp"
