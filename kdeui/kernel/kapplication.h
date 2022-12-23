/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef KAPP_H
#define KAPP_H

#include <kdeui_export.h>

class KConfig;

#include <QtGui/QApplication>
#include <kcomponentdata.h>
#include <kglobal.h>

#ifdef Q_WS_X11
#include <QtGui/qx11info_x11.h>
#endif

#define kapp KApplication::kApplication()
#define KAPPLICATION_GUI_TYPE KApplication::Gui

class KApplicationPrivate;

/**
* Controls and provides information to all KDE applications.
*
* Only one object of this class can be instantiated in a single app.
* This instance is always accessible via the 'kapp' global variable.
*
* This class provides the following services to all KDE applications.
*
* @li It controls the event queue (see QApplication ).
* @li It provides the application with KDE resources such as
* accelerators, common menu entries, a KConfig object. session
* management events, help invocation etc.
* @li Installs an empty signal handler for the SIGPIPE signal.
* If you want to catch this signal
* yourself, you have set a new signal handler after KApplication's
* constructor has run.
* @li It can start new services
*
*
* @short Controls and provides information to all KDE applications.
* @author Matthias Kalle Dalheimer <kalle@kde.org>
*/
class KDEUI_EXPORT KApplication : public QApplication
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.KApplication")
public:
  /**
   * This constructor is the one you should use.
   * It takes aboutData and command line arguments from KCmdLineArgs.
   *
   * Note that for a non-GUI daemon, you might want to use QCoreApplication
   * and a KComponentData instance instead. You'll save an unnecessary dependency
   * to kdeui. The main difference is that you will have to do a number of things yourself:
   * <ul>
   *  <li>Register to DBus, if necessary.</li>
   *  <li>Call KGlobal::locale(), if using multiple threads.</li>
   * </ul>
   */
  explicit KApplication();

#ifdef Q_WS_X11
  /**
   * Constructor. Parses command-line arguments. Use this constructor when you
   * you need to use a non-default visual or colormap.
   *
   * @param display Will be passed to Qt as the X display. The display must be
   * valid and already opened.
   *
   * @param visual A pointer to the X11 visual that should be used by the
   * application. Note that only TrueColor visuals are supported on depths
   * greater than 8 bpp. If this parameter is NULL, the default visual will
   * be used instead.
   *
   * @param colormap The colormap that should be used by the application. If
   * this parameter is 0, the default colormap will be used instead.
   */
  explicit KApplication(Display *display, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);

  /**
   * Constructor. Parses command-line arguments. Use this constructor to use KApplication
   * in a Motif or Xt program.
   *
   * @param display Will be passed to Qt as the X display. The display must be valid and already
   * opened.
   *
   * @param argc command line argument count
   *
   * @param argv command line argument value(s)
   *
   * @param rAppName application name. Will be used for finding the
   * associated message files and icon files, and as the default
   * registration name for DCOP. This is a mandatory parameter.
   */
  explicit KApplication(Display *display, int& argc, char** argv, const QByteArray& rAppName);
#endif

  virtual ~KApplication();

  /**
   * Returns the current application object.
   *
   * This is similar to the global QApplication pointer qApp. It
   * allows access to the single global KApplication object, since
   * more than one cannot be created in the same application. It
   * saves you the trouble of having to pass the pointer explicitly
   * to every function that may require it.
   * @return the current application object
   */
  static KApplication* kApplication();

  /**
   * Returns the application session config object.
   *
   * @return A pointer to the application's instance specific
   * KConfig object.
   * @see KConfig
   */
  KConfig* sessionConfig();


  /**
   * Disables session management for this application.
   *
   * Useful in case  your application is started by the
   * initial "startkde" script.
   */
  void disableSessionManagement();

  /**
   * Enables session management for this application, formerly
   * disabled by calling disableSessionManagement(). You usually
   * shouldn't call this function, as session management is enabled
   * by default.
   */
  void enableSessionManagement();

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManager.
     * @internal
     */
  void commitData( QSessionManager& sm );

    /**
     * Reimplemented for internal purposes, mainly the highlevel
     *  handling of session management with KSessionManager.
     * @internal
     */
  void saveState( QSessionManager& sm );

  /**
   * Returns true if the application is currently saving its session
   * data (most probably before KDE logout). This is intended for use
   * mainly in KMainWindow::queryClose().
   *
   * @see KMainWindow::queryClose
   */
  bool sessionSaving() const;


  /**
   *  Sets the top widget of the application.
   *  This means basically applying the right window caption.
   *  An application may have several top widgets. You don't
   *  need to call this function manually when using KMainWindow.
   *
   *  @param topWidget A top widget of the application.
   *
   *  @see icon(), caption()
   **/
  void setTopWidget( QWidget *topWidget );

  /**
   *  Installs widget filter as global X11 event filter.
   *
   * The widget
   *  filter receives XEvents in its standard QWidget::x11Event() function.
   *
   *  Warning: Only do this when absolutely necessary. An installed X11 filter
   *  can slow things down.
   **/
  void installX11EventFilter( QWidget* filter );

  /**
   * Removes global X11 event filter previously installed by
   * installX11EventFilter().
   */
  void removeX11EventFilter( const QWidget* filter );


  /**
   * Returns the app startup notification identifier for this running
   * application.
   * @return the startup notification identifier
   */
  QByteArray startupId() const;

  /**
   * @internal
   * Sets a new value for the application startup notification window property for newly
   * created toplevel windows.
   * @param startup_id the startup notification identifier
   * @see KStartupInfo::setNewStartupId
   */
  void setStartupId( const QByteArray& startup_id );
  /**
   * @internal
   * Used only by KStartupId.
   */
  void clearStartupId();

  /**
   * Sets how the primary and clipboard selections are synchronized in an X11 environment
   */
  void setSynchronizeClipboard(bool synchronize);

  /**
   * Returns the last user action timestamp or 0 if no user activity has taken place yet.
   * @see updateuserTimestamp
   */
  unsigned long userTimestamp() const;

  /**
   * Updates the last user action timestamp in the application registered to DBUS with id service
   * to the given time, or to this application's user time, if 0 is given.
   * Use before causing user interaction in the remote application, e.g. invoking a dialog
   * in the application using a DCOP call.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   */
  void updateRemoteUserTimestamp( const QString& service, int time = 0 );

  /**
   * Setups signal handler for SIGTERM, SIGHUP and SIGINT to call QApplication::quit() when such
   * signal is received.
   * @note By default KApplication constructor calls this static method, unless QCoreApplication
   * or QApplication instance is used you should not call it.
   */
  static void quitOnSignal();

  /**
   * Connects D-Bus disconnected to QApplication::quit().
   * @note By default KApplication constructor calls this static method, unless QCoreApplication
   * or QApplication instance is used you should not call it.
   * @since 4.23
   */
  static void quitOnDisconnected();

#ifdef Q_WS_X11
  /**
      @internal
    */
  bool notify( QObject* receiver, QEvent* event );
#endif // Q_WS_X11

public Q_SLOTS:
  /**
   * Updates the last user action timestamp to the given time, or to the current time,
   * if 0 is given. Do not use unless you're really sure what you're doing.
   * Consult focus stealing prevention section in kdebase/kwin/README.
   */
  Q_SCRIPTABLE void updateUserTimestamp( int time = 0 );

  // D-Bus slots:
  Q_SCRIPTABLE void reparseConfiguration();
  Q_SCRIPTABLE void quit();

protected:
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication(const KComponentData &cData);

#ifdef Q_WS_X11
  /**
   * @internal Used by KUniqueApplication
   */
  KApplication(Display *display, Qt::HANDLE visual, Qt::HANDLE colormap,
          const KComponentData &cData);

  /**
   * Used to catch X11 events
   */
  bool x11EventFilter( XEvent * );
#endif

  /// Current application object.
  static KApplication *KApp;

private:
  KApplication(const KApplication&);
  KApplication& operator=(const KApplication&);

  friend class KApplicationPrivate;
  KApplicationPrivate* const d;

  Q_PRIVATE_SLOT(d, void _k_x11FilterDestroyed())
  Q_PRIVATE_SLOT(d, void _k_checkAppStartedSlot())
  Q_PRIVATE_SLOT(d, void _k_KToolInvocation_hook(QStringList&, QByteArray&))
  Q_PRIVATE_SLOT(d, void _k_disableAutorestartSlot())
};

#endif

