/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

#include "config.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kmessage.h"
#include "kstandarddirs.h"
#include "kcomponentdata.h"
#include "kdatetime.h"
#include "kurl.h"

#include <QCoreApplication>
#include <QFile>
#include <QMutex>

#include <stdio.h>
#include <syslog.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

static QByteArray kDebugHeader(const QByteArray &areaname, const char* const file, const int line, const char* const funcinfo)
{
    // TODO: KDE_DEBUG_METHODNAME, KDE_COLOR_DEBUG
    Q_UNUSED(file);
    Q_UNUSED(line);
    Q_UNUSED(funcinfo);

    static const bool kde_debug_timestamp = !qgetenv("KDE_DEBUG_TIMESTAMP").isEmpty();
    if (kde_debug_timestamp) {
        static const QString timestamp_format = QString::fromLatin1("hh:mm:ss.zzz");
        const QByteArray timestamp = QDateTime::currentDateTime().time().toString(timestamp_format).toLocal8Bit();

        QByteArray result(areaname);
        result.append(" at ");
        result.append(timestamp.constData(), timestamp.size());
        return result;
    }

    return areaname;
}

K_GLOBAL_STATIC(QMutex, globalKDebugMutex)


class KDebugDevicesMap : public QMap<int,QIODevice*>
{
public:
    ~KDebugDevicesMap()
        {
            foreach (const int area, keys()) {
                QIODevice* qiodevice = take(area);
                delete qiodevice;
            }
        }
};
K_GLOBAL_STATIC(KDebugDevicesMap, globalKDebugDevices)


class KDebugNullDevice: public QIODevice
{
    Q_OBJECT
public:
    KDebugNullDevice() { open(QIODevice::WriteOnly); }

protected:
    qint64 readData(char*, qint64) final
        { return 0; /* eof */ }
    qint64 writeData(const char*, qint64 len)
        { return len; }
};
K_GLOBAL_STATIC(KDebugNullDevice, globalKDebugNullDevie)


class KDebugFileDevice: public KDebugNullDevice
{
    Q_OBJECT
public:
    KDebugFileDevice()
        : m_level(QtDebugMsg),
        m_filepath(QString::fromLatin1("kdebug.log"))
        { }

    void setLevel(const QtMsgType level)
        { m_level = level; }
    void setHeader(const QByteArray &header)
        { m_header = header; }
    void setFilepath(const QString &filepath)
        { m_filepath = filepath; }

protected:
    qint64 writeData(const char* data, qint64 len) final
        {
            QFile writefile(m_filepath);
            if (!writefile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered)) {
                return 0;
            }
            // TODO: m_level
            writefile.write(m_header.constData(), m_header.size());
            writefile.write(": ", 2);
            writefile.write(data, len);
            writefile.write("\n", 1);
            return len;
        }

private:
    Q_DISABLE_COPY(KDebugFileDevice);

    int m_level;
    QByteArray m_header;
    QString m_filepath;
};

class KDebugMessageBoxDevice: public KDebugNullDevice
{
    Q_OBJECT
public:
    KDebugMessageBoxDevice()
        : m_level(QtDebugMsg)
        { }

    void setLevel(const QtMsgType level)
        { m_level = level; }
    void setHeader(const QByteArray &header)
        { m_header = header; }

protected:
    qint64 writeData(const char* data, qint64 len) final
        {
            const QString text = QString::fromLatin1("%1: %2").arg(
                QString::fromLocal8Bit(m_header.constData(), m_header.size()),
                QString::fromLocal8Bit(data, len)
            );
            switch (m_level) {
                case QtDebugMsg: {
                    KMessage::message(KMessage::Information, text);
                    break;
                }
                case QtWarningMsg: {
                    KMessage::message(KMessage::Warning, text);
                    break;
                }
                case QtCriticalMsg: {
                    KMessage::message(KMessage::Error, text);
                    break;
                }
                case QtFatalMsg: {
                    KMessage::message(KMessage::Fatal, text);
                    break;
                }
            }
            return len;
        }

private:
    Q_DISABLE_COPY(KDebugMessageBoxDevice);

    int m_level;
    QByteArray m_header;
};

class KDebugShellDevice: public KDebugNullDevice
{
    Q_OBJECT
public:
    KDebugShellDevice()
        : m_level(QtDebugMsg)
        { }

    void setLevel(const QtMsgType level)
        { m_level = level; }
    void setHeader(const QByteArray &header)
        { m_header = header; }

protected:
    qint64 writeData(const char* data, qint64 len) final
        {
            if (m_level == QtDebugMsg) {
                ::fprintf(stdout, "%s: %s\n", m_header.constData(), data);
            } else {
                ::fprintf(stderr, "%s: %s\n", m_header.constData(), data);
            }
            return len;
        }

private:
    Q_DISABLE_COPY(KDebugShellDevice);

    int m_level;
    QByteArray m_header;
};

class KDebugSyslogDevice: public KDebugNullDevice
{
    Q_OBJECT
public:
    KDebugSyslogDevice(const QByteArray &areaname)
        : m_level(LOG_INFO)
    { ::openlog(areaname.constData(), 0, LOG_USER); }

    void setLevel(const QtMsgType level)
        {
            switch (level) {
                case QtDebugMsg: {
                    m_level = LOG_INFO;
                    break;
                }
                case QtWarningMsg: {
                    m_level = LOG_WARNING;
                    break;
                }
                case QtCriticalMsg: {
                    m_level = LOG_CRIT;
                    break;
                }
                case QtFatalMsg: {
                    m_level = LOG_ERR;
                    break;
                }
            }
        }

    void setHeader(const QByteArray &header)
        { m_header = header; }

protected:
    qint64 writeData(const char* data, qint64 len) final
        {
            ::syslog(m_level, "%s: %s", m_header.constData(), data);
            return len;
        }

private:
    Q_DISABLE_COPY(KDebugSyslogDevice);

    int m_level;
    QByteArray m_header;
};


class KDebugConfig : public KConfig
{
public:
    enum KDebugType {
        TypeFile = 0,
        TypeMessageBox = 1,
        TypeShell = 2,
        TypeSyslog = 3,
        TypeOff = 4
    };

    KDebugConfig();

    void readAreas();

    QByteArray areaName(const int number) const;

private:
    Q_DISABLE_COPY(KDebugConfig);
    QMap<int,QByteArray> m_areanames;
};
K_GLOBAL_STATIC(KDebugConfig, globalKDebugConfig)

KDebugConfig::KDebugConfig()
    : KConfig(QString::fromLatin1("kdebugrc"), KConfig::NoGlobals)
{
    readAreas();
}

void KDebugConfig::readAreas()
{
    m_areanames.clear();

    const QString kdebugareas = KStandardDirs::locate("config", QString::fromLatin1("kdebug.areas"));
    if (kdebugareas.isEmpty()) {
        return;
    }

    QFile kdebugareasfile(kdebugareas);
    if (!kdebugareasfile.open(QFile::ReadOnly)) {
        return;
    }
    while (!kdebugareasfile.atEnd()) {
        QByteArray kdebugareasline = kdebugareasfile.readLine().trimmed();
        if (kdebugareasline.isEmpty() || kdebugareasline.startsWith('#')) {
            continue;
        }

        const int spaceindex = kdebugareasline.indexOf(' ');
        if (spaceindex < 1) {
            continue;
        }

        const int areanumber = kdebugareasline.mid(0, spaceindex).toLongLong();
        const QByteArray areaname = kdebugareasline.mid(spaceindex + 1, kdebugareasline.size() - spaceindex - 1).trimmed();
        if (areanumber <= 0 || areaname.isEmpty()) {
            continue;
        }

        m_areanames.insert(areanumber, areaname);

        // qDebug() << Q_FUNC_INFO << areanumber << areaname;
    }
}

QByteArray KDebugConfig::areaName(const int number) const
{
    const QByteArray areaname = m_areanames.value(number);
    if (!areaname.isEmpty()) {
        return areaname;
    }

    if (KGlobal::hasMainComponent()) {
        return KGlobal::mainComponent().componentName().toUtf8();
    }
    return QCoreApplication::applicationName().toUtf8();
}


QString kBacktrace(int levels)
{
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    if (!n)
        return QString();
    char** strings = backtrace_symbols(trace, n);

    if (levels != -1) {
        n = qMin(n, levels);
    }

    QString s = QString::fromLatin1("[\n");
    for (int i = 0; i < n; ++i) {
        s += QString::number(i) + QLatin1String(": ") +
             QString::fromLatin1(strings[i]) + QLatin1Char('\n');
    }
    s += QLatin1String("]\n");

    if (strings) {
        ::free(strings);
    }

    return s;
#else
    return QString();
#endif // HAVE_BACKTRACE
}

QDebug kDebugStream(QtMsgType level, int area, const char *file, int line, const char *funcinfo)
{
    QMutexLocker locker(globalKDebugMutex);

    KConfigGroup generalgroup = globalKDebugConfig->group(QString());
    const bool disableall = generalgroup.readEntry("DisableAll", false);
    if (disableall) {
        return QDebug(globalKDebugNullDevie);
    }

    KConfigGroup areagroup = globalKDebugConfig->group(QString::number(area));
    int areaoutput = int(KDebugConfig::TypeShell);
    QString areafilename = QString::fromLatin1("kdebug.log");
    // TODO: abort when? can't show message box and abort immediately
    bool areaabort = true;
    switch (level) {
        case QtDebugMsg: {
            areaoutput = areagroup.readEntry("InfoOutput", int(KDebugConfig::TypeOff));
            areafilename = areagroup.readPathEntry("InfoFilename", areafilename);
            break;
        }
        case QtWarningMsg: {
            areaoutput = areagroup.readEntry("WarnOutput", areaoutput);
            areagroup.readPathEntry("WarnFilename", areafilename);
            break;
        }
        case QtCriticalMsg: {
            areaoutput = areagroup.readEntry("ErrorOutput", areaoutput);
            areafilename = areagroup.readPathEntry("ErrorFilename", areafilename);
            break;
        }
        case QtFatalMsg: {
            areaoutput = areagroup.readEntry("FatalOutput", areaoutput);
            areafilename = areagroup.readPathEntry("FatalFilename", areafilename);
            areaabort = areagroup.readEntry("AbortFatal", true);
            break;
        }
    }

    switch (areaoutput) {
        case KDebugConfig::TypeFile: {
            QIODevice* qiodevice = globalKDebugDevices->value(area, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugFileDevice();
                globalKDebugDevices->insert(area, qiodevice);
            }
            KDebugFileDevice* kdebugdevice = qobject_cast<KDebugFileDevice*>(qiodevice);
            kdebugdevice->setLevel(level);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), file, line, funcinfo));
            kdebugdevice->setFilepath(areafilename);
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeMessageBox: {
            QIODevice* qiodevice = globalKDebugDevices->value(area, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugMessageBoxDevice();
                globalKDebugDevices->insert(area, qiodevice);
            }
            KDebugMessageBoxDevice* kdebugdevice = qobject_cast<KDebugMessageBoxDevice*>(qiodevice);
            kdebugdevice->setLevel(level);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), file, line, funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeShell: {
            QIODevice* qiodevice = globalKDebugDevices->value(area, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugShellDevice();
                globalKDebugDevices->insert(area, qiodevice);
            }
            KDebugShellDevice* kdebugdevice = qobject_cast<KDebugShellDevice*>(qiodevice);
            kdebugdevice->setLevel(level);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), file, line, funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeSyslog: {
            QIODevice* qiodevice = globalKDebugDevices->value(area, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugSyslogDevice(globalKDebugConfig->areaName(area));
                globalKDebugDevices->insert(area, qiodevice);
            }
            KDebugSyslogDevice* kdebugdevice = qobject_cast<KDebugSyslogDevice*>(qiodevice);
            kdebugdevice->setLevel(level);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), file, line, funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeOff:
        default: {
            return QDebug(globalKDebugNullDevie);
        }
    }
    Q_UNREACHABLE();
}

void kClearDebugConfig()
{
    QMutexLocker locker(globalKDebugMutex);
    globalKDebugConfig->reparseConfiguration();
    globalKDebugConfig->readAreas();
}

QDebug operator<<(QDebug s, const KDateTime &time)
{
    if ( time.isDateOnly() )
        s.nospace() << "KDateTime(" << qPrintable(time.toString(KDateTime::QtTextDate)) << ")";
    else
        s.nospace() << "KDateTime(" << qPrintable(time.toString(KDateTime::ISODate)) << ")";
    return s.space();
}

QDebug operator<<(QDebug s, const KUrl &url)
{
    s.nospace() << "KUrl(" << url.prettyUrl() << ")";
    return s.space();
}

#include "kdebug.moc"
