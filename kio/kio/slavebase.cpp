/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (c) 2000 David Faure <faure@kde.org>
 *  Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
 *  Copyright (c) 2007 Thiago Macieira <thiago@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 **/

#include "slavebase.h"

#include <config.h>

#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QElapsedTimer>
#include <QtCore/QCoreApplication>

#include "kdebug.h"
#include "kcrash.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kde_file.h"
#include "klocale.h"
#include "kpassworddialog.h"
#include "kwindowsystem.h"
#include "kpasswdstore.h"
#include "kremoteencoding.h"
#include "connection.h"
#include "ioslave_defaults.h"
#include "slaveinterface.h"
#include "job_p.h"

#define AUTHINFO_EXTRAFIELD_DOMAIN QLatin1String("domain")
#define AUTHINFO_EXTRAFIELD_ANONYMOUS QLatin1String("anonymous")
#define AUTHINFO_EXTRAFIELD_HIDE_USERNAME_INPUT QLatin1String("hide-username-line")

extern "C" {
    static void sigpipe_handler(int sig);
}

using namespace KIO;

typedef QList<QByteArray> AuthKeysList;
typedef QMap<QString,QByteArray> AuthKeysMap;
#define KIO_DATA QByteArray data; QDataStream stream( &data, QIODevice::WriteOnly ); stream
#define KIO_FILESIZE_T(x) quint64(x)

namespace KIO {

static const int s_quit_signals[] = {
    SIGTERM,
    SIGHUP,
    SIGINT,
    0
};

static QByteArray authInfoKey(const AuthInfo &authinfo)
{
    return KPasswdStore::makeKey(authinfo.url.prettyUrl());
}

static QString authInfoToData(const AuthInfo &authinfo)
{
    QByteArray authdata;
    QDataStream authstream(&authdata, QIODevice::WriteOnly);
    authstream << authinfo;
    return QString::fromLatin1(authdata.toHex());
}

static AuthInfo authInfoFromData(const QByteArray &authdata)
{
    QBuffer authbuffer;
    authbuffer.setData(QByteArray::fromHex(authdata));
    authbuffer.open(QBuffer::ReadOnly);
    AuthInfo authinfo;
    QDataStream authstream(&authbuffer);
    authstream >> authinfo;
    return authinfo;
}

class SlaveBasePrivate {
public:
    SlaveBasePrivate(const QByteArray &protocol);
    ~SlaveBasePrivate();

    UDSEntryList pendingListEntries;
    QElapsedTimer m_timeSinceLastBatch;
    Connection appConnection;

    bool needSendCanResume;
    bool wasKilled;
    bool exit_loop;
    MetaData configData;
    KConfig *config;
    KConfigGroup *configGroup;

    struct timeval last_tv;
    KIO::filesize_t totalSize;
    KRemoteEncoding *remotefile;
    time_t timeout;
    enum { Idle, InsideMethod, FinishedCalled, ErrorCalled } m_state;
    QByteArray timeoutData;

    KPasswdStore* m_passwdStore;

    QByteArray m_protocol;
    // Often used by slaves and unlikely to change
    MetaData m_outgoingMetaData;
    MetaData m_incomingMetaData;

    // Reconstructs configGroup from configData and m_incomingMetaData
    void rebuildConfig()
    {
        configGroup->deleteGroup(KConfigGroup::WriteConfigFlags());

        // m_incomingMetaData cascades over config, so we write config first,
        // to let it be overwritten
        MetaData::ConstIterator end = configData.constEnd();
        for (MetaData::ConstIterator it = configData.constBegin(); it != end; ++it) {
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());
        }

        end = m_incomingMetaData.constEnd();
        for (MetaData::ConstIterator it = m_incomingMetaData.constBegin(); it != end; ++it) {
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());
        }
    }

    void verifyState(const char* cmdName)
    {
        if ((m_state != SlaveBasePrivate::FinishedCalled) && (m_state != SlaveBasePrivate::ErrorCalled)){
            kWarning(7019) << m_protocol << cmdName << "did not call finished() or error()! Please fix the KIO slave.";
        }
    }

    void verifyErrorFinishedNotCalled(const char* cmdName)
    {
        if (m_state == SlaveBasePrivate::FinishedCalled || m_state == SlaveBasePrivate::ErrorCalled) {
            kWarning(7019) << m_protocol << cmdName << "called finished() or error(), but it's not supposed to! Please fix the KIO slave.";
        }
    }

    KPasswdStore* passwdStore()
    {
        if (!m_passwdStore) {
            m_passwdStore = new KPasswdStore();
            m_passwdStore->setStoreID("KIO");
        }

        return m_passwdStore;
    }
};

SlaveBasePrivate::SlaveBasePrivate(const QByteArray &protocol)
    : needSendCanResume(false),
    wasKilled(false),
    exit_loop(false),
    config(nullptr),
    configGroup(nullptr),
    totalSize(0),
    remotefile(nullptr),
    timeout(0),
    m_passwdStore(nullptr),
    m_protocol(protocol)
{
    config = new KConfig(QString(), KConfig::SimpleConfig);
    // The KConfigGroup needs the KConfig to exist during its whole lifetime.
    configGroup = new KConfigGroup(config, QString());
    last_tv.tv_sec = 0;
    last_tv.tv_usec = 0;
}

SlaveBasePrivate::~SlaveBasePrivate()
{
    delete m_passwdStore;
}

}

static SlaveBase *globalSlave = nullptr;

static volatile bool slaveWriteError = false;

static void genericsig_handler(int sigNumber)
{
    KDE_signal(sigNumber, SIG_DFL);

    kDebug(7019) << "exiting due to signal" << sigNumber;
    // set the flag which will be checked in dispatchLoop() and which *should* be checked
    // in lengthy operations in the various slaves
    if (globalSlave) {
        globalSlave->setKillFlag();
    }
}

//////////////

SlaveBase::SlaveBase(const QByteArray &protocol,
                     const QByteArray &app_socket)
    : d(new SlaveBasePrivate(protocol))

{
    if (qgetenv("KDE_DEBUG").isEmpty()) {
        KCrash::setFlags(KCrash::CrashFlags(KCrash::Notify | KCrash::Log));
    }

    struct sigaction act;
    act.sa_handler = sigpipe_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE, &act, 0);

    sigset_t handlermask;
    ::sigemptyset(&handlermask);
    int counter = 0;
    while (s_quit_signals[counter]) {
        KDE_signal(s_quit_signals[counter], genericsig_handler);
        ::sigaddset(&handlermask, s_quit_signals[counter]);
        counter++;
    }
    ::sigprocmask(SIG_UNBLOCK, &handlermask, NULL);

    globalSlave = this;

    const QString address = QFile::decodeName(app_socket);
    d->appConnection.connectToRemote(address);
    if (!d->appConnection.inited()) {
        kDebug(7019) << "failed to connect to" << address << '\n'
                     << "Reason:" << d->appConnection.errorString();
        exit();
    }
}

SlaveBase::~SlaveBase()
{
    delete d->configGroup;
    delete d->config;
    delete d->remotefile;
    delete d;
}

void SlaveBase::dispatchLoop()
{
    while (!d->exit_loop) {
        if (d->timeout && (d->timeout < time(0))) {
            QByteArray data = d->timeoutData;
            d->timeout = 0;
            d->timeoutData = QByteArray();
            special(data);
        }

        Q_ASSERT(d->appConnection.inited());

        int ms = -1;
        if (d->timeout) {
            ms = 1000 * qMax<time_t>(d->timeout - time(0), 1);
        }

        int ret = -1;
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(ms)) {
            // dispatch application messages
            int cmd;
            QByteArray data;
            ret = d->appConnection.read(&cmd, data);

            if (ret != -1) {
                dispatch(cmd, data);
            }
        }

        if (ret == -1 || !d->appConnection.isConnected()) {
            // some error occurred or not connected to application socket
            break;
        }

        // I think we get here when we were killed in dispatch() and not in select()
        if (wasKilled()) {
            kDebug(7019) << "slave was killed, returning";
            break;
        }

        // execute deferred deletes
        QCoreApplication::sendPostedEvents(NULL, QEvent::DeferredDelete);
    }

    // execute deferred deletes
    QCoreApplication::sendPostedEvents(NULL, QEvent::DeferredDelete);
}

void SlaveBase::setMetaData(const QString &key, const QString &value)
{
    // replaces existing key if already there
    d->m_outgoingMetaData.insert(key, value);
}

QString SlaveBase::metaData(const QString &key) const
{
    if (d->m_incomingMetaData.contains(key)) {
        return d->m_incomingMetaData[key];
    }
    if (d->configData.contains(key)) {
        return d->configData[key];
    }
    return QString();
}

MetaData SlaveBase::allMetaData() const
{
    return d->m_incomingMetaData;
}

bool SlaveBase::hasMetaData(const QString &key) const
{
    if (d->m_incomingMetaData.contains(key)) {
        return true;
    }
    if (d->configData.contains(key)) {
        return true;
    }
    return false;
}

KConfigGroup *SlaveBase::config()
{
    return d->configGroup;
}

void SlaveBase::sendMetaData()
{
    sendAndKeepMetaData();
    d->m_outgoingMetaData.clear();
}

void SlaveBase::sendAndKeepMetaData()
{
    if (!d->m_outgoingMetaData.isEmpty()) {
        KIO_DATA << d->m_outgoingMetaData;

        send(INF_META_DATA, data);
    }
}

KRemoteEncoding *SlaveBase::remoteEncoding()
{
    if (d->remotefile) {
        return d->remotefile;
    }
    const QByteArray charset (metaData(QLatin1String("Charset")).toLatin1());
    return (d->remotefile = new KRemoteEncoding(charset));
}

void SlaveBase::data(const QByteArray &data)
{
    sendMetaData();
    send(MSG_DATA, data);
}

void SlaveBase::dataReq()
{
    // sendMetaData();
    if (d->needSendCanResume) {
        canResume(0);
    }
    send(MSG_DATA_REQ);
}

void SlaveBase::error(int _errid, const QString &_text)
{
    if (d->m_state == SlaveBasePrivate::ErrorCalled) {
        kWarning(7019) << "error() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == SlaveBasePrivate::FinishedCalled) {
        kWarning(7019) << "error() called after finished()! Please fix the KIO slave.";
        return;
    }

    d->m_state = SlaveBasePrivate::ErrorCalled;
    d->m_incomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    d->m_outgoingMetaData.clear();
    KIO_DATA << (qint32)_errid << _text;

    send(MSG_ERROR, data);
    // reset
    d->totalSize = 0;
}

void SlaveBase::finished()
{
    if (d->m_state == SlaveBasePrivate::FinishedCalled) {
        kWarning(7019) << "finished() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == SlaveBasePrivate::ErrorCalled) {
        kWarning(7019) << "finished() called after error()! Please fix the KIO slave.";
        return;
    }

    d->m_state = SlaveBasePrivate::FinishedCalled;
    d->m_incomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    sendMetaData();
    send(MSG_FINISHED);

    // reset
    d->totalSize = 0;
}

void SlaveBase::canResume()
{
    send(MSG_CANRESUME);
}

void SlaveBase::totalSize(KIO::filesize_t _bytes)
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    send(INF_TOTAL_SIZE, data);

    //this one is usually called before the first item is listed in listDir()
    d->totalSize = _bytes;
}

void SlaveBase::processedSize(KIO::filesize_t _bytes)
{
    bool emitSignal = false;
    struct timeval tv;
    int gettimeofday_res = gettimeofday(&tv, 0L);

    if (_bytes == d->totalSize) {
        emitSignal = true;
    } else if (gettimeofday_res == 0) {
        time_t msecdiff = 2000;
        if (d->last_tv.tv_sec) {
            // Compute difference, in ms
            msecdiff = 1000 * (tv.tv_sec - d->last_tv.tv_sec);
            time_t usecdiff = tv.tv_usec - d->last_tv.tv_usec;
            if (usecdiff < 0) {
                msecdiff--;
                msecdiff += 1000;
            }
            msecdiff += usecdiff / 1000;
        }
        emitSignal = msecdiff >= 100; // emit size 10 times a second
    }

    if (emitSignal) {
        KIO_DATA << KIO_FILESIZE_T(_bytes);
        send(INF_PROCESSED_SIZE, data);
        if (gettimeofday_res == 0) {
            d->last_tv.tv_sec = tv.tv_sec;
            d->last_tv.tv_usec = tv.tv_usec;
        }
    }
}

void SlaveBase::speed(unsigned long _bytes_per_second)
{
    KIO_DATA << (quint32) _bytes_per_second;
    send(INF_SPEED, data);
}

void SlaveBase::redirection(const KUrl &_url)
{
    KIO_DATA << _url;
    send(INF_REDIRECTION, data);
}

static bool isSubCommand(int cmd)
{
   return ((cmd == CMD_REPARSECONFIGURATION) ||
           (cmd == CMD_META_DATA) ||
           (cmd == CMD_CONFIG));
}

void SlaveBase::mimeType(const QString &_type)
{
    kDebug(7019) << _type;
    int cmd = 0;
    do {
        // Send the meta-data each time we send the mime-type.
        if (!d->m_outgoingMetaData.isEmpty()) {
            // kDebug(7019) << "emitting meta data";
            KIO_DATA << d->m_outgoingMetaData;
            send(INF_META_DATA, data);
        }
        KIO_DATA << _type;
        send(INF_MIME_TYPE, data);
        while (true) {
            cmd = 0;
            int ret = -1;
            if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
                ret = d->appConnection.read(&cmd, data);
            }
            if (ret == -1) {
                kDebug(7019) << "read error";
                exit();
                return;
            }
            // kDebug(7019) << "got" << cmd;
            if (cmd == CMD_HOST) {
                // Ignore.
                continue;
            }
            if (!isSubCommand(cmd)) {
                break;
            }

            dispatch(cmd, data );
        }
    } while (cmd != CMD_NONE);
    d->m_outgoingMetaData.clear();
}

void SlaveBase::exit()
{
    d->exit_loop = true;
    // We do need to call exit(), otherwise a long download (get()) would
    // keep going until it ends, even though the application exited.
    ::exit(255);
}

void SlaveBase::warning(const QString &_msg)
{
    KIO_DATA << _msg;
    send(INF_WARNING, data);
}

void SlaveBase::infoMessage(const QString &_msg)
{
    KIO_DATA << _msg;
    send(INF_INFOMESSAGE, data);
}

void SlaveBase::statEntry(const UDSEntry &entry)
{
    KIO_DATA << entry;
    send(MSG_STAT_ENTRY, data);
}

void SlaveBase::listEntry(const UDSEntry &entry, bool ready)
{
    static const int maximum_updatetime = 300;

    // We start measuring the time from the point we start filling the list
    if (d->pendingListEntries.isEmpty()) {
        d->m_timeSinceLastBatch.restart();
    }

    if (!ready) {
        d->pendingListEntries.append(entry);

        // If more then maximum_updatetime time is passed, emit the current batch
        if (d->m_timeSinceLastBatch.elapsed() > maximum_updatetime) {
            ready = true;
        }
    }

    if (ready) { // may happen when we started with !ready
        listEntries(d->pendingListEntries);
        d->pendingListEntries.clear();

        // Restart time
        d->m_timeSinceLastBatch.restart();
    }
}

void SlaveBase::listEntries(const UDSEntryList &list)
{
    KIO_DATA << (quint32)list.count();
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it) {
        stream << *it;
    }
    send(MSG_LIST_ENTRIES, data);
}

static void sigpipe_handler (int)
{
    // We ignore a SIGPIPE in slaves.
    // A SIGPIPE can happen in two cases:
    // 1) Communication error with application.
    // 2) Communication error with network.
    slaveWriteError = true;

    // Don't add anything else here, especially no debug output
}

void SlaveBase::setHost(const QString&, quint16, const QString&, QString const &)
{
}

void SlaveBase::stat(KUrl const &)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_STAT)); }
void SlaveBase::put(KUrl const &, int, JobFlags )
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_PUT)); }
void SlaveBase::special(const QByteArray &)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_SPECIAL)); }
void SlaveBase::listDir(KUrl const &)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_LISTDIR)); }
void SlaveBase::get(KUrl const & )
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_GET)); }
void SlaveBase::mimetype(KUrl const &url)
{ get(url); }
void SlaveBase::rename(KUrl const &, KUrl const &, JobFlags)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_RENAME)); }
void SlaveBase::symlink(QString const &, KUrl const &, JobFlags)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_SYMLINK)); }
void SlaveBase::copy(KUrl const &, KUrl const &, int, JobFlags)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_COPY)); }
void SlaveBase::del(KUrl const &, bool)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_DEL)); }
void SlaveBase::mkdir(KUrl const &, int)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_MKDIR)); }
void SlaveBase::chmod(KUrl const &, int)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_CHMOD)); }
void SlaveBase::setModificationTime(KUrl const &, const QDateTime&)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_SETMODIFICATIONTIME)); }
void SlaveBase::chown(KUrl const &, const QString &, const QString &)
{ error(ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(d->m_protocol, CMD_CHOWN)); }

void SlaveBase::reparseConfiguration()
{
    delete d->remotefile;
    d->remotefile = nullptr;
}

bool SlaveBase::openPasswordDialog(AuthInfo& info, const QString &errorMsg)
{
    if (metaData(QLatin1String("no-auth-prompt")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    const qlonglong windowId = metaData(QLatin1String("window-id")).toLongLong();
    QWidget *windowWidget = QWidget::find(windowId);

    AuthInfo dlgInfo(info);

    KPasswdStore* passwdstore = d->passwdStore();
    Q_ASSERT(passwdstore);

    // assemble dialog-flags
    KPasswordDialog::KPasswordDialogFlags dialogFlags;

    if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).isValid()) {
        dialogFlags |= KPasswordDialog::ShowDomainLine;
    }

    if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).isValid()) {
        dialogFlags |= KPasswordDialog::ShowAnonymousLoginCheckBox;
    }

    if (!dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_HIDE_USERNAME_INPUT).toBool()) {
        dialogFlags |= KPasswordDialog::ShowUsernameLine;
    }

    // If store is not enabled and the caller explicitly requested for it,
    // do not show the keep password checkbox.
    if (dlgInfo.keepPassword && !passwdstore->cacheOnly()) {
        dialogFlags |= KPasswordDialog::ShowKeepPassword;
    }

    KPasswordDialog* dlg = new KPasswordDialog(windowWidget, dialogFlags);

    QString username = dlgInfo.username;
    QString password = dlgInfo.password;

    dlg->setPrompt(dlgInfo.prompt);
    dlg->setUsername(username);
    if (dlgInfo.caption.isEmpty()) {
        dlg->setWindowTitle(i18n("Authentication Dialog"));
    } else {
        dlg->setWindowTitle(dlgInfo.caption);
    }

    if (!dlgInfo.comment.isEmpty()) {
        dlg->addCommentLine(dlgInfo.commentLabel, dlgInfo.comment);
    }

    if (!password.isEmpty()) {
        dlg->setPassword(password);
    }

    if (dlgInfo.readOnly) {
        dlg->setUsernameReadOnly(true);
    }

    if (!passwdstore->cacheOnly()) {
        dlg->setKeepPassword(true);
    }

    if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).isValid()) {
        dlg->setDomain(dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).toString());
    }

    if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).isValid () && password.isEmpty() && username.isEmpty()) {
        dlg->setAnonymousMode(dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).toBool());
    }

    KWindowSystem::setMainWindow(dlg, windowId);

    if (dlg->exec() == KPasswordDialog::Accepted) {
        dlgInfo.username = dlg->username();
        dlgInfo.password = dlg->password();
        dlgInfo.keepPassword = dlg->keepPassword();

        if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).isValid()) {
            dlgInfo.setExtraField(AUTHINFO_EXTRAFIELD_DOMAIN, dlg->domain());
        }
        if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).isValid()) {
            dlgInfo.setExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS, dlg->anonymousMode());
        }

        info = dlgInfo;
        return true;
    }

    return false;
}

int SlaveBase::messageBox(MessageBoxType type, const QString &text, const QString &caption,
                          const QString &buttonYes, const QString &buttonNo)
{
    return messageBox(text, type, caption, buttonYes, buttonNo, QString());
}

int SlaveBase::messageBox(const QString &text, MessageBoxType type, const QString &caption,
                          const QString &buttonYes, const QString &buttonNo,
                          const QString &dontAskAgainName)
{
    kDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo;
    KIO_DATA << (qint32)type << text << caption << buttonYes << buttonNo << dontAskAgainName;
    send(INF_MESSAGEBOX, data);
    if (waitForAnswer(CMD_MESSAGEBOXANSWER, 0, data) != -1) {
        QDataStream stream(data);
        int answer;
        stream >> answer;
        kDebug(7019) << "got messagebox answer" << answer;
        return answer;
    }
    // communication failure
    return 0;
}

bool SlaveBase::canResume(KIO::filesize_t offset)
{
    kDebug(7019) << "offset=" << KIO::number(offset);
    d->needSendCanResume = false;
    KIO_DATA << KIO_FILESIZE_T(offset);
    send(MSG_RESUME, data);
    if (offset) {
        int cmd = 0;
        if (waitForAnswer(CMD_RESUMEANSWER, CMD_NONE, data, &cmd) != -1) {
            kDebug(7019) << "returning" << (cmd == CMD_RESUMEANSWER);
            return cmd == CMD_RESUMEANSWER;
        }
        return false;
    }
    // No resuming possible -> no answer to wait for
    return true;
}

int SlaveBase::waitForAnswer(int expected1, int expected2, QByteArray &data, int *pCmd)
{
    int cmd = 0;
    int result = -1;
    for (;;) {
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
            result = d->appConnection.read(&cmd, data);
        }
        if (result == -1) {
            kDebug(7019) << "read error.";
            return result;
        }

        if (cmd == expected1 || cmd == expected2) {
            if (pCmd) {
                *pCmd = cmd;
            }
            return result;
        }
        if (isSubCommand(cmd)) {
            dispatch(cmd, data);
        } else {
            kFatal(7019) << "Got cmd " << cmd << " while waiting for an answer!";
        }
    }
    return result;
}

int SlaveBase::readData(QByteArray &buffer)
{
    int result = waitForAnswer(MSG_DATA, 0, buffer);
    // kDebug(7019) << "readData: length = " << result << " ";
    return result;
}

void SlaveBase::setTimeoutSpecialCommand(int timeout, const QByteArray &data)
{
    if (timeout > 0) {
        d->timeout = time(0)+(time_t)timeout;
    } else if (timeout == 0) {
        d->timeout = 1; // Immediate timeout
    } else {
        d->timeout = 0; // Canceled
    }

   d->timeoutData = data;
}

void SlaveBase::dispatch(int command, const QByteArray &data)
{
    QDataStream stream(data);

    switch(command) {
        case CMD_HOST: {
            QString passwd;
            QString host, user;
            quint16 port;
            stream >> host >> port >> user >> passwd;
            d->m_state = SlaveBasePrivate::InsideMethod;
            setHost(host, port, user, passwd);
            d->verifyErrorFinishedNotCalled("setHost()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_REPARSECONFIGURATION: {
            d->m_state = SlaveBasePrivate::InsideMethod;
            reparseConfiguration();
            d->verifyErrorFinishedNotCalled("reparseConfiguration()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_CONFIG: {
            stream >> d->configData;
            d->rebuildConfig();
            delete d->remotefile;
            d->remotefile = nullptr;
            break;
        }
        case CMD_GET: {
            KUrl url;
            stream >> url;
            d->m_state = SlaveBasePrivate::InsideMethod;
            get(url);
            d->verifyState("get()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_PUT: {
            KUrl url;
            int permissions;
            qint8 iOverwrite, iResume;
            stream >> url >> iOverwrite >> iResume >> permissions;
            JobFlags flags = DefaultFlags;
            if (iOverwrite != 0) {
                flags |= Overwrite;
            }
            if (iResume != 0) {
                flags |= Resume;
            }

            // Remember that we need to send canResume(), TransferJob is expecting
            // it. Well, in theory this shouldn't be done
            d->needSendCanResume = true;

            d->m_state = SlaveBasePrivate::InsideMethod;
            put(url, permissions, flags);
            d->verifyState("put()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_STAT: {
            KUrl url;
            stream >> url;
            d->m_state = SlaveBasePrivate::InsideMethod;
            stat(url);
            d->verifyState("stat()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_MIMETYPE: {
            KUrl url;
            stream >> url;
            d->m_state = SlaveBasePrivate::InsideMethod;
            mimetype(url);
            d->verifyState("mimetype()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_LISTDIR: {
            KUrl url;
            stream >> url;
            d->m_state = SlaveBasePrivate::InsideMethod;
            listDir(url);
            d->verifyState("listDir()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_MKDIR: {
            KUrl url;
            int i;
            stream >> url >> i;
            d->m_state = SlaveBasePrivate::InsideMethod;
            mkdir(url, i);
            d->verifyState("mkdir()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_RENAME: {
            KUrl url;
            KUrl url2;
            qint8 iOverwrite;
            stream >> url >> url2 >> iOverwrite;
            JobFlags flags = DefaultFlags;
            if (iOverwrite != 0) {
                flags |= Overwrite;
            }
            d->m_state = SlaveBasePrivate::InsideMethod;
            rename(url, url2, flags);
            d->verifyState("rename()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_SYMLINK: {
            KUrl url;
            QString target;
            qint8 iOverwrite;
            stream >> target >> url >> iOverwrite;
            JobFlags flags = DefaultFlags;
            if (iOverwrite != 0) {
                flags |= Overwrite;
            }
            d->m_state = SlaveBasePrivate::InsideMethod;
            symlink(target, url, flags);
            d->verifyState("symlink()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_COPY: {
            KUrl url;
            KUrl url2;
            int permissions;
            qint8 iOverwrite;
            stream >> url >> url2 >> permissions >> iOverwrite;
            JobFlags flags = DefaultFlags;
            if (iOverwrite != 0) {
                flags |= Overwrite;
            }
            d->m_state = SlaveBasePrivate::InsideMethod;
            copy(url, url2, permissions, flags);
            d->verifyState("copy()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_DEL: {
            KUrl url;
            qint8 isFile;
            stream >> url >> isFile;
            d->m_state = SlaveBasePrivate::InsideMethod;
            del(url, isFile != 0);
            d->verifyState("del()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_CHMOD: {
            KUrl url;
            int i;
            stream >> url >> i;
            d->m_state = SlaveBasePrivate::InsideMethod;
            chmod(url, i);
            d->verifyState("chmod()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_CHOWN: {
            KUrl url;
            QString owner, group;
            stream >> url >> owner >> group;
            d->m_state = SlaveBasePrivate::InsideMethod;
            chown(url, owner, group);
            d->verifyState("chown()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_SETMODIFICATIONTIME: {
            KUrl url;
            QDateTime dt;
            stream >> url >> dt;
            d->m_state = SlaveBasePrivate::InsideMethod;
            setModificationTime(url, dt);
            d->verifyState("setModificationTime()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_SPECIAL: {
            d->m_state = SlaveBasePrivate::InsideMethod;
            special(data);
            d->verifyState("special()");
            d->m_state = SlaveBasePrivate::Idle;
            break;
        }
        case CMD_META_DATA: {
            // kDebug(7019) << "(" << getpid() << ") Incoming meta-data...";
            stream >> d->m_incomingMetaData;
            d->rebuildConfig();
            break;
        }
        case CMD_NONE: {
            kWarning(7019) << "Got unexpected CMD_NONE!";
            break;
        }
        default: {
            // Some command we don't understand.
            // Just ignore it, it may come from some future version of KDE.
            break;
        }
    }
}

bool SlaveBase::checkCachedAuthentication(AuthInfo &info)
{
    KPasswdStore* passwdstore = d->passwdStore();
    Q_ASSERT(passwdstore);
    const qlonglong windowId = metaData(QLatin1String("window-id")).toLongLong();
    const QByteArray authkey = authInfoKey(info);
    if (passwdstore->hasPasswd(authkey, windowId)) {
        const QString passwd = passwdstore->getPasswd(authkey, windowId);
        info = authInfoFromData(passwd.toLatin1());
        return true;
    }
    return false;
}

bool SlaveBase::cacheAuthentication(const AuthInfo &info)
{
    KPasswdStore* passwdstore = d->passwdStore();
    Q_ASSERT(passwdstore);
    passwdstore->storePasswd(authInfoKey(info), authInfoToData(info), metaData(QLatin1String("window-id")).toLongLong());
    return true;
}

int SlaveBase::connectTimeout()
{
    bool ok = false;
    QString tmp = metaData(QLatin1String("ConnectTimeout"));
    int result = tmp.toInt(&ok);
    if (ok) {
        return result;
    }
    return DEFAULT_CONNECT_TIMEOUT;
}

int SlaveBase::proxyConnectTimeout()
{
    bool ok = false;
    QString tmp = metaData(QLatin1String("ProxyConnectTimeout"));
    int result = tmp.toInt(&ok);
    if (ok) {
        return result;
    }
    return DEFAULT_PROXY_CONNECT_TIMEOUT;
}

int SlaveBase::responseTimeout()
{
    bool ok = false;
    QString tmp = metaData(QLatin1String("ResponseTimeout"));
    int result = tmp.toInt(&ok);
    if (ok) {
        return result;
    }
    return DEFAULT_RESPONSE_TIMEOUT;
}

int SlaveBase::readTimeout()
{
    bool ok = false;
    QString tmp = metaData(QLatin1String("ReadTimeout"));
    int result = tmp.toInt(&ok);
    if (ok) {
        return result;
    }
    return DEFAULT_READ_TIMEOUT;
}

bool SlaveBase::wasKilled() const
{
   return d->wasKilled;
}

void SlaveBase::setKillFlag()
{
    d->wasKilled = true;
}

void SlaveBase::send(int cmd, const QByteArray &arr)
{
    slaveWriteError = false;
    if (!d->appConnection.send(cmd, arr)) {
        // Note that slaveWriteError can also be set by sigpipe_handler
        slaveWriteError = true;
    }
    if (slaveWriteError) {
        exit();
    }
}
