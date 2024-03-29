/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *            (C) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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
 **/

#ifndef KDED_H
#define KDED_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusServiceWatcher>

#include <ksycoca.h>
#include <ksycocatype.h>
#include <kdedmodule.h>
#include <kservice.h>

#include <QDBusServiceWatcher>

class KDirWatch;

// Apps get read-only access
class Kded : public QObject
{
  Q_OBJECT
public:
   Kded(QObject *parent);
   virtual ~Kded();

   static Kded *self() { return _self;}
   static void messageFilter(const QDBusMessage &);

   void noDemandLoad(const QString &obj); // Don't load obj on demand

   KDEDModule *loadModule(const QString &obj, bool onDemand);
   KDEDModule *loadModule(const KService::Ptr& service, bool onDemand);
   QStringList loadedModules();
   bool unloadModule(const QString &obj);
   //bool isWindowRegistered(qlonglong windowId) const;
   /**
    * Applications can register/unregister their windows with kded modules.
    */
   //@{
   /**
    * Register a window with KDED
    */
   void registerWindowId(qlonglong windowId, const QString &sender);
   /**
    * Unregister a window previously registered with KDED
    */
   void unregisterWindowId(qlonglong windowId, const QString &sender);
   //@}
   void loadSecondPhase();

   //@{
   /**
    * Check if a module should be loaded on startup.
    *
    * @param module the name of the desktop file for the module, without the .desktop extension
    * @return @c true if the module will be loaded at startup, @c false otherwise
    */
   bool isModuleAutoloaded(const QString &module) const;
   /**
    * Check if a module should be loaded on startup.
    *
    * @param module a service description for the module
    * @return @c true if the module will be loaded at startup, @c false otherwise
    */
   bool isModuleAutoloaded(const KService::Ptr &module) const;
   //@}

   //@{
   /**
    * Check if a module should be loaded on demand
    *
    * @param module the name of the desktop file for the module, without the .desktop extension
    * @return @c true if the module will be loaded when its D-Bus interface
    *         is requested, @c false otherwise
    */
   bool isModuleLoadedOnDemand(const QString &module) const;
   /**
    * Check if a module should be loaded on demand
    *
    * @param module a service description for the module
    * @return @c true if the module will be loaded when its D-Bus interface
    *         is requested, @c false otherwise
    */
   bool isModuleLoadedOnDemand(const KService::Ptr &module) const;
   //@}

   /**
    * Configure whether a module should be loaded on startup
    *
    * If a module is set to be auto-loaded, it will be loaded at the start of a KDE
    * session.  Depending on the phase it is set to load in, it may also be loaded
    * when the first KDE application is run outside of a KDE session.
    *
    * @param module the name of the desktop file for the module, without the .desktop extension
    * @param autoload if @c true, the module will be loaded at startup,
    *                 otherwise it will not
    */
   void setModuleAutoloading(const QString &module, bool autoload);


public Q_SLOTS:
   /**
    * Loads / unloads modules according to config
    */
   void initModules();

   /**
    * Recreate the database file
    */
   void recreate();

   /**
    * Collect all directories to watch
    */
   void updateDirWatch();

   /**
    * Update directories to watch
    */
   void updateResourceList();

   /**
    * An application unregistered itself from DBus
    */
   void slotApplicationRemoved(const QString&);

   /**
    * A KDEDModule is about to get destroyed.
    */
   void slotKDEDModuleRemoved(KDEDModule *);

private Q_SLOTS:

   /**
    * @internal Triggers rebuilding
    */
   void update(const QString& dir);

   /**
    * @internal Executes kdontchangethehostname when hostname changes
    */
   void checkHostname();

private:
   /**
    * Pointer to the dirwatch class which tells us, when some directories
    * changed.
    */
   KDirWatch* m_pDirWatch;

   /**
    * When a desktop file is updated, a timer is started (5 sec)
    * before rebuilding the binary - so that multiple updates result
    * in only one rebuilding.
    */
   QTimer* m_pTimer;

   /**
    * Timer for periodic hostname checking.
    */
   QTimer* m_hTimer;
   /**
    * The current hostname.
    */
   QByteArray m_hostname;

   QHash<QString,KDEDModule *> m_modules;
    //QHash<QString,QLibrary *> m_libs;
   QHash<QString,QObject *> m_dontLoad;

   //window id tracking, with a QDBusServiceWatcher to remove them as needed
   QDBusServiceWatcher *m_serviceWatcher;
   QHash<QString,QList<qlonglong> > m_windowIdList;
   QSet<long> m_globalWindowIdList;

   QStringList m_allResourceDirs;

   static Kded *_self;
};

class KBuildsycocaAdaptor: public QDBusAbstractAdaptor
{
   Q_OBJECT
   Q_CLASSINFO("D-Bus Interface", "org.kde.kbuildsycoca")
public:
   KBuildsycocaAdaptor(QObject *parent);

public Q_SLOTS:
   void recreate();
};

#endif
