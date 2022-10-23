/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

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

#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

static const QString s_kdebugfilepath = QString::fromLatin1("kdebug.log");

static int s_kde_debug_methodname = -1;
static int s_kde_debug_timestamp = -1;
static int s_kde_debug_color = -1;

static QByteArray kDebugHeader(const QByteArray &areaname, const char* const funcinfo)
{
    if (s_kde_debug_methodname == -1) {
        s_kde_debug_methodname = !qgetenv("KDE_DEBUG_METHODNAME").isEmpty();
    }
    if (s_kde_debug_timestamp == -1) {
        s_kde_debug_timestamp = !qgetenv("KDE_DEBUG_TIMESTAMP").isEmpty();
    }

    if (!s_kde_debug_methodname && !s_kde_debug_timestamp) {
        return areaname;
    }

    QByteArray result(areaname);
    if (s_kde_debug_methodname) {
        result.append(" from ");
        const QList<QByteArray> funcinfolist = QByteArray(funcinfo).split(' ');
        bool foundfunc = false;
        foreach (const QByteArray &it, funcinfolist) {
            if (it.contains('(') && it.contains(')')) {
                result.append(it);
                foundfunc = true;
                break;
            }
        }
        if (!foundfunc) {
            result.append(funcinfo);
        }
    }

    if (s_kde_debug_timestamp) {
        static const QString timestamp_format = QString::fromLatin1("hh:mm:ss.zzz");
        const QByteArray timestamp = QDateTime::currentDateTime().time().toString(timestamp_format).toLocal8Bit();
        result.append(" at ");
        result.append(timestamp.constData(), timestamp.size());
    }

    return result;
}

K_GLOBAL_STATIC(QMutex, globalKDebugMutex)


class KDebugDevicesMap : public QMap<uint,QIODevice*>
{
public:
    ~KDebugDevicesMap()
        {
            destroyDevices();
        }

    void destroyDevices()
        {
            foreach (const uint area, keys()) {
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
        m_filepath(s_kdebugfilepath)
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
            if (s_kde_debug_color == -1) {
                s_kde_debug_color = !qgetenv("KDE_DEBUG_COLOR").isEmpty();
            }

            if (s_kde_debug_color) {
                static const bool isttyoutput = ::isatty(::fileno(stderr));
                if (isttyoutput) {
                    switch (m_level) {
                        // for reference:
                        // https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
                        case QtDebugMsg: {
                            ::fprintf(stderr, "\033[0;32m%s: %s\033[0m\n", m_header.constData(), data);
                            ::fflush(stderr);
                            break;
                        }
                        case QtWarningMsg: {
                            ::fprintf(stderr, "\033[0;93m%s: %s\033[0m\n", m_header.constData(), data);
                            ::fflush(stderr);
                            break;
                        }
                        case QtCriticalMsg: {
                            ::fprintf(stderr, "\033[0;33m%s: %s\033[0m\n", m_header.constData(), data);
                            ::fflush(stderr);
                            break;
                        }
                        case QtFatalMsg: {
                            ::fprintf(stderr, "\033[0;31m%s: %s\033[0m\n", m_header.constData(), data);
                            ::fflush(stderr);
                            break;
                        }
                    }
                    return len;
                }
            }

            ::fprintf(stderr, "%s: %s\n", m_header.constData(), data);
            ::fflush(stderr);
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
        : m_level(LOG_INFO),
        m_areaname(areaname)
    { }

    void setLevel(const QtMsgType level)
        { m_level = level; }

    void setHeader(const QByteArray &header)
        { m_header = header; }

protected:
    qint64 writeData(const char* data, qint64 len) final
        {
            ::openlog(m_areaname.constData(), 0, LOG_USER);
            switch (m_level) {
                case QtDebugMsg: {
                    ::syslog(LOG_INFO, "%s: %s", m_header.constData(), data);
                    break;
                }
                case QtWarningMsg: {
                    ::syslog(LOG_WARNING, "%s: %s", m_header.constData(), data);
                    break;
                }
                case QtCriticalMsg: {
                    ::syslog(LOG_CRIT, "%s: %s", m_header.constData(), data);
                    break;
                }
                case QtFatalMsg: {
                    ::syslog(LOG_ERR, "%s: %s", m_header.constData(), data);
                    break;
                }
            }
            ::closelog();
            return len;
        }

private:
    Q_DISABLE_COPY(KDebugSyslogDevice);
    int m_level;
    QByteArray m_header;
    QByteArray m_areaname;
};

class KDebugAreaCache
{
public:
    KDebugAreaCache()
        : infooutput(4),
        warnoutput(2),
        erroroutput(2),
        fataloutput(2),
        infofilename(s_kdebugfilepath),
        warnfilename(s_kdebugfilepath),
        errorfilename(s_kdebugfilepath),
        fatalfilename(s_kdebugfilepath),
        abortfatal(true)
    { }

    int infooutput;
    int warnoutput;
    int erroroutput;
    int fataloutput;
    QString infofilename;
    QString warnfilename;
    QString errorfilename;
    QString fatalfilename;
    bool abortfatal;
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

    void cacheAreas();

    bool disableAll() const;
    QByteArray areaName(const int number) const;
    KDebugAreaCache areaCache(const int number) const;

private:
    Q_DISABLE_COPY(KDebugConfig);
    bool m_disableall;
    QMap<int,QByteArray> m_areanames;
    QMap<int,KDebugAreaCache> m_areacache;
};
K_GLOBAL_STATIC(KDebugConfig, globalKDebugConfig)

KDebugConfig::KDebugConfig()
    : KConfig(QString::fromLatin1("kdebugrc"), KConfig::NoGlobals),
    m_disableall(false)
{
    cacheAreas();
}

void KDebugConfig::cacheAreas()
{
    m_areanames.clear();
    m_areacache.clear();

    KConfigGroup generalgroup = KConfig::group(QString());
    m_disableall = generalgroup.readEntry("DisableAll", false);
    if (m_disableall) {
        return;
    }

    KDebugAreaCache kdebugareacache;
    foreach (const QString &area, KConfig::groupList()) {
        KConfigGroup areagroup = KConfig::group(area);
        kdebugareacache.infooutput = areagroup.readEntry("InfoOutput", int(KDebugConfig::TypeOff));
        kdebugareacache.infofilename = areagroup.readPathEntry("InfoFilename", s_kdebugfilepath);
        kdebugareacache.warnoutput = areagroup.readEntry("WarnOutput", int(KDebugConfig::TypeShell));
        kdebugareacache.warnfilename = areagroup.readPathEntry("WarnFilename", s_kdebugfilepath);
        kdebugareacache.erroroutput = areagroup.readEntry("ErrorOutput", int(KDebugConfig::TypeShell));
        kdebugareacache.errorfilename = areagroup.readPathEntry("ErrorFilename", s_kdebugfilepath);
        kdebugareacache.fataloutput = areagroup.readEntry("FatalOutput", int(KDebugConfig::TypeShell));
        kdebugareacache.fatalfilename = areagroup.readPathEntry("FatalFilename", s_kdebugfilepath);
        kdebugareacache.abortfatal = areagroup.readEntry("AbortFatal", true);
        m_areacache.insert(area.toInt(), kdebugareacache);
    }

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

bool KDebugConfig::disableAll() const
{
    return m_disableall;
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

KDebugAreaCache KDebugConfig::areaCache(const int number) const
{
    return m_areacache.value(number);
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

void kClearDebugConfig()
{
    QMutexLocker locker(globalKDebugMutex);

    globalKDebugDevices->destroyDevices();

    globalKDebugConfig->reparseConfiguration();
    globalKDebugConfig->cacheAreas();

    s_kde_debug_methodname = -1;
    s_kde_debug_timestamp = -1;
    s_kde_debug_color = -1;
}

QDebug KDebug(const QtMsgType type, const char* const funcinfo, const int area)
{
    QMutexLocker locker(globalKDebugMutex);

    if (globalKDebugConfig->disableAll()) {
        return QDebug(globalKDebugNullDevie);
    }

    const KDebugAreaCache kdebugareacache = globalKDebugConfig->areaCache(area);
    int areaoutput = int(KDebugConfig::TypeShell);
    QString areafilename;
    // TODO: abort when? can't show message box and abort immediately
    bool areaabort = true;
    switch (type) {
        case QtDebugMsg: {
            areaoutput = kdebugareacache.infooutput;
            areafilename = kdebugareacache.infofilename;
            break;
        }
        case QtWarningMsg: {
            areaoutput = kdebugareacache.warnoutput;
            areafilename = kdebugareacache.warnfilename;
            break;
        }
        case QtCriticalMsg: {
            areaoutput = kdebugareacache.erroroutput;
            areafilename = kdebugareacache.errorfilename;
            break;
        }
        case QtFatalMsg: {
            areaoutput = kdebugareacache.fataloutput;
            areafilename = kdebugareacache.fatalfilename;
            areaabort = kdebugareacache.abortfatal;
            break;
        }
    }

    const uint areakey = (area << 8 | areaoutput);
    switch (areaoutput) {
        case KDebugConfig::TypeFile: {
            QIODevice* qiodevice = globalKDebugDevices->value(areakey, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugFileDevice();
                globalKDebugDevices->insert(areakey, qiodevice);
            }
            KDebugFileDevice* kdebugdevice = qobject_cast<KDebugFileDevice*>(qiodevice);
            kdebugdevice->setLevel(type);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), funcinfo));
            kdebugdevice->setFilepath(areafilename);
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeMessageBox: {
            QIODevice* qiodevice = globalKDebugDevices->value(areakey, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugMessageBoxDevice();
                globalKDebugDevices->insert(areakey, qiodevice);
            }
            KDebugMessageBoxDevice* kdebugdevice = qobject_cast<KDebugMessageBoxDevice*>(qiodevice);
            kdebugdevice->setLevel(type);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeShell: {
            QIODevice* qiodevice = globalKDebugDevices->value(areakey, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugShellDevice();
                globalKDebugDevices->insert(areakey, qiodevice);
            }
            KDebugShellDevice* kdebugdevice = qobject_cast<KDebugShellDevice*>(qiodevice);
            kdebugdevice->setLevel(type);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeSyslog: {
            QIODevice* qiodevice = globalKDebugDevices->value(areakey, nullptr);
            if (!qiodevice) {
                qiodevice = new KDebugSyslogDevice(globalKDebugConfig->areaName(area));
                globalKDebugDevices->insert(areakey, qiodevice);
            }
            KDebugSyslogDevice* kdebugdevice = qobject_cast<KDebugSyslogDevice*>(qiodevice);
            kdebugdevice->setLevel(type);
            kdebugdevice->setHeader(kDebugHeader(globalKDebugConfig->areaName(area), funcinfo));
            return QDebug(kdebugdevice);
        }
        case KDebugConfig::TypeOff:
        default: {
            return QDebug(globalKDebugNullDevie);
        }
    }
    Q_UNREACHABLE();
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
