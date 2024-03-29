// -*- mode: c++; c-basic-offset: 2 -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRUN_P_H
#define KRUN_P_H

#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "kprocess.h"
#include "kstartupinfo.h"

/**
 * @internal
 * This class watches a process launched by KRun.
 * It sends a notification when the process exits (for the taskbar)
 * and it will show an error message if necessary (e.g. "program not found").
 */
class KProcessRunner : public QObject
{
  Q_OBJECT

  public:

#ifndef Q_WS_X11
    static qint64 run(KProcess *, const QString & executable);
#else
    static qint64 run(KProcess *, const QString & executable, const KStartupInfoId& id);
#endif

    virtual ~KProcessRunner();

    qint64 pid() const;

  protected Q_SLOTS:

    void slotProcessExited(int, QProcess::ExitStatus);

  private:
#ifndef Q_WS_X11
    KProcessRunner(KProcess *, const QString & binName);
#else
    KProcessRunner(KProcess *, const QString & binName, const KStartupInfoId& id);
#endif

    void terminateStartupNotification();

    KProcess *m_process;
    QString m_executable; // can be a full path
    KStartupInfoId m_id;
    qint64 m_pid;

    Q_DISABLE_COPY(KProcessRunner)
};

/**
 * @internal
 */
class KRun::KRunPrivate
{
public:
    KRunPrivate(KRun *parent);

    void init (const KUrl& url, QWidget* window, mode_t mode,
               bool isLocalFile, bool showProgressInfo, const QByteArray& asn);

    // This helper method makes debugging easier: a single breakpoint for all
    // the code paths that start the timer - at least from KRun itself.
    // TODO: add public method startTimer() and deprecate timer() accessor,
    // starting is the only valid use of the timer in subclasses
    void startTimer();

    bool runExecutable(const QString& _exec);

    KRun *q;
    bool m_showingDialog;
    bool m_runExecutables;

    QString m_preferredService;
    QString m_localPath;
    QString m_suggestedFileName;
    QPointer <QWidget> m_window;
    QByteArray m_asn;
    KUrl m_strURL;
    bool m_bFault;
    bool m_bAutoDelete;
    bool m_bProgressInfo;
    bool m_bFinished;
    KIO::Job * m_job;
    QTimer m_timer;

    /**
     * Used to indicate that the next action is to scan the file.
     * This action is invoked from slotTimeout.
     */
    bool m_bScanFile;
    bool m_bIsDirectory;

    /**
     * Used to indicate that the next action is to initialize.
     * This action is invoked from slotTimeout
     */
    bool m_bInit;

    bool m_bIsLocalFile;
    mode_t m_mode;
};

#endif  // KRUN_P_H
