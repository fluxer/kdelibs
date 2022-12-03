/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -p klauncher_iface -m ../kinit/org.kde.KLauncher.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech AS. All rights reserved.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

/*
 KDE5:
 This is a manual copy of an automatically generated file, and the output from dbusxml2cpp
 can change between dbusxml2cpp versions. This is currently no binary compatible
 with what dbusxml2cpp generates these days, so if something else autogenerates the interface,
 there will be crashes on systems with no symbol visibility (happened in ksmserver).
 Either dbusxml2cpp should be fixed or this should be removed from kdelibs.
*/

#ifndef KLAUNCHER_IFACE_H_84591156096727
#define KLAUNCHER_IFACE_H_84591156096727

#include <kdecore_export.h>

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QDBusAbstractInterface>
#include <QtDBus/QDBusReply>

/*
 * Proxy class for interface org.kde.KLauncher
 */
class KDECORE_EXPORT OrgKdeKLauncherInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.kde.KLauncher"; }

public:
    OrgKdeKLauncherInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgKdeKLauncherInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<void> autoStart()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("autoStart"), argumentList);
    }

    inline QDBusReply<void> autoStart(int phase)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(phase);
        return callWithArgumentList(QDBus::Block, QLatin1String("autoStart"), argumentList);
    }

    inline QDBusReply<void> exec_blind(const QString &name, const QStringList &arg_list)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(name) << qVariantFromValue(arg_list);
        return callWithArgumentList(QDBus::Block, QLatin1String("exec_blind"), argumentList);
    }

    inline QDBusReply<int> kdeinit_exec(const QString &app, const QStringList &args, const QStringList &env, const QString &startup_id, QString &dbusServiceName, QString &error, qint64 &pid)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(app) << qVariantFromValue(args) << qVariantFromValue(env) << qVariantFromValue(startup_id);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QLatin1String("kdeinit_exec"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 4) {
            dbusServiceName = qdbus_cast<QString>(reply.arguments().at(1));
            error = qdbus_cast<QString>(reply.arguments().at(2));
            pid = qdbus_cast<qint64>(reply.arguments().at(3));
        }
        return reply;
    }

    inline QDBusReply<int> kdeinit_exec_wait(const QString &app, const QStringList &args, const QStringList &env, const QString &startup_id, QString &dbusServiceName, QString &error, qint64 &pid)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(app) << qVariantFromValue(args) << qVariantFromValue(env) << qVariantFromValue(startup_id);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QLatin1String("kdeinit_exec_wait"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 4) {
            dbusServiceName = qdbus_cast<QString>(reply.arguments().at(1));
            error = qdbus_cast<QString>(reply.arguments().at(2));
            pid = qdbus_cast<qint64>(reply.arguments().at(3));
        }
        return reply;
    }

    inline QDBusReply<void> reparseConfiguration()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("reparseConfiguration"), argumentList);
    }

    inline QDBusReply<void> setLaunchEnv(const QString &name, const QString &value)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(name) << qVariantFromValue(value);
        return callWithArgumentList(QDBus::Block, QLatin1String("setLaunchEnv"), argumentList);
    }

    inline QDBusReply<int> start_service_by_desktop_name(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, QString &dbusServiceName, QString &error, qint64 &pid)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(serviceName) << qVariantFromValue(urls) << qVariantFromValue(envs) << qVariantFromValue(startup_id) << qVariantFromValue(blind);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QLatin1String("start_service_by_desktop_name"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 4) {
            dbusServiceName = qdbus_cast<QString>(reply.arguments().at(1));
            error = qdbus_cast<QString>(reply.arguments().at(2));
            pid = qdbus_cast<qint64>(reply.arguments().at(3));
        }
        return reply;
    }

    inline QDBusReply<int> start_service_by_desktop_path(const QString &serviceName, const QStringList &urls, const QStringList &envs, const QString &startup_id, bool blind, QString &dbusServiceName, QString &error, qint64 &pid)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(serviceName) << qVariantFromValue(urls) << qVariantFromValue(envs) << qVariantFromValue(startup_id) << qVariantFromValue(blind);
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QLatin1String("start_service_by_desktop_path"), argumentList);
        if (reply.type() == QDBusMessage::ReplyMessage && reply.arguments().count() == 4) {
            dbusServiceName = qdbus_cast<QString>(reply.arguments().at(1));
            error = qdbus_cast<QString>(reply.arguments().at(2));
            pid = qdbus_cast<qint64>(reply.arguments().at(3));
        }
        return reply;
    }

Q_SIGNALS: // SIGNALS
    void autoStart0Done();
    void autoStart1Done();
    void autoStart2Done();
};

namespace org {
  namespace kde {
    typedef ::OrgKdeKLauncherInterface KLauncher;
  }
}
#endif
