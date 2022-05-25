/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#define QT_NO_CAST_FROM_ASCII

#include "klauncher.h"
#include "klauncher_cmds.h"
#include "klauncher_adaptor.h"

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#ifdef Q_WS_X11
#include <kstartupinfo.h>
#include <X11/Xlib.h>
#endif

#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <qplatformdefs.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kde_file.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kdesktopfile.h>
#include <kurl.h>

#include <kio/global.h>
#include <kio/connection.h>
#include <kio/slaveinterface.h>

// Dispose slaves after being idle for SLAVE_MAX_IDLE seconds
#define SLAVE_MAX_IDLE	30

// #define KLAUNCHER_VERBOSE_OUTPUT

#ifdef KLAUNCHER_VERBOSE_OUTPUT
static const char* const s_DBusStartupTypeToString[] =
    { "DBusNone", "DBusUnique", "DBusMulti", "DBusWait", "ERROR" };
#endif

using namespace KIO;

IdleSlave::IdleSlave(QObject *parent)
    : QObject(parent)
{
   QObject::connect(&mConn, SIGNAL(readyRead()), this, SLOT(gotInput()));
   // Send it a SLAVE_STATUS command.
   mConn.send( CMD_SLAVE_STATUS );
   mPid = 0;
   mBirthDate = time(0);
   mOnHold = false;
}

template<int T> struct PIDType { typedef pid_t PID_t; } ;
template<> struct PIDType<2> { typedef qint16 PID_t; } ;
template<> struct PIDType<4> { typedef qint32 PID_t; } ;

void
IdleSlave::gotInput()
{
   int cmd;
   QByteArray data;
   if (mConn.read( &cmd, data) == -1)
   {
      // Communication problem with slave.
      // kError(7016) << "No communication with slave." << endl;
      deleteLater();
   }
   else if (cmd == MSG_SLAVE_ACK)
   {
      deleteLater();
   }
   else if (cmd != MSG_SLAVE_STATUS)
   {
      kError(7016) << "Unexpected data from slave." << endl;
      deleteLater();
   }
   else
   {
      QDataStream stream( data );
      PIDType<sizeof(pid_t)>::PID_t stream_pid;
      pid_t pid;
      QByteArray protocol;
      QString host;
      qint8 b;
      stream >> stream_pid >> protocol >> host >> b;
      pid = stream_pid;
// Overload with (bool) onHold, (KUrl) url.
      if (!stream.atEnd())
      {
         KUrl url;
         stream >> url;
         mOnHold = true;
         mUrl = url;
      }

      mPid = pid;
      mConnected = (b != 0);
      mProtocol = QString::fromLatin1(protocol);
      mHost = host;
      emit statusUpdate(this);
   }
}

void
IdleSlave::connect(const QString &app_socket)
{
   QByteArray data;
   QDataStream stream( &data, QIODevice::WriteOnly);
   stream << app_socket;
   mConn.send( CMD_SLAVE_CONNECT, data );
   // Timeout!
}

void
IdleSlave::reparseConfiguration()
{
   mConn.send( CMD_REPARSECONFIGURATION );
}

bool
IdleSlave::match(const QString &protocol, const QString &host, bool needConnected) const
{
   if (mOnHold || protocol != mProtocol) {
      return false;
   }
   if (host.isEmpty()) {
      return true;
   }
   return (host == mHost) && (!needConnected || mConnected);
}

bool
IdleSlave::onHold(const KUrl &url) const
{
   if (!mOnHold) return false;
   return (url == mUrl);
}

int
IdleSlave::age(time_t now) const
{
   return (int) difftime(now, mBirthDate);
}

static KLauncher* g_klauncher_self = NULL;


// From qcore_unix_p.h. We could also port to QLocalSocket :)
#define K_EINTR_LOOP(var, cmd)                    \
    do {                                        \
        var = cmd;                              \
    } while (var == -1 && errno == EINTR)

ssize_t kde_safe_write(int fd, const void *buf, size_t count)
{
    ssize_t ret = 0;
    K_EINTR_LOOP(ret, QT_WRITE(fd, buf, count));
    if (ret < 0)
        kWarning() << "write failed:" << strerror(errno);
    return ret;
}

KLauncher::KLauncher(int _kdeinitSocket)
  : QObject(0),
    kdeinitSocket(_kdeinitSocket)
{
#ifdef Q_WS_X11
   mCached_dpy = NULL;
#endif
   Q_ASSERT( g_klauncher_self == NULL );
   g_klauncher_self = this;

   mAutoTimer.setSingleShot(true);
   new KLauncherAdaptor(this);
   QDBusConnection::sessionBus().registerObject(QLatin1String("/KLauncher"), this); // same as ktoolinvocation.cpp

   connect(&mAutoTimer, SIGNAL(timeout()), this, SLOT(slotAutoStart()));
   connect(QDBusConnection::sessionBus().interface(),
           SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(slotNameOwnerChanged(QString,QString,QString)));

   mConnectionServer.listenForRemote();
   connect(&mConnectionServer, SIGNAL(newConnection()), SLOT(acceptSlave()));
   if (!mConnectionServer.isListening())
   {
      // Severe error!
      kDebug() << "Fatal error, can't create tempfile!";
      ::_exit(1);
   }

   connect(&mTimer, SIGNAL(timeout()), SLOT(idleTimeout()));

   kdeinitNotifier = new QSocketNotifier(kdeinitSocket, QSocketNotifier::Read);
   connect(kdeinitNotifier, SIGNAL(activated(int)),
           this, SLOT(slotKDEInitData(int)));
   kdeinitNotifier->setEnabled( true );
   lastRequest = 0;
   bProcessingQueue = false;

   mSlaveDebug = QString::fromLocal8Bit(qgetenv("KDE_SLAVE_DEBUG_WAIT"));
   if (!mSlaveDebug.isEmpty())
   {
      kWarning() << "Running in slave-debug mode for slaves of protocol:" << qPrintable(mSlaveDebug);
   }
   mSlaveValgrind = QString::fromLocal8Bit(qgetenv("KDE_SLAVE_VALGRIND"));
   if (!mSlaveValgrind.isEmpty())
   {
      mSlaveValgrindSkin = QString::fromLocal8Bit(qgetenv("KDE_SLAVE_VALGRIND_SKIN"));
      kWarning() << "Running slaves through valgrind for slaves of protocol:" << qPrintable(mSlaveValgrind);
   }
   klauncher_header request_header;
   request_header.cmd = LAUNCHER_OK;
   request_header.arg_length = 0;
   kde_safe_write(kdeinitSocket, &request_header, sizeof(request_header));
}

KLauncher::~KLauncher()
{
   close();
   g_klauncher_self = NULL;
}

void KLauncher::close()
{
#ifdef Q_WS_X11
   if( mCached_dpy != NULL )
   {
       XCloseDisplay( mCached_dpy );
       mCached_dpy = NULL;
   }
#endif
}

void
KLauncher::destruct()
{
    if (g_klauncher_self)
        g_klauncher_self->close();
    // We don't delete the app here, that's intentional.
    ::_exit(255);
}

void KLauncher::setLaunchEnv(const QString &name, const QString &value)
{
   klauncher_header request_header;
   QByteArray requestData;
   requestData.append(name.toLocal8Bit()).append('\0').append(value.toLocal8Bit()).append('\0');
   request_header.cmd = LAUNCHER_SETENV;
   request_header.arg_length = requestData.size();
   kde_safe_write(kdeinitSocket, &request_header, sizeof(request_header));
   kde_safe_write(kdeinitSocket, requestData.data(), request_header.arg_length);
}

/*
 * Read 'len' bytes from 'sock' into buffer.
 * returns -1 on failure, 0 on no data.
 */
static int
read_socket(int sock, char *buffer, int len)
{
  ssize_t result;
  int bytes_left = len;
    while (bytes_left > 0) {
        // in case we get a request to start an application and data arrive
        // to kdeinitSocket at the same time, requestStart() will already
        // call slotKDEInitData(), so we must check there's still something
        // to read, otherwise this would block

        // Same thing if kdeinit dies without warning.

        fd_set in;
        timeval tm = { 30, 0 }; // 30 seconds timeout, so we're not stuck in case kdeinit dies on us
        FD_ZERO ( &in );
        FD_SET( sock, &in );
        select( sock + 1, &in, 0, 0, &tm );
        if( !FD_ISSET( sock, &in )) {
            kDebug(7016) << "read_socket" << sock << "nothing to read, kdeinit4 must be dead";
            return -1;
        }

     result = read(sock, buffer, bytes_left);
     if (result > 0)
     {
        buffer += result;
        bytes_left -= result;
     }
     else if (result == 0)
        return -1;
     else if ((result == -1) && (errno != EINTR))
        return -1;
  }
  return 0;
}

void
KLauncher::slotKDEInitData(int)
{
   klauncher_header request_header;
   QByteArray requestData;

   int result = read_socket(kdeinitSocket, (char *) &request_header,
                            sizeof( request_header));
   if (result == -1)
   {
      kDebug(7016) << "Exiting on read_socket errno:" << errno;
      KDE_signal( SIGHUP, SIG_IGN);
      KDE_signal( SIGTERM, SIG_IGN);
      destruct(); // Exit!
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(kdeinitSocket, (char *) requestData.data(),
                        request_header.arg_length);

   processRequestReturn(request_header.cmd,requestData);
}

void KLauncher::processRequestReturn(int status, const QByteArray &requestData)
{
   if (status == LAUNCHER_CHILD_DIED)
   {
     long *request_data = (long *) requestData.data();
     processDied(request_data[0], request_data[1]);
     return;
   }
   if (lastRequest && (status == LAUNCHER_OK))
   {
     long *request_data = (long *) requestData.data();
     lastRequest->pid = (pid_t) (*request_data);
     kDebug(7016).nospace() << lastRequest->name << " (pid " << lastRequest->pid <<
        ") up and running.";
     switch(lastRequest->dbus_startup_type)
     {
       case KService::DBusNone:
         lastRequest->status = KLaunchRequest::Running;
         break;
       case KService::DBusUnique:
       case KService::DBusWait:
       case KService::DBusMulti:
         lastRequest->status = KLaunchRequest::Launching;
         break;
     }
     lastRequest = 0;
     return;
   }
   if (lastRequest && (status == LAUNCHER_ERROR))
   {
     lastRequest->status = KLaunchRequest::Error;
     kDebug(7016) << lastRequest->name << " failed." << endl;
     if (!requestData.isEmpty())
        lastRequest->errorMsg = QString::fromUtf8((char *) requestData.data());
     lastRequest = 0;
     return;
   }

   kWarning(7016)<< "Unexpected request return" << (unsigned int) status;
}

void
KLauncher::processDied(pid_t pid, long exitStatus)
{
#ifdef KLAUNCHER_VERBOSE_OUTPUT
    kDebug(7016) << pid << "exitStatus=" << exitStatus;
#else
    Q_UNUSED(exitStatus);
    // We should probably check the exitStatus for the uniqueapp case?
#endif
   foreach (KLaunchRequest *request, requestList)
   {
#ifdef KLAUNCHER_VERBOSE_OUTPUT
       kDebug(7016) << "  had pending request" << request->pid;
#endif
      if (request->pid == pid)
      {
         if (request->dbus_startup_type == KService::DBusWait)
             request->status = KLaunchRequest::Done;
         else if ((request->dbus_startup_type == KService::DBusUnique)
                  && QDBusConnection::sessionBus().interface()->isServiceRegistered(request->dbus_name)) {
             request->status = KLaunchRequest::Running;
#ifdef KLAUNCHER_VERBOSE_OUTPUT
             kDebug(7016) << pid << "running as a unique app";
#endif
         } else {
             request->status = KLaunchRequest::Error;
#ifdef KLAUNCHER_VERBOSE_OUTPUT
             kDebug(7016) << pid << "died, requestDone. status=" << request->status;
#endif
         }
         requestDone(request);
         return;
      }
   }
#ifdef KLAUNCHER_VERBOSE_OUTPUT
   kDebug(7016) << "found no pending requests for PID" << pid;
#endif
}

static bool matchesPendingRequest(const QString& appId, const QString& pendingAppId)
{
    // appId just registered, e.g. org.koffice.kword-12345
    // Let's see if this is what pendingAppId (e.g. org.koffice.kword or *.kword) was waiting for.

    const QString newAppId = appId.left(appId.lastIndexOf(QLatin1Char('-'))); // strip out the -12345 if present.

    //kDebug() << "appId=" << appId << "newAppId=" << newAppId << "pendingAppId=" << pendingAppId;

    if (pendingAppId.startsWith(QLatin1String("*."))) {
        const QString pendingName = pendingAppId.mid(2);
        const QString appName = newAppId.mid(newAppId.lastIndexOf(QLatin1Char('.'))+1);
        //kDebug() << "appName=" << appName;
        return appName == pendingName;
    }

    return newAppId == pendingAppId;
}

void
KLauncher::slotNameOwnerChanged(const QString &appId, const QString &oldOwner,
                                const QString &newOwner)
{
   Q_UNUSED(oldOwner);
   if (appId.isEmpty() || newOwner.isEmpty())
      return;

#ifdef KLAUNCHER_VERBOSE_OUTPUT
   kDebug(7016) << "new app" << appId;
#endif
   foreach (KLaunchRequest *request, requestList)
   {
      if (request->status != KLaunchRequest::Launching)
         continue;

#ifdef KLAUNCHER_VERBOSE_OUTPUT
      kDebug(7016) << "had pending request" << request->name << s_DBusStartupTypeToString[request->dbus_startup_type] << "dbus_name" << request->dbus_name << request->tolerant_dbus_name;
#endif
      // For unique services check the requested service name first
      if (request->dbus_startup_type == KService::DBusUnique) {
          if ((appId == request->dbus_name) || // just started
              QDBusConnection::sessionBus().interface()->isServiceRegistered(request->dbus_name)) { // was already running
              request->status = KLaunchRequest::Running;
#ifdef KLAUNCHER_VERBOSE_OUTPUT
              kDebug(7016) << "OK, unique app" << request->dbus_name << "is running";
#endif
              requestDone(request);
              continue;
          } else {
#ifdef KLAUNCHER_VERBOSE_OUTPUT
              kDebug(7016) << "unique app" << request->dbus_name << "not running yet";
#endif
          }
      }

      const QString rAppId = !request->tolerant_dbus_name.isEmpty() ? request->tolerant_dbus_name : request->dbus_name;
#ifdef KLAUNCHER_VERBOSE_OUTPUT
      //kDebug(7016) << "using" << rAppId << "for matching";
#endif
      if (rAppId.isEmpty())
          continue;

      if (matchesPendingRequest(appId, rAppId)) {
#ifdef KLAUNCHER_VERBOSE_OUTPUT
         kDebug(7016) << "ok, request done";
#endif
         request->dbus_name = appId;
         request->status = KLaunchRequest::Running;
         requestDone(request);
         continue;
      }
   }
}

void
KLauncher::autoStart(int phase)
{
   if( mAutoStart.phase() >= phase )
       return;
   mAutoStart.setPhase(phase);
   if (phase == 0)
      mAutoStart.loadAutoStartList();
   mAutoTimer.start(0);
}

void
KLauncher::slotAutoStart()
{
   KService::Ptr s;
   do
   {
      QString service = mAutoStart.startService();
      if (service.isEmpty())
      {
         // Done
        if( !mAutoStart.phaseDone())
        {
            mAutoStart.setPhaseDone();
            switch( mAutoStart.phase())
                {
                case 0:
                    emit autoStart0Done();
                    break;
                case 1:
                    emit autoStart1Done();
                    break;
                case 2:
                    emit autoStart2Done();
                    break;
                }
	 }
         return;
      }
      s = new KService(service);
   }
   while (!start_service(s, QStringList(), QStringList(), "0", false, true, QDBusMessage()));
   // Loop till we find a service that we can start.
}

void
KLauncher::requestDone(KLaunchRequest *request)
{
   if ((request->status == KLaunchRequest::Running) ||
       (request->status == KLaunchRequest::Done))
   {
      requestResult.result = 0;
      requestResult.dbusName = request->dbus_name;
      requestResult.error = QString::fromLatin1(""); // not null, cf assert further down
      requestResult.pid = request->pid;
   }
   else
   {
      requestResult.result = 1;
      requestResult.dbusName.clear();
      requestResult.error = i18n("KDEInit could not launch '%1'", request->name);
      if (!request->errorMsg.isEmpty())
          requestResult.error += QString::fromLatin1(":\n") + request->errorMsg;
      requestResult.pid = 0;

#ifdef Q_WS_X11
      if (!request->startup_dpy.isEmpty())
      {
         Display* dpy = NULL;
         if( (mCached_dpy != NULL) &&
              (request->startup_dpy == XDisplayString( mCached_dpy )))
            dpy = mCached_dpy;
         if( dpy == NULL )
            dpy = XOpenDisplay(request->startup_dpy);
         if( dpy )
         {
            KStartupInfoId id;
            id.initId(request->startup_id);
            KStartupInfo::sendFinishX( dpy, id );
            if( mCached_dpy != dpy && mCached_dpy != NULL )
               XCloseDisplay( mCached_dpy );
            mCached_dpy = dpy;
         }
      }
#endif
   }

   if (request->autoStart)
   {
      mAutoTimer.start(0);
   }

   if (request->transaction.type() != QDBusMessage::InvalidMessage)
   {
      if ( requestResult.dbusName.isNull() ) // null strings can't be sent
          requestResult.dbusName.clear();
      Q_ASSERT( !requestResult.error.isNull() );
      PIDType<sizeof(pid_t)>::PID_t stream_pid = requestResult.pid;
      QDBusConnection::sessionBus().send(request->transaction.createReply(QVariantList() << requestResult.result
                                     << requestResult.dbusName
                                     << requestResult.error
                                     << stream_pid));
   }
#ifdef KLAUNCHER_VERBOSE_OUTPUT
   kDebug(7016) << "removing done request" << request->name << "PID" << request->pid;
#endif

   requestList.removeAll( request );
   delete request;
}

static void appendLong(QByteArray &ba, long l)
{
   const int sz = ba.size();
   ba.resize(sz + sizeof(long));
   memcpy(ba.data() + sz, &l, sizeof(long));
}

void
KLauncher::requestStart(KLaunchRequest *request)
{
   requestList.append( request );
   // Send request to kdeinit.
   klauncher_header request_header;
   QByteArray requestData;
   requestData.reserve(1024);

   appendLong(requestData, request->arg_list.count() + 1);
   requestData.append(request->name.toLocal8Bit());
   requestData.append('\0');
   foreach (const QString &arg, request->arg_list)
       requestData.append(arg.toLocal8Bit()).append('\0');
   appendLong(requestData, request->envs.count());
   foreach (const QString &env, request->envs)
       requestData.append(env.toLocal8Bit()).append('\0');
   appendLong(requestData, 0); // avoid_loops, always false here
#ifdef Q_WS_X11
   bool startup_notify = !request->startup_id.isNull() && request->startup_id != "0";
   if( startup_notify )
       requestData.append(request->startup_id).append('\0');
#endif
   if (!request->cwd.isEmpty())
       requestData.append(QFile::encodeName(request->cwd)).append('\0');

#ifdef Q_WS_X11
   request_header.cmd = startup_notify ? LAUNCHER_EXT_EXEC : LAUNCHER_EXEC_NEW;
#else
   request_header.cmd = LAUNCHER_EXEC_NEW;
#endif
   request_header.arg_length = requestData.length();

#ifdef KLAUNCHER_VERBOSE_OUTPUT
   kDebug(7016) << "Asking kdeinit to start" << request->name << request->arg_list
                << "cmd=" << commandToString(request_header.cmd);
#endif

   kde_safe_write(kdeinitSocket, &request_header, sizeof(request_header));
   kde_safe_write(kdeinitSocket, requestData.data(), requestData.length());

   // Wait for pid to return.
   lastRequest = request;
   do {
      slotKDEInitData( kdeinitSocket );
   }
   while (lastRequest != 0);
}

void KLauncher::exec_blind(const QString &name, const QStringList &arg_list)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;
   request->name = name;
   request->arg_list =  arg_list;
   request->dbus_startup_type = KService::DBusNone;
   request->pid = 0;
   request->status = KLaunchRequest::Launching;
   request->envs = QStringList();

   requestStart(request);
   // We don't care about this request any longer....
   requestDone(request);
}

bool
KLauncher::start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
    const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg)
{
   KService::Ptr service;
   // Find service
   const QFileInfo fi(serviceName);
   if (fi.isAbsolute() && fi.exists())
   {
      // Full path
      service = new KService(serviceName);
   }
   else
   {
      service = KService::serviceByDesktopPath(serviceName);
      // TODO?
      //if (!service)
      //    service = KService::serviceByStorageId(serviceName); // This method should be named start_service_by_storage_id ideally...
   }
   if (!service)
   {
      requestResult.result = ENOENT;
      requestResult.error = i18n("Could not find service '%1'.", serviceName);
      cancel_service_startup_info( NULL, startup_id.toLocal8Bit(), envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id.toLocal8Bit(), blind, false, msg);
}

bool
KLauncher::start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
    const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg)
{
   KService::Ptr service = KService::serviceByDesktopName(serviceName);
   if (!service)
   {
      requestResult.result = ENOENT;
      requestResult.error = i18n("Could not find service '%1'.", serviceName);
      cancel_service_startup_info( NULL, startup_id.toLocal8Bit(), envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id.toLocal8Bit(), blind, false, msg);
}

bool
KLauncher::start_service(KService::Ptr service, const QStringList &_urls,
                         const QStringList &envs, const QByteArray &startup_id,
                         bool blind, bool autoStart, QDBusMessage msg)
{
   QStringList urls = _urls;
   bool runPermitted = KDesktopFile::isAuthorizedDesktopFile(service->entryPath());

   if (!service->isValid() || !runPermitted)
   {
      requestResult.result = ENOEXEC;
      if (service->isValid())
         requestResult.error = i18n("Service '%1' must be executable to run.", service->entryPath());
      else
         requestResult.error = i18n("Service '%1' is malformatted.", service->entryPath());
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = autoStart;

   if ((urls.count() > 1) && !service->allowMultipleFiles())
   {
      // We need to launch the application N times. That sucks.
      // We ignore the result for application 2 to N.
      // For the first file we launch the application in the
      // usual way. The reported result is based on this
      // application.
      foreach(const QString it, urls) {
         QByteArray startup_id2 = startup_id;
         const QStringList singleUrl(it);
         if( !startup_id2.isEmpty() && startup_id2 != "0" )
             startup_id2 = "0"; // can't use the same startup_id several times // krazy:exclude=doublequote_chars
         start_service( service, singleUrl, envs, startup_id2, true, false, msg);
      }
      QString firstURL = *(urls.begin());
      urls.clear();
      urls.append(firstURL);
   }
   createArgs(request, service, urls);

   // We must have one argument at least!
   if (!request->arg_list.count())
   {
      requestResult.result = ENOEXEC;
      requestResult.error = i18n("Service '%1' is malformatted.", service->entryPath());
      delete request;
      cancel_service_startup_info( NULL, startup_id, envs );
      return false;
   }

   request->name = request->arg_list.takeFirst();

   if (request->name.endsWith(QLatin1String("/kioexec"))) {
       // Special case for kioexec; if createArgs said we were going to use it,
       // then we have to expect a kioexec-PID, not a org.kde.finalapp...
       // Testcase: konqueror www.kde.org, RMB on link, open with, kruler.

       request->dbus_startup_type = KService::DBusMulti;
       request->dbus_name = QString::fromLatin1("org.kde.kioexec");
   } else {
       request->dbus_startup_type = service->dbusStartupType();

       if ((request->dbus_startup_type == KService::DBusUnique) ||
           (request->dbus_startup_type == KService::DBusMulti)) {
           const QVariant v = service->property(QLatin1String("X-DBUS-ServiceName"));
           if (v.isValid()) {
               request->dbus_name = v.toString();
           }
           if (request->dbus_name.isEmpty()) {
               const QString binName = KRun::binaryName(service->exec(), true);
               request->dbus_name = QString::fromLatin1("org.kde.") + binName;
               request->tolerant_dbus_name = QString::fromLatin1("*.") + binName;
           }
       }
   }

#ifdef KLAUNCHER_VERBOSE_OUTPUT
   kDebug(7016) << "name=" << request->name << "dbus_name=" << request->dbus_name
                << "startup type=" << s_DBusStartupTypeToString[request->dbus_startup_type];
#endif

   request->pid = 0;
   request->envs = envs;
   send_service_startup_info( request, service, startup_id, envs );

   // Request will be handled later.
   if (!blind && !autoStart)
   {
      msg.setDelayedReply(true);
      request->transaction = msg;
   }
   queueRequest(request);
   return true;
}

void
KLauncher::send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QByteArray& startup_id,
    const QStringList &envs )
{
#ifdef Q_WS_X11
    request->startup_id = "0";// krazy:exclude=doublequote_chars
    if (startup_id == "0")
        return;
    bool silent;
    QByteArray wmclass;
    if( !KRun::checkStartupNotify( QString(), service.data(), &silent, &wmclass ))
        return;
    KStartupInfoId id;
    id.initId(startup_id);
    QByteArray dpy_str;
    foreach (const QString &env, envs) {
        if (env.startsWith(QLatin1String("DISPLAY=")))
            dpy_str = env.mid(8).toLocal8Bit();
    }
    Display* dpy = NULL;
    if (!dpy_str.isEmpty() && mCached_dpy != NULL && dpy_str != XDisplayString(mCached_dpy))
        dpy = mCached_dpy;
    if (dpy == NULL)
        dpy = XOpenDisplay(dpy_str);
    request->startup_id = id.id();
    if (dpy == NULL) {
        cancel_service_startup_info( request, startup_id, envs );
        return;
    }

    request->startup_dpy = dpy_str;

    KStartupInfoData data;
    data.setName( service->name());
    data.setIcon( service->icon());
    data.setDescription( i18n( "Launching %1" ,  service->name()));
    if( !wmclass.isEmpty())
        data.setWMClass( wmclass );
    if( silent )
        data.setSilent( KStartupInfoData::Yes );
    data.setApplicationId( service->entryPath());
    // the rest will be sent by kdeinit
    KStartupInfo::sendStartupX( dpy, id, data );
    if( mCached_dpy != dpy && mCached_dpy != NULL )
        XCloseDisplay( mCached_dpy );
    mCached_dpy = dpy;
    return;
#else
    return;
#endif
}

void
KLauncher::cancel_service_startup_info( KLaunchRequest* request, const QByteArray& startup_id,
    const QStringList &envs )
{
#ifdef Q_WS_X11
    if( request != NULL )
        request->startup_id = "0"; // krazy:exclude=doublequote_chars
    if( !startup_id.isEmpty() && startup_id != "0" )
    {
        QString dpy_str;
        foreach (const QString &env, envs) {
            if (env.startsWith(QLatin1String("DISPLAY=")))
                dpy_str = env.mid(8);
        }
        Display* dpy = NULL;
        if( !dpy_str.isEmpty() && mCached_dpy != NULL
            && dpy_str != QString::fromLatin1(XDisplayString( mCached_dpy )) )
            dpy = mCached_dpy;
        if( dpy == NULL )
            dpy = XOpenDisplay( dpy_str.toLatin1().constData() );
        if( dpy == NULL )
            return;
        KStartupInfoId id;
        id.initId(startup_id);
        KStartupInfo::sendFinishX( dpy, id );
        if( mCached_dpy != dpy && mCached_dpy != NULL )
           XCloseDisplay( mCached_dpy );
        mCached_dpy = dpy;
    }
#endif
}

bool
KLauncher::kdeinit_exec(const QString &app, const QStringList &args,
                        const QString& workdir, const QStringList &envs,
                        const QString &startup_id, bool wait, QDBusMessage msg)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;
   request->arg_list = args;
   request->name = app;
   if (wait)
      request->dbus_startup_type = KService::DBusWait;
   else
      request->dbus_startup_type = KService::DBusNone;
   request->pid = 0;
#ifdef Q_WS_X11
   request->startup_id = startup_id.toLocal8Bit();
#endif
   request->envs = envs;
   request->cwd = workdir;
#ifdef Q_WS_X11
   if (!app.endsWith(QLatin1String("kbuildsycoca4"))) { // avoid stupid loop
       // Find service, if any - strip path if needed
       const QString desktopName = app.mid(app.lastIndexOf(QLatin1Char('/')) + 1);
       KService::Ptr service = KService::serviceByDesktopName(desktopName);
       if (service)
           send_service_startup_info(request, service,
                                     request->startup_id, envs);
       else // no .desktop file, no startup info
           cancel_service_startup_info(request, request->startup_id, envs);
   }
#endif
   msg.setDelayedReply(true);
   request->transaction = msg;
   queueRequest(request);
   return true;
}

void
KLauncher::queueRequest(KLaunchRequest *request)
{
   requestQueue.append( request );
   if (!bProcessingQueue)
   {
      bProcessingQueue = true;
      QTimer::singleShot(0, this, SLOT(slotDequeue()));
   }
}

void
KLauncher::slotDequeue()
{
   do {
      KLaunchRequest *request = requestQueue.takeFirst();
      // process request
      request->status = KLaunchRequest::Launching;
      requestStart(request);
      if (request->status != KLaunchRequest::Launching)
      {
         // Request handled.
#ifdef KLAUNCHER_VERBOSE_OUTPUT
         kDebug(7016) << "Request handled already";
#endif
         requestDone( request );
         continue;
      }
   } while(requestQueue.count());
   bProcessingQueue = false;
}

void
KLauncher::createArgs( KLaunchRequest *request, const KService::Ptr service ,
                       const QStringList &urls)
{
  const QStringList params = KRun::processDesktopExec(*service, urls);

  for(QStringList::ConstIterator it = params.begin();
      it != params.end(); ++it)
  {
     request->arg_list.append(*it);
  }

  const QString& path = service->path();
  if (!path.isEmpty()) {
      request->cwd = path;
  } else if (!urls.isEmpty()) {
      const KUrl url(urls.first());
      if (url.isLocalFile()) {
          request->cwd = url.directory();
      }
  }
}

///// IO-Slave functions

pid_t
KLauncher::requestHoldSlave(const KUrl &url, const QString &app_socket)
{
    IdleSlave *slave = 0;
    foreach (IdleSlave *p, mSlaveList)
    {
       if (p->onHold(url))
       {
          slave = p;
          break;
       }
    }
    if (slave)
    {
       mSlaveList.removeAll(slave);
       slave->connect(app_socket);
       return slave->pid();
    }
    return 0;
}

pid_t
KLauncher::requestSlave(const QString &protocol,
                        const QString &host,
                        const QString &app_socket,
                        QString &error)
{
    IdleSlave *slave = 0;
    foreach (IdleSlave *p, mSlaveList)
    {
       if (p->match(protocol, host, true))
       {
          slave = p;
          break;
       }
    }
    if (!slave)
    {
       foreach (IdleSlave *p, mSlaveList)
       {
          if (p->match(protocol, host, false))
          {
             slave = p;
             break;
          }
       }
    }
    if (!slave)
    {
       foreach (IdleSlave *p, mSlaveList)
       {
          if (p->match(protocol, QString(), false))
          {
             slave = p;
             break;
          }
       }
    }
    if (slave)
    {
       mSlaveList.removeAll(slave);
       slave->connect(app_socket);
       return slave->pid();
    }

    QString name = KProtocolInfo::exec(protocol);
    if (name.isEmpty())
    {
	error = i18n("Unknown protocol '%1'.\n", protocol);
        return 0;
    }

    QStringList arg_list;
    arg_list.append(protocol);
    arg_list.append(mConnectionServer.address());
    arg_list.append(app_socket);

    kDebug(7016) << "KLauncher: launching new slave " << name << " with protocol=" << protocol
     << " args=" << arg_list << endl;

#ifdef Q_OS_UNIX
    if (mSlaveDebug == protocol)
    {
       klauncher_header request_header;
       request_header.cmd = LAUNCHER_DEBUG_WAIT;
       request_header.arg_length = 0;
       kde_safe_write(kdeinitSocket, &request_header, sizeof(request_header));
    }
    if (mSlaveValgrind == protocol) {
       KPluginLoader lib(name, KGlobal::mainComponent());
       arg_list.prepend(lib.fileName());
       arg_list.prepend(KStandardDirs::locate("exe", QString::fromLatin1("kioslave")));
       name = QString::fromLatin1("valgrind");

       if (!mSlaveValgrindSkin.isEmpty()) {
           arg_list.prepend(QLatin1String("--tool=") + mSlaveValgrindSkin);
       } else {
           arg_list.prepend(QLatin1String("--tool=memcheck"));
       }
    }
#endif
    KLaunchRequest *request = new KLaunchRequest;
    request->autoStart = false;
    request->name = name;
    request->arg_list =  arg_list;
    request->dbus_startup_type = KService::DBusNone;
    request->pid = 0;
#ifdef Q_WS_X11
    request->startup_id = "0"; // krazy:exclude=doublequote_chars
#endif
    request->status = KLaunchRequest::Launching;
    requestStart(request);
    pid_t pid = request->pid;

//    kDebug(7016) << "Slave launched, pid = " << pid;

    // We don't care about this request any longer....
    requestDone(request);
    if (!pid)
    {
       error = i18n("Error loading '%1'.\n", name);
    }
    return pid;
}

bool KLauncher::checkForHeldSlave(const QString &url)
{
    Q_FOREACH (const IdleSlave *p, mSlaveList) {
       if (p->onHold(url)) {
          return true;
       }
    }
    return false;
}

void
KLauncher::waitForSlave(pid_t pid, QDBusMessage msg)
{
    foreach (IdleSlave *slave, mSlaveList)
    {
        if (slave->pid() == pid)
           return; // Already here.
    }
    SlaveWaitRequest *waitRequest = new SlaveWaitRequest;
    msg.setDelayedReply(true);
    waitRequest->transaction = msg;
    waitRequest->pid = pid;
    mSlaveWaitRequest.append(waitRequest);
}

void
KLauncher::acceptSlave()
{
    IdleSlave *slave = new IdleSlave(this);
    mConnectionServer.setNextPendingConnection(&slave->mConn);
    mSlaveList.append(slave);
    connect(slave, SIGNAL(destroyed()), this, SLOT(slotSlaveGone()));
    connect(slave, SIGNAL(statusUpdate(IdleSlave*)),
           this, SLOT(slotSlaveStatus(IdleSlave*)));
    if (!mTimer.isActive())
    {
       mTimer.start(1000*10);
    }
}

void
KLauncher::slotSlaveStatus(IdleSlave *slave)
{
    QMutableListIterator<SlaveWaitRequest *> it(mSlaveWaitRequest);
    while(it.hasNext())
    {
       SlaveWaitRequest *waitRequest = it.next();
       if (waitRequest->pid == slave->pid())
       {
           QDBusConnection::sessionBus().send(waitRequest->transaction.createReply());
          it.remove();
          delete waitRequest;
       }
    }
}

void
KLauncher::slotSlaveGone()
{
    IdleSlave *slave = (IdleSlave *) sender();
    mSlaveList.removeAll(slave);
    if ((mSlaveList.count() == 0) && (mTimer.isActive()))
    {
       mTimer.stop();
    }
}

void
KLauncher::idleTimeout()
{
    const time_t now = time(0);
    foreach (IdleSlave *slave, mSlaveList)
    {
        if (slave->age(now) > SLAVE_MAX_IDLE)
        {
           // killing idle slave
           delete slave;
        }
    }
}

void KLauncher::reparseConfiguration()
{
   KProtocolManager::reparseConfiguration();
   foreach (IdleSlave *slave, mSlaveList)
      slave->reparseConfiguration();
}

void KLauncher::terminate_kdeinit()
{
    kDebug(7016);
    klauncher_header request_header;
    request_header.cmd = LAUNCHER_TERMINATE_KDEINIT;
    request_header.arg_length = 0;
    kde_safe_write(kdeinitSocket, &request_header, sizeof(request_header));
}

#include "moc_klauncher.cpp"
