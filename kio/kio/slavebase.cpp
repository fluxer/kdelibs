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

#include <kdebug.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <QtCore/QBuffer>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QDateTime>
#include <QtCore/QCoreApplication>

#include <kcrash.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kde_file.h>
#include <klocale.h>
#include <kpassworddialog.h>
#include <kwindowsystem.h>

#include "kremoteencoding.h"

#include "connection.h"
#include "ioslave_defaults.h"
#include "slaveinterface.h"
#include "kpasswdstore.h"

#define AUTHINFO_EXTRAFIELD_DOMAIN QLatin1String("domain")
#define AUTHINFO_EXTRAFIELD_ANONYMOUS QLatin1String("anonymous")
#define AUTHINFO_EXTRAFIELD_SKIP_CACHING_ON_QUERY QLatin1String("skip-caching-on-query")
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
    SlaveBase* q;
    SlaveBasePrivate(SlaveBase* owner): q(owner), m_passwdStore(nullptr) {}
    ~SlaveBasePrivate() { delete m_passwdStore; }

    UDSEntryList pendingListEntries;
    QTime m_timeSinceLastBatch;
    Connection appConnection;

    bool resume:1;
    bool needSendCanResume:1;
    bool onHold:1;
    bool wasKilled:1;
    bool inOpenLoop:1;
    bool exit_loop:1;
    MetaData configData;
    KConfig *config;
    KConfigGroup *configGroup;
    KUrl onHoldUrl;

    struct timeval last_tv;
    KIO::filesize_t totalSize;
    KRemoteEncoding *remotefile;
    time_t timeout;
    enum { Idle, InsideMethod, FinishedCalled, ErrorCalled } m_state;
    QByteArray timeoutData;

    KPasswdStore* m_passwdStore;

    // Reconstructs configGroup from configData and mIncomingMetaData
    void rebuildConfig()
    {
        configGroup->deleteGroup(KConfigGroup::WriteConfigFlags());

        // mIncomingMetaData cascades over config, so we write config first,
        // to let it be overwritten
        MetaData::ConstIterator end = configData.constEnd();
        for (MetaData::ConstIterator it = configData.constBegin(); it != end; ++it)
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());

        end = q->mIncomingMetaData.constEnd();
        for (MetaData::ConstIterator it = q->mIncomingMetaData.constBegin(); it != end; ++it)
            configGroup->writeEntry(it.key(), it->toUtf8(), KConfigGroup::WriteConfigFlags());
    }

    void verifyState(const char* cmdName)
    {
        if ((m_state != FinishedCalled) && (m_state != ErrorCalled)){
            kWarning(7019) << cmdName << "did not call finished() or error()! Please fix the KIO slave.";
        }
    }

    void verifyErrorFinishedNotCalled(const char* cmdName)
    {
        if (m_state == FinishedCalled || m_state == ErrorCalled) {
            kWarning(7019) << cmdName << "called finished() or error(), but it's not supposed to! Please fix the KIO slave.";
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

}

static SlaveBase *globalSlave = nullptr;

static volatile bool slaveWriteError = false;

extern "C" {
static void genericsig_handler(int sigNumber)
{
   KDE_signal(sigNumber, SIG_IGN);
   //WABA: Don't do anything that requires malloc, we can deadlock on it since
   //a SIGTERM signal can come in while we are in malloc/free.
   //kDebug()<<"kioslave : exiting due to signal "<<sigNumber;
   //set the flag which will be checked in dispatchLoop() and which *should* be checked
   //in lengthy operations in the various slaves
   if (globalSlave != 0)
      globalSlave->setKillFlag();
   KDE_signal(SIGALRM, SIG_DFL);
   alarm(5);  //generate an alarm signal in 5 seconds, in this time the slave has to exit
}
}

//////////////

SlaveBase::SlaveBase( const QByteArray &protocol,
                      const QByteArray &app_socket )
    : mProtocol(protocol),
      d(new SlaveBasePrivate(this))

{
    if (qgetenv("KDE_DEBUG").isEmpty())
    {
        KCrash::setFlags(KCrash::flags() | KCrash::DrKonqi);
    }

    struct sigaction act;
    act.sa_handler = sigpipe_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE, &act, 0);

    KDE_signal(SIGINT, &genericsig_handler);
    KDE_signal(SIGQUIT, &genericsig_handler);
    KDE_signal(SIGTERM, &genericsig_handler);

    globalSlave = this;

    d->resume = false;
    d->needSendCanResume = false;
    d->config = new KConfig(QString(), KConfig::SimpleConfig);
    // The KConfigGroup needs the KConfig to exist during its whole lifetime.
    d->configGroup = new KConfigGroup(d->config, QString());
    d->onHold = false;
    d->wasKilled=false;
    d->last_tv.tv_sec = 0;
    d->last_tv.tv_usec = 0;
    d->totalSize=0;
    d->timeout = 0;
    connectSlave(QFile::decodeName(app_socket));

    d->remotefile = 0;
    d->inOpenLoop = false;
    d->exit_loop = false;
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
        if (d->timeout)
            ms = 1000 * qMax<time_t>(d->timeout - time(0), 1);

        int ret = -1;
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(ms)) {
            // dispatch application messages
            int cmd;
            QByteArray data;
            ret = d->appConnection.read(&cmd, data);

            if (ret != -1) {
                if (d->inOpenLoop)
                    dispatchOpenCommand(cmd, data);
                else
                    dispatch(cmd, data);
            }
        } else {
            ret = d->appConnection.isConnected() ? 0 : -1;
        }

        if (ret == -1) { // some error occurred, perhaps no more application
            disconnectSlave();
            closeConnection();
            break;
        }

        //I think we get here when we were killed in dispatch() and not in select()
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

void SlaveBase::connectSlave(const QString &address)
{
    d->appConnection.connectToRemote(address);

    if (!d->appConnection.inited())
    {
        kDebug(7019) << "failed to connect to" << address << '\n'
                     << "Reason:" << d->appConnection.errorString();
        exit();
        return;
    }

    d->inOpenLoop = false;
}

void SlaveBase::disconnectSlave()
{
    d->appConnection.close();
}

void SlaveBase::setMetaData(const QString &key, const QString &value)
{
    mOutgoingMetaData.insert(key, value); // replaces existing key if already there
}

QString SlaveBase::metaData(const QString &key) const
{
   if (mIncomingMetaData.contains(key))
      return mIncomingMetaData[key];
   if (d->configData.contains(key))
      return d->configData[key];
   return QString();
}

MetaData SlaveBase::allMetaData() const
{
    return mIncomingMetaData;
}

bool SlaveBase::hasMetaData(const QString &key) const
{
   if (mIncomingMetaData.contains(key))
      return true;
   if (d->configData.contains(key))
      return true;
   return false;
}

KConfigGroup *SlaveBase::config()
{
   return d->configGroup;
}

void SlaveBase::sendMetaData()
{
    sendAndKeepMetaData();
    mOutgoingMetaData.clear();
}

void SlaveBase::sendAndKeepMetaData()
{
    if (!mOutgoingMetaData.isEmpty()) {
        KIO_DATA << mOutgoingMetaData;

        send(INF_META_DATA, data);
    }
}

KRemoteEncoding *SlaveBase::remoteEncoding()
{
   if (d->remotefile)
      return d->remotefile;

   const QByteArray charset (metaData(QLatin1String("Charset")).toLatin1());
   return (d->remotefile = new KRemoteEncoding( charset ));
}

void SlaveBase::data( const QByteArray &data )
{
   sendMetaData();
   send( MSG_DATA, data );
}

void SlaveBase::dataReq( )
{
   //sendMetaData();
   if (d->needSendCanResume)
      canResume(0);
   send( MSG_DATA_REQ );
}

void SlaveBase::opened()
{
   sendMetaData();
   send( MSG_OPENED );
   d->inOpenLoop = true;
}

void SlaveBase::error( int _errid, const QString &_text )
{
    if (d->m_state == d->ErrorCalled) {
        kWarning(7019) << "error() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == d->FinishedCalled) {
        kWarning(7019) << "error() called after finished()! Please fix the KIO slave.";
        return;
    }

    d->m_state = d->ErrorCalled;
    mIncomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    mOutgoingMetaData.clear();
    KIO_DATA << (qint32) _errid << _text;

    send( MSG_ERROR, data );
    //reset
    d->totalSize=0;
    d->inOpenLoop=false;
}

void SlaveBase::connected()
{
    send( MSG_CONNECTED );
}

void SlaveBase::finished()
{
    if (d->m_state == d->FinishedCalled) {
        kWarning(7019) << "finished() called twice! Please fix the KIO slave.";
        return;
    } else if (d->m_state == d->ErrorCalled) {
        kWarning(7019) << "finished() called after error()! Please fix the KIO slave.";
        return;
    }

    d->m_state = d->FinishedCalled;
    mIncomingMetaData.clear(); // Clear meta data
    d->rebuildConfig();
    sendMetaData();
    send( MSG_FINISHED );

    // reset
    d->totalSize=0;
    d->inOpenLoop=false;
}

void SlaveBase::needSubUrlData()
{
    send( MSG_NEED_SUBURL_DATA );
}

void SlaveBase::slaveStatus( const QString &host, bool connected )
{
    qint64 pid = static_cast<qint64>(::getpid());
    qint8 b = connected ? 1 : 0;
    KIO_DATA << pid << mProtocol << host << b;
    if (d->onHold)
       stream << d->onHoldUrl;
    send( MSG_SLAVE_STATUS, data );
}

void SlaveBase::canResume()
{
    send( MSG_CANRESUME );
}

void SlaveBase::totalSize( KIO::filesize_t _bytes )
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    send( INF_TOTAL_SIZE, data );

    //this one is usually called before the first item is listed in listDir()
    d->totalSize=_bytes;
}

void SlaveBase::processedSize( KIO::filesize_t _bytes )
{
    bool           emitSignal=false;
    struct timeval tv;
    int            gettimeofday_res=gettimeofday( &tv, 0L );

    if( _bytes == d->totalSize )
        emitSignal=true;
    else if ( gettimeofday_res == 0 ) {
        time_t msecdiff = 2000;
        if (d->last_tv.tv_sec) {
            // Compute difference, in ms
            msecdiff = 1000 * ( tv.tv_sec - d->last_tv.tv_sec );
            time_t usecdiff = tv.tv_usec - d->last_tv.tv_usec;
            if ( usecdiff < 0 ) {
                msecdiff--;
                msecdiff += 1000;
            }
            msecdiff += usecdiff / 1000;
        }
        emitSignal=msecdiff >= 100; // emit size 10 times a second
    }

    if( emitSignal ) {
        KIO_DATA << KIO_FILESIZE_T(_bytes);
        send( INF_PROCESSED_SIZE, data );
        if ( gettimeofday_res == 0 ) {
            d->last_tv.tv_sec = tv.tv_sec;
            d->last_tv.tv_usec = tv.tv_usec;
        }
    }
}

void SlaveBase::written( KIO::filesize_t _bytes )
{
    KIO_DATA << KIO_FILESIZE_T(_bytes);
    send( MSG_WRITTEN, data );
}

void SlaveBase::position( KIO::filesize_t _pos )
{
    KIO_DATA << KIO_FILESIZE_T(_pos);
    send( INF_POSITION, data );
}

void SlaveBase::speed( unsigned long _bytes_per_second )
{
    KIO_DATA << (quint32) _bytes_per_second;
    send( INF_SPEED, data );
}

void SlaveBase::redirection( const KUrl& _url )
{
    KIO_DATA << _url;
    send( INF_REDIRECTION, data );
}

static bool isSubCommand(int cmd)
{
   return ( (cmd == CMD_REPARSECONFIGURATION) ||
            (cmd == CMD_META_DATA) ||
            (cmd == CMD_CONFIG) ||
            (cmd == CMD_SUBURL) ||
            (cmd == CMD_SLAVE_STATUS) ||
            (cmd == CMD_SLAVE_CONNECT));
}

void SlaveBase::mimeType( const QString &_type)
{
  kDebug(7019) << _type;
  int cmd;
  do
  {
    // Send the meta-data each time we send the mime-type.
    if (!mOutgoingMetaData.isEmpty())
    {
      // kDebug(7019) << "emitting meta data";
      KIO_DATA << mOutgoingMetaData;
      send( INF_META_DATA, data );
    }
    KIO_DATA << _type;
    send( INF_MIME_TYPE, data );
    while(true)
    {
       cmd = 0;
       int ret = -1;
       if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
           ret = d->appConnection.read( &cmd, data );
       }
       if (ret == -1) {
           kDebug(7019) << "read error";
           exit();
           return;
       }
       // kDebug(7019) << "got" << cmd;
       if ( cmd == CMD_HOST) // Ignore.
          continue;
       if (!isSubCommand(cmd))
          break;

       dispatch( cmd, data );
    }
  }
  while (cmd != CMD_NONE);
  mOutgoingMetaData.clear();
}

void SlaveBase::exit()
{
    d->exit_loop = true;
    // Using ::exit() here is too much (crashes in qdbus's qglobalstatic object),
    // so let's cleanly exit dispatchLoop() instead.
    // Update: we do need to call exit(), otherwise a long download (get()) would
    // keep going until it ends, even though the application exited.
    ::exit(255);
}

void SlaveBase::warning( const QString &_msg)
{
    KIO_DATA << _msg;
    send( INF_WARNING, data );
}

void SlaveBase::infoMessage( const QString &_msg)
{
    KIO_DATA << _msg;
    send( INF_INFOMESSAGE, data );
}

void SlaveBase::statEntry( const UDSEntry& entry )
{
    KIO_DATA << entry;
    send( MSG_STAT_ENTRY, data );
}

void SlaveBase::listEntry( const UDSEntry& entry, bool _ready )
{
    static const int maximum_updatetime = 300;

    // We start measuring the time from the point we start filling the list
    if (d->pendingListEntries.isEmpty()) {
        d->m_timeSinceLastBatch.restart();
    }

    if (!_ready) {
        d->pendingListEntries.append(entry);

        // If more then maximum_updatetime time is passed, emit the current batch
        if (d->m_timeSinceLastBatch.elapsed() > maximum_updatetime) {
            _ready = true;
        }
    }

    if (_ready) { // may happen when we started with !ready
        listEntries( d->pendingListEntries );
        d->pendingListEntries.clear();

        // Restart time
        d->m_timeSinceLastBatch.restart();
    }
}

void SlaveBase::listEntries( const UDSEntryList& list )
{
    KIO_DATA << (quint32)list.count();
    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
      stream << *it;
    send( MSG_LIST_ENTRIES, data);
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

void SlaveBase::setHost(QString const &, quint16, QString const &, QString const &)
{
}

void SlaveBase::openConnection(void)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CONNECT)); }
void SlaveBase::closeConnection(void)
{ } // No response!
void SlaveBase::stat(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_STAT)); }
void SlaveBase::put(KUrl const &, int, JobFlags )
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_PUT)); }
void SlaveBase::special(const QByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SPECIAL)); }
void SlaveBase::listDir(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_LISTDIR)); }
void SlaveBase::get(KUrl const & )
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_GET)); }
void SlaveBase::open(KUrl const &, QIODevice::OpenMode)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_OPEN)); }
void SlaveBase::read(KIO::filesize_t)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_READ)); }
void SlaveBase::write(const QByteArray &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_WRITE)); }
void SlaveBase::seek(KIO::filesize_t)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SEEK)); }
void SlaveBase::close()
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CLOSE)); }
void SlaveBase::mimetype(KUrl const &url)
{ get(url); }
void SlaveBase::rename(KUrl const &, KUrl const &, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_RENAME)); }
void SlaveBase::symlink(QString const &, KUrl const &, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SYMLINK)); }
void SlaveBase::copy(KUrl const &, KUrl const &, int, JobFlags)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_COPY)); }
void SlaveBase::del(KUrl const &, bool)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_DEL)); }
void SlaveBase::setLinkDest(const KUrl &, const QString&)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SETLINKDEST)); }
void SlaveBase::mkdir(KUrl const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_MKDIR)); }
void SlaveBase::chmod(KUrl const &, int)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CHMOD)); }
void SlaveBase::setModificationTime(KUrl const &, const QDateTime&)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SETMODIFICATIONTIME)); }
void SlaveBase::chown(KUrl const &, const QString &, const QString &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_CHOWN)); }
void SlaveBase::setSubUrl(KUrl const &)
{ error(  ERR_UNSUPPORTED_ACTION, unsupportedActionErrorString(mProtocol, CMD_SUBURL)); }


void SlaveBase::slave_status()
{ slaveStatus( QString(), false ); }

void SlaveBase::reparseConfiguration()
{
    delete d->remotefile;
    d->remotefile = 0;
}

bool SlaveBase::openPasswordDialog( AuthInfo& info, const QString &errorMsg )
{
    if (metaData(QLatin1String("no-auth-prompt")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0) {
        return false;
    }

    const qlonglong windowId = metaData(QLatin1String("window-id")).toLongLong();
    QWidget *windowWidget = QWidget::find(windowId);

    AuthInfo dlgInfo(info);
    // Prevent queryAuthInfo from caching the user supplied password since
    // we need the ioslaves to first authenticate against the server with
    // it to ensure it is valid.
    dlgInfo.setExtraField(AUTHINFO_EXTRAFIELD_SKIP_CACHING_ON_QUERY, true);

    KPasswdStore* passwdstore = d->passwdStore();

    if (passwdstore) {
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
        if (dlgInfo.keepPassword && !passwdstore->cacheOnly())
            dialogFlags |= KPasswordDialog::ShowKeepPassword;

        KPasswordDialog* dlg = new KPasswordDialog(windowWidget, dialogFlags);

        QString username = dlgInfo.username;
        QString password = dlgInfo.password;

        dlg->setPrompt(dlgInfo.prompt);
        dlg->setUsername(username);
        if (dlgInfo.caption.isEmpty())
            dlg->setWindowTitle(i18n("Authentication Dialog"));
        else
            dlg->setWindowTitle(dlgInfo.caption);

        if (!dlgInfo.comment.isEmpty() )
            dlg->addCommentLine(dlgInfo.commentLabel, dlgInfo.comment);

        if (!password.isEmpty())
            dlg->setPassword(password);

        if (dlgInfo.readOnly)
            dlg->setUsernameReadOnly(true);

        if (!passwdstore->cacheOnly())
            dlg->setKeepPassword(true);

        if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).isValid ())
            dlg->setDomain(dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).toString());

        if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).isValid () && password.isEmpty() && username.isEmpty())
            dlg->setAnonymousMode(dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).toBool());

        KWindowSystem::setMainWindow(dlg, windowId);

        if (dlg->exec() == KPasswordDialog::Accepted) {
            dlgInfo.username = dlg->username();
            dlgInfo.password = dlg->password();
            dlgInfo.keepPassword = dlg->keepPassword();

            if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_DOMAIN).isValid())
                dlgInfo.setExtraField(AUTHINFO_EXTRAFIELD_DOMAIN, dlg->domain());
            if (dlgInfo.getExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS).isValid())
                dlgInfo.setExtraField(AUTHINFO_EXTRAFIELD_ANONYMOUS, dlg->anonymousMode());

            info = dlgInfo;
            return true;
        }
    }

    return false;
}

int SlaveBase::messageBox( MessageBoxType type, const QString &text, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo )
{
    return messageBox( text, type, caption, buttonYes, buttonNo, QString() );
}

int SlaveBase::messageBox( const QString &text, MessageBoxType type, const QString &caption,
                           const QString &buttonYes, const QString &buttonNo,
                           const QString &dontAskAgainName )
{
    kDebug(7019) << "messageBox " << type << " " << text << " - " << caption << buttonYes << buttonNo;
    KIO_DATA << (qint32)type << text << caption << buttonYes << buttonNo << dontAskAgainName;
    send( INF_MESSAGEBOX, data );
    if ( waitForAnswer( CMD_MESSAGEBOXANSWER, 0, data ) != -1 )
    {
        QDataStream stream( data );
        int answer;
        stream >> answer;
        kDebug(7019) << "got messagebox answer" << answer;
        return answer;
    } else
        return 0; // communication failure
}

bool SlaveBase::canResume( KIO::filesize_t offset )
{
    kDebug(7019) << "offset=" << KIO::number(offset);
    d->needSendCanResume = false;
    KIO_DATA << KIO_FILESIZE_T(offset);
    send( MSG_RESUME, data );
    if ( offset )
    {
        int cmd;
        if ( waitForAnswer( CMD_RESUMEANSWER, CMD_NONE, data, &cmd ) != -1 )
        {
            kDebug(7019) << "returning" << (cmd == CMD_RESUMEANSWER);
            return cmd == CMD_RESUMEANSWER;
        } else
            return false;
    }
    else // No resuming possible -> no answer to wait for
        return true;
}



int SlaveBase::waitForAnswer( int expected1, int expected2, QByteArray & data, int *pCmd )
{
    int cmd, result = -1;
    for (;;)
    {
        if (d->appConnection.hasTaskAvailable() || d->appConnection.waitForIncomingTask(-1)) {
            result = d->appConnection.read( &cmd, data );
        }
        if (result == -1) {
            kDebug(7019) << "read error.";
            return -1;
        }

        if ( cmd == expected1 || cmd == expected2 )
        {
            if ( pCmd ) *pCmd = cmd;
            return result;
        }
        if ( isSubCommand(cmd) )
        {
            dispatch( cmd, data );
        }
        else
        {
            kFatal(7019) << "Got cmd " << cmd << " while waiting for an answer!";
        }
    }
}


int SlaveBase::readData( QByteArray &buffer)
{
   int result = waitForAnswer( MSG_DATA, 0, buffer );
   //kDebug(7019) << "readData: length = " << result << " ";
   return result;
}

void SlaveBase::setTimeoutSpecialCommand(int timeout, const QByteArray &data)
{
   if (timeout > 0)
      d->timeout = time(0)+(time_t)timeout;
   else if (timeout == 0)
      d->timeout = 1; // Immediate timeout
   else
      d->timeout = 0; // Canceled

   d->timeoutData = data;
}

void SlaveBase::dispatch( int command, const QByteArray &data )
{
    QDataStream stream( data );

    switch( command ) {
        case CMD_HOST: {
            QString passwd;
            QString host, user;
            quint16 port;
            stream >> host >> port >> user >> passwd;
            d->m_state = d->InsideMethod;
            setHost( host, port, user, passwd );
            d->verifyErrorFinishedNotCalled("setHost()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_CONNECT: {
            openConnection( );
            break;
        }
        case CMD_DISCONNECT: {
            closeConnection();
            break;
        }
        case CMD_SLAVE_STATUS: {
            d->m_state = d->InsideMethod;
            slave_status();
            // TODO verify that the slave has called slaveStatus()?
            d->verifyErrorFinishedNotCalled("slave_status()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_SLAVE_CONNECT: {
            d->onHold = false;
            QString app_socket;
            QDataStream stream( data );
            stream >> app_socket;
            d->appConnection.send( MSG_SLAVE_ACK );
            disconnectSlave();
            connectSlave(app_socket);
            break;
        }
        case CMD_REPARSECONFIGURATION: {
            d->m_state = d->InsideMethod;
            reparseConfiguration();
            d->verifyErrorFinishedNotCalled("reparseConfiguration()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_CONFIG: {
            stream >> d->configData;
            d->rebuildConfig();
            delete d->remotefile;
            d->remotefile = 0;
            break;
        }
        case CMD_GET: {
            KUrl url;
            stream >> url;
            d->m_state = d->InsideMethod;
            get( url );
            d->verifyState("get()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_OPEN: {
            KUrl url;
            int i;
            stream >> url >> i;
            QIODevice::OpenMode mode = QFlag(i);
            d->m_state = d->InsideMethod;
            open(url, mode); //krazy:exclude=syscalls
            d->m_state = d->Idle;
            break;
        }
        case CMD_PUT: {
            KUrl url;
            int permissions;
            qint8 iOverwrite, iResume;
            stream >> url >> iOverwrite >> iResume >> permissions;
            JobFlags flags;
            if ( iOverwrite != 0 ) flags |= Overwrite;
            if ( iResume != 0 ) flags |= Resume;

            // Remember that we need to send canResume(), TransferJob is expecting
            // it. Well, in theory this shouldn't be done if resume is true.
            //   (the resume bool is currently unused)
            d->needSendCanResume = true   /* !resume */;

            d->m_state = d->InsideMethod;
            put( url, permissions, flags);
            d->verifyState("put()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_STAT: {
            KUrl url;
            stream >> url;
            d->m_state = d->InsideMethod;
            stat( url ); //krazy:exclude=syscalls
            d->verifyState("stat()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_MIMETYPE: {
            KUrl url;
            stream >> url;
            d->m_state = d->InsideMethod;
            mimetype( url );
            d->verifyState("mimetype()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_LISTDIR: {
            KUrl url;
            stream >> url;
            d->m_state = d->InsideMethod;
            listDir( url );
            d->verifyState("listDir()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_MKDIR: {
            KUrl url;
            int i;
            stream >> url >> i;
            d->m_state = d->InsideMethod;
            mkdir( url, i ); //krazy:exclude=syscalls
            d->verifyState("mkdir()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_RENAME: {
            KUrl url;
            KUrl url2;
            qint8 iOverwrite;
            stream >> url >> url2 >> iOverwrite;
            JobFlags flags;
            if ( iOverwrite != 0 ) flags |= Overwrite;
            d->m_state = d->InsideMethod;
            rename( url, url2, flags ); //krazy:exclude=syscalls
            d->verifyState("rename()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_SYMLINK: {
            KUrl url;
            QString target;
            qint8 iOverwrite;
            stream >> target >> url >> iOverwrite;
            JobFlags flags;
            if ( iOverwrite != 0 ) flags |= Overwrite;
            d->m_state = d->InsideMethod;
            symlink( target, url, flags );
            d->verifyState("symlink()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_COPY: {
            KUrl url;
            KUrl url2;
            int permissions;
            qint8 iOverwrite;
            stream >> url >> url2 >> permissions >> iOverwrite;
            JobFlags flags;
            if ( iOverwrite != 0 ) flags |= Overwrite;
            d->m_state = d->InsideMethod;
            copy( url, url2, permissions, flags );
            d->verifyState("copy()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_DEL: {
            KUrl url;
            qint8 isFile;
            stream >> url >> isFile;
            d->m_state = d->InsideMethod;
            del( url, isFile != 0);
            d->verifyState("del()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_CHMOD: {
            KUrl url;
            int i;
            stream >> url >> i;
            d->m_state = d->InsideMethod;
            chmod( url, i);
            d->verifyState("chmod()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_CHOWN: {
            KUrl url;
            QString owner, group;
            stream >> url >> owner >> group;
            d->m_state = d->InsideMethod;
            chown(url, owner, group);
            d->verifyState("chown()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_SETMODIFICATIONTIME: {
            KUrl url;
            QDateTime dt;
            stream >> url >> dt;
            d->m_state = d->InsideMethod;
            setModificationTime(url, dt);
            d->verifyState("setModificationTime()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_SPECIAL: {
            d->m_state = d->InsideMethod;
            special( data );
            d->verifyState("special()");
            d->m_state = d->Idle;
            break;
        }
        case CMD_META_DATA: {
            //kDebug(7019) << "(" << getpid() << ") Incoming meta-data...";
            stream >> mIncomingMetaData;
            d->rebuildConfig();
            break;
        }
        case CMD_SUBURL: {
            KUrl url;
            stream >> url;
            d->m_state = d->InsideMethod;
            setSubUrl(url);
            d->verifyErrorFinishedNotCalled("setSubUrl()");
            d->m_state = d->Idle;
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

bool SlaveBase::checkCachedAuthentication( AuthInfo& info )
{
    KPasswdStore* passwdstore = d->passwdStore();
    if (!passwdstore) {
        return false;
    }
    const qlonglong windowId = metaData(QLatin1String("window-id")).toLongLong();
    const QByteArray authkey = authInfoKey(info);
    if (passwdstore->hasPasswd(authkey, windowId)) {
        const QString passwd = passwdstore->getPasswd(authkey, windowId);
        info = authInfoFromData(passwd.toLatin1());
        return true;
    }
    return false;
}

void SlaveBase::dispatchOpenCommand( int command, const QByteArray &data )
{
    QDataStream stream( data );

    switch( command ) {
        case CMD_READ: {
            KIO::filesize_t bytes;
            stream >> bytes;
            read(bytes);
            break;
        }
        case CMD_WRITE: {
            write(data);
            break;
        }
        case CMD_SEEK: {
            KIO::filesize_t offset;
            stream >> offset;
            seek(offset);
            break;
        }
        case CMD_NONE: {
            break;
        }
        case CMD_CLOSE: {
            close();                // must call finish(), which will set d->inOpenLoop=false
            break;
        }
        default: {
            // Some command we don't understand.
            // Just ignore it, it may come from some future version of KDE.
            break;
        }
    }
}

bool SlaveBase::cacheAuthentication( const AuthInfo& info )
{
    KPasswdStore* passwdstore = d->passwdStore();

    if (!passwdstore) {
        return false;
    }

    passwdstore->storePasswd(authInfoKey(info), authInfoToData(info), metaData(QLatin1String("window-id")).toLongLong());
    return true;
}

int SlaveBase::connectTimeout()
{
    bool ok;
    QString tmp = metaData(QLatin1String("ConnectTimeout"));
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_CONNECT_TIMEOUT;
}

int SlaveBase::proxyConnectTimeout()
{
    bool ok;
    QString tmp = metaData(QLatin1String("ProxyConnectTimeout"));
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_PROXY_CONNECT_TIMEOUT;
}


int SlaveBase::responseTimeout()
{
    bool ok;
    QString tmp = metaData(QLatin1String("ResponseTimeout"));
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_RESPONSE_TIMEOUT;
}

int SlaveBase::readTimeout()
{
    bool ok;
    QString tmp = metaData(QLatin1String("ReadTimeout"));
    int result = tmp.toInt(&ok);
    if (ok)
       return result;
    return DEFAULT_READ_TIMEOUT;
}

bool SlaveBase::wasKilled() const
{
   return d->wasKilled;
}

void SlaveBase::setKillFlag()
{
   d->wasKilled=true;
}

void SlaveBase::send(int cmd, const QByteArray& arr )
{
   slaveWriteError = false;
   if (!d->appConnection.send(cmd, arr))
       // Note that slaveWriteError can also be set by sigpipe_handler
       slaveWriteError = true;
   if (slaveWriteError) exit();
}
