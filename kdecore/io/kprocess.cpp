/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

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

#include "kprocess_p.h"

#include <kstandarddirs.h>
#include <kshell.h>

#include <qfile.h>

# include <unistd.h>
# include <errno.h>

# define STD_OUTPUT_HANDLE 1
# define STD_ERROR_HANDLE 2


void KProcessPrivate::writeAll(const QByteArray &buf, int fd)
{
    int off = 0;
    do {
        int ret = ::write(fd, buf.data() + off, buf.size() - off);
        if (ret < 0) {
            if (errno != EINTR)
                return;
        } else {
            off += ret;
        }
    } while (off < buf.size());
}

void KProcessPrivate::forwardStd(KProcess::ProcessChannel good, int fd)
{
    Q_Q(KProcess);

    QProcess::ProcessChannel oc = q->readChannel();
    q->setReadChannel(good);
    writeAll(q->readAll(), fd);
    q->setReadChannel(oc);
}

void KProcessPrivate::_k_forwardStdout()
{
    forwardStd(KProcess::StandardOutput, STD_OUTPUT_HANDLE);
}

void KProcessPrivate::_k_forwardStderr()
{
    forwardStd(KProcess::StandardError, STD_ERROR_HANDLE);
}

/////////////////////////////
// public member functions //
/////////////////////////////

KProcess::KProcess(QObject *parent) :
    QProcess(parent),
    d_ptr(new KProcessPrivate)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::KProcess(KProcessPrivate *d, QObject *parent) :
    QProcess(parent),
    d_ptr(d)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::~KProcess()
{
    delete d_ptr;
}

void KProcess::setOutputChannelMode(OutputChannelMode mode)
{
    Q_D(KProcess);

    d->outputChannelMode = mode;
    disconnect(this, SIGNAL(readyReadStandardOutput()));
    disconnect(this, SIGNAL(readyReadStandardError()));
    switch (mode) {
    case OnlyStdoutChannel:
        connect(this, SIGNAL(readyReadStandardError()), SLOT(_k_forwardStderr()));
        break;
    case OnlyStderrChannel:
        connect(this, SIGNAL(readyReadStandardOutput()), SLOT(_k_forwardStdout()));
        break;
    default:
        QProcess::setProcessChannelMode((ProcessChannelMode)mode);
        return;
    }
    QProcess::setProcessChannelMode(QProcess::SeparateChannels);
}

KProcess::OutputChannelMode KProcess::outputChannelMode() const
{
    Q_D(const KProcess);

    return d->outputChannelMode;
}

void KProcess::setNextOpenMode(QIODevice::OpenMode mode)
{
    Q_D(KProcess);

    d->openMode = mode;
}

#define DUMMYENV "_KPROCESS_DUMMY_="

void KProcess::clearEnvironment()
{
    setEnvironment(QStringList() << QString::fromLatin1(DUMMYENV));
}

void KProcess::setEnv(const QString &name, const QString &value, bool overwrite)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            if (overwrite) {
                *it = fname.append(value);
                setEnvironment(env);
            }
            return;
        }
    env.append(fname.append(value));
    setEnvironment(env);
}

void KProcess::unsetEnv(const QString &name)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            env.erase(it);
            if (env.isEmpty())
                env.append(QString::fromLatin1(DUMMYENV));
            setEnvironment(env);
            return;
        }
}

void KProcess::setProgram(const QString &exe, const QStringList &args)
{
    Q_D(KProcess);

    d->prog = exe;
    d->args = args;
}

void KProcess::setProgram(const QStringList &argv)
{
    Q_D(KProcess);

    Q_ASSERT( !argv.isEmpty() );
    d->args = argv;
    d->prog = d->args.takeFirst();
}

KProcess &KProcess::operator<<(const QString &arg)
{
    Q_D(KProcess);

    if (d->prog.isEmpty())
        d->prog = arg;
    else
        d->args << arg;
    return *this;
}

KProcess &KProcess::operator<<(const QStringList &args)
{
    Q_D(KProcess);

    if (d->prog.isEmpty())
        setProgram(args);
    else
        d->args << args;
    return *this;
}

void KProcess::clearProgram()
{
    Q_D(KProcess);

    d->prog.clear();
    d->args.clear();
}

void KProcess::setShellCommand(const QString &cmd)
{
    Q_D(KProcess);

    KShell::Errors err;
    d->args = KShell::splitArgs(
            cmd, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::NoError && !d->args.isEmpty()) {
        d->prog = KStandardDirs::findExe(d->args[0]);
        if (!d->prog.isEmpty()) {
            d->args.removeFirst();
            return;
        }
    }

    d->args.clear();

#ifdef Q_OS_UNIX
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
# if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
    // If /bin/sh is a symlink, we can be pretty sure that it points to a
    // POSIX shell - the original bourne shell is about the only non-POSIX
    // shell still in use and it is always installed natively as /bin/sh.
    d->prog = QFile::symLinkTarget(QString::fromLatin1("/bin/sh"));
    if (d->prog.isEmpty()) {
        // Try some known POSIX shells.
        d->prog = KStandardDirs::findExe(QString::fromLatin1("ksh"));
        if (d->prog.isEmpty()) {
            d->prog = KStandardDirs::findExe(QString::fromLatin1("ash"));
            if (d->prog.isEmpty()) {
                d->prog = KStandardDirs::findExe(QString::fromLatin1("bash"));
                if (d->prog.isEmpty()) {
                    d->prog = KStandardDirs::findExe(QString::fromLatin1("zsh"));
                    if (d->prog.isEmpty())
                        // We're pretty much screwed, to be honest ...
                        d->prog = QString::fromLatin1("/bin/sh");
                }
            }
        }
    }
# else
    d->prog = QString::fromLatin1("/bin/sh");
# endif

    d->args << QString::fromLatin1("-c") << cmd;
#else // Q_OS_UNIX
    // KMacroExpander::expandMacrosShellQuote(), KShell::quoteArg() and
    // KShell::joinArgs() may generate these for security reasons.
    setEnv(PERCENT_VARIABLE, QLatin1String("%"));

    WCHAR sysdir[MAX_PATH + 1];
    UINT size = GetSystemDirectoryW(sysdir, MAX_PATH + 1);
    d->prog = QString::fromUtf16((const ushort *) sysdir, size);
    d->prog += QLatin1String("\\cmd.exe");
    setNativeArguments(QLatin1String("/V:OFF /S /C \"") + cmd + QLatin1Char('"'));
#endif
}

QStringList KProcess::program() const
{
    Q_D(const KProcess);

    QStringList argv = d->args;
    argv.prepend(d->prog);
    return argv;
}

void KProcess::start()
{
    Q_D(KProcess);

    QProcess::start(d->prog, d->args, d->openMode);
}

int KProcess::execute(int msecs)
{
    start();
    if (!waitForFinished(msecs)) {
        kill();
        waitForFinished(-1);
        return -2;
    }
    return (exitStatus() == QProcess::NormalExit) ? exitCode() : -1;
}

// static
int KProcess::execute(const QString &exe, const QStringList &args, int msecs)
{
    KProcess p;
    p.setProgram(exe, args);
    return p.execute(msecs);
}

// static
int KProcess::execute(const QStringList &argv, int msecs)
{
    KProcess p;
    p.setProgram(argv);
    return p.execute(msecs);
}

int KProcess::startDetached()
{
    Q_D(KProcess);

    qint64 pid;
    if (!QProcess::startDetached(d->prog, d->args, workingDirectory(), &pid))
        return 0;
    return (int) pid;
}

// static
int KProcess::startDetached(const QString &exe, const QStringList &args)
{
    qint64 pid;
    if (!QProcess::startDetached(exe, args, QString(), &pid))
        return 0;
    return (int) pid;
}

// static
int KProcess::startDetached(const QStringList &argv)
{
    QStringList args = argv;
    QString prog = args.takeFirst();
    return startDetached(prog, args);
}

int KProcess::pid() const
{
#ifdef Q_OS_UNIX
    return (int) QProcess::pid();
#else
    return QProcess::pid() ? QProcess::pid()->dwProcessId : 0;
#endif
}

#include "moc_kprocess.cpp"
