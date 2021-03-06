/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
 * Copyright (C) 2008 Michael Leupold <lemma@confuego.org>
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kwallet.h"
#include "config-kwallet.h"

#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <QtDBus/QtDBus>
#include <QtCore/qtimer.h>
#include <ktoolinvocation.h>

#include <assert.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <ksharedconfig.h>
#include <kwindowsystem.h>


#include "kwallet_interface.h"

typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

namespace KWallet
{

class KWalletDLauncher
{
public:
    KWalletDLauncher();
    ~KWalletDLauncher();
    org::kde::KWallet &getInterface();

    org::kde::KWallet *m_wallet;
    KConfigGroup m_cgroup;
};

K_GLOBAL_STATIC(KWalletDLauncher, walletLauncher)

static QString appid()
{
    if (KGlobal::hasMainComponent()) {
        KComponentData cData = KGlobal::mainComponent();
        if (cData.isValid()) {
            const KAboutData* aboutData = cData.aboutData();
            if (aboutData) {
                return aboutData->programName();
            }
            return cData.componentName();
        }
    }
    return qApp->applicationName();
}

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
        qDBusRegisterMetaType<StringByteArrayMap>();
        registered = true;
    }
}

const QString Wallet::LocalWallet() {
    // NOTE: This method stays unchanged for KSecretsService
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));
    if (!cfg.readEntry("Use One Wallet", true)) {
        QString tmp = cfg.readEntry("Local Wallet", "localwallet");
        if (tmp.isEmpty()) {
            return "localwallet";
        }
        return tmp;
    }

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return "kdewallet";
    }
    return tmp;
}

const QString Wallet::NetworkWallet() {
    // NOTE: This method stays unchanged for KSecretsService
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return "kdewallet";
    }
    return tmp;
}

const QString Wallet::PasswordFolder() {
    return "Passwords";
}

const QString Wallet::FormDataFolder() {
    return "Form Data";
}

class Wallet::WalletPrivate
{
public:
    WalletPrivate(Wallet *wallet, int h, const QString &n)
     : q(wallet), name(n), handle(h)
    {}

    void walletServiceUnregistered();


    Wallet *q;
    QString name;
    QString folder;
    int handle;
    int transactionId;
};


static const char s_kwalletdServiceName[] = "org.kde.kwalletd";

Wallet::Wallet(int handle, const QString& name)
    : QObject(0L), d(new WalletPrivate(this, handle, name))
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QString::fromLatin1(s_kwalletdServiceName), QDBusConnection::sessionBus(),
                                                        QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(walletServiceUnregistered()));

    connect(&walletLauncher->getInterface(), SIGNAL(walletClosed(int)), SLOT(slotWalletClosed(int)));
    connect(&walletLauncher->getInterface(), SIGNAL(folderListUpdated(QString)), SLOT(slotFolderListUpdated(QString)));
    connect(&walletLauncher->getInterface(), SIGNAL(folderUpdated(QString,QString)), SLOT(slotFolderUpdated(QString,QString)));
    connect(&walletLauncher->getInterface(), SIGNAL(applicationDisconnected(QString,QString)), SLOT(slotApplicationDisconnected(QString,QString)));

    // Verify that the wallet is still open
    if (d->handle != -1) {
        QDBusReply<bool> r = walletLauncher->getInterface().isOpen(d->handle);
        if (r.isValid() && !r) {
            d->handle = -1;
            d->name.clear();
        }
    }
}


Wallet::~Wallet() {
    if (d->handle != -1) {
        if (!walletLauncher.isDestroyed()) {
            walletLauncher->getInterface().close(d->handle, false, appid());
        } else {
            kDebug(285) << "Problem with static destruction sequence."
                        "Destroy any static Wallet before the event-loop exits.";
        }
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
    }
    delete d;
}


QStringList Wallet::walletList() {
    QStringList result;
    QDBusReply<QStringList> r = walletLauncher->getInterface().wallets();

    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
    } else {
        result = r;
    }
    return result;
}


void Wallet::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::changePassword().";

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();
    walletLauncher->getInterface().changePassword(name, (qlonglong)w, appid());
}


bool Wallet::isEnabled() {
    QDBusReply<bool> r = walletLauncher->getInterface().isEnabled();
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}


bool Wallet::isOpen(const QString& name) {
    QDBusReply<bool> r = walletLauncher->getInterface().isOpen(name);
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}

int Wallet::closeWallet(const QString& name, bool force) {
    QDBusReply<int> r = walletLauncher->getInterface().close(name, force);
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return -1;
    } else {
        return r;
    }
}


int Wallet::deleteWallet(const QString& name) {
    QDBusReply<int> r = walletLauncher->getInterface().deleteWallet(name);
    if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return -1;
    } else {
        return r;
    }
}

Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::openWallet().";

        Wallet *wallet = new Wallet(-1, name);

        // connect the daemon's opened signal to the slot filtering the
        // signals we need
        connect(&walletLauncher->getInterface(), SIGNAL(walletAsyncOpened(int,int)),
                wallet, SLOT(walletAsyncOpened(int,int)));

        // Make sure the password prompt window will be visible and activated
        KWindowSystem::allowExternalProcessWindowActivation();

        // do the call
        QDBusReply<int> r;
        if (ot == Synchronous) {
            r = walletLauncher->getInterface().open(name, (qlonglong)w, appid());
            // after this call, r would contain a transaction id >0 if OK or -1 if NOK
            // if OK, the slot walletAsyncOpened should have been received, but the transaction id
            // will not match. We'll get that handle from the reply - see below
        }
        else if (ot == Asynchronous) {
            r = walletLauncher->getInterface().openAsync(name, (qlonglong)w, appid(), true);
        } else if (ot == Path) {
            r = walletLauncher->getInterface().openPathAsync(name, (qlonglong)w, appid(), true);
        } else {
            delete wallet;
            return 0;
        }
        // error communicating with the daemon (maybe not running)
        if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            delete wallet;
            return 0;
        }
        wallet->d->transactionId = r.value();

        if (ot == Synchronous || ot == Path) {
            // check for an immediate error
            if (wallet->d->transactionId < 0) {
                delete wallet;
                wallet = 0;
            } else {
                wallet->d->handle = r.value();
            }
        } else if (ot == Asynchronous) {
            if (wallet->d->transactionId < 0) {
                QTimer::singleShot(0, wallet, SLOT(emitWalletAsyncOpenError()));
                // client code is responsible for deleting the wallet
            }
        }

        return wallet;
}

void Wallet::slotCollectionDeleted()
{
    d->folder.clear();
    d->name.clear();
    emit walletClosed();
}

bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
        QDBusReply<bool> r = walletLauncher->getInterface().disconnectApplication(wallet, app);

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
}


QStringList Wallet::users(const QString& name) {
        QDBusReply<QStringList> r = walletLauncher->getInterface().users(name);
        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return QStringList();
        }
        else
            return r;
}


int Wallet::sync() {
        if (d->handle == -1) {
            return -1;
        }

        walletLauncher->getInterface().sync(d->handle, appid());
    return 0;
}


int Wallet::lockWallet() {
        if (d->handle == -1) {
            return -1;
        }

        QDBusReply<int> r = walletLauncher->getInterface().close(d->handle, true, appid());
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        if (r.isValid()) {
            return r;
        }
        else {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return -1;
        }
}


const QString& Wallet::walletName() const {
    return d->name;
}


bool Wallet::isOpen() const {
    return d->handle != -1;
}


void Wallet::requestChangePassword(WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";

        if (d->handle == -1) {
            return;
        }

        // Make sure the password prompt window will be visible and activated
        KWindowSystem::allowExternalProcessWindowActivation();

        walletLauncher->getInterface().changePassword(d->name, (qlonglong)w, appid());
}


void Wallet::slotWalletClosed(int handle) {
    if (d->handle == handle) {
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        emit walletClosed();
    }
}


QStringList Wallet::folderList() {
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().folderList(d->handle, appid());
    if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return QStringList();
    } else {
        return r;
    }
}


QStringList Wallet::entryList() {
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().entryList(d->handle, d->folder, appid());
    if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return QStringList();
    } else {
        return r;
    }
}


bool Wallet::hasFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().hasFolder(d->handle, f, appid());
    if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
    } else {
        return r;
    }
}


bool Wallet::createFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = walletLauncher->getInterface().createFolder(d->handle, f, appid());

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }

    return true; // folder already exists
}


bool Wallet::setFolder(const QString& f) {
    bool rc = false;

        if (d->handle == -1) {
            return rc;
        }

        // Don't do this - the folder could have disappeared?
    #if 0
        if (f == d->folder) {
            return true;
        }
    #endif

        if (hasFolder(f)) {
            d->folder = f;
            rc = true;
        }

    return rc;
}


bool Wallet::removeFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().removeFolder(d->handle, f, appid());
    if (d->folder == f) {
        setFolder(QString());
    }

    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}


const QString& Wallet::currentFolder() const {
    return d->folder;
}


int Wallet::readEntry(const QString& key, QByteArray& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = walletLauncher->getInterface().readEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}


int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {

    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = walletLauncher->getInterface().readEntryList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        // convert <QString, QVariant> to <QString, QByteArray>
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toByteArray());
        }
    }

    return rc;
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().renameEntry(d->handle, d->folder, oldName, newName, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = walletLauncher->getInterface().readMap(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        QByteArray v = r;
        if (!v.isEmpty()) {
            QDataStream ds(&v, QIODevice::ReadOnly);
            ds >> value;
        }
    }

    return rc;
}


int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r =
        walletLauncher->getInterface().readMapList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            QByteArray mapData = it.value().toByteArray();
            if (!mapData.isEmpty()) {
                QDataStream ds(&mapData, QIODevice::ReadOnly);
                QMap<QString,QString> v;
                ds >> v;
                value.insert(it.key(), v);
            }
        }
    }

    return rc;
}


int Wallet::readPassword(const QString& key, QString& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QString> r = walletLauncher->getInterface().readPassword(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}


int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = walletLauncher->getInterface().readPasswordList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toString());
        }
    }

    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().writeEntry(d->handle, d->folder, key, value, int(entryType), appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().writeEntry(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    QDBusReply<int> r = walletLauncher->getInterface().writeMap(d->handle, d->folder, key, mapData, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writePassword(const QString& key, const QString& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().writePassword(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


bool Wallet::hasEntry(const QString& key) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().hasEntry(d->handle, d->folder, key, appid());
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}


int Wallet::removeEntry(const QString& key) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().removeEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


Wallet::EntryType Wallet::entryType(const QString& key) {
    int rc = 0;

    if (d->handle == -1) {
        return Wallet::Unknown;
    }

    QDBusReply<int> r = walletLauncher->getInterface().entryType(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return static_cast<EntryType>(rc);
}


void Wallet::WalletPrivate::walletServiceUnregistered()
{
    if (handle >= 0) {
        q->slotWalletClosed(handle);
    }
}

void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder) {
    if (d->name == wallet) {
        emit folderUpdated(folder);
    }
}


void Wallet::slotFolderListUpdated(const QString& wallet) {
    if (d->name == wallet) {
        emit folderListUpdated();
    }
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application) {
    if (d->handle >= 0
        && d->name == wallet
        && application == appid()) {
        slotWalletClosed(d->handle);
    }
}

void Wallet::walletAsyncOpened(int tId, int handle) {
    // ignore responses to calls other than ours
    if (d->transactionId != tId || d->handle != -1) {
        return;
    }

    // disconnect the async signal
    disconnect(this, SLOT(walletAsyncOpened(int,int)));

    d->handle = handle;
    emit walletOpened(handle > 0);
}

void Wallet::emitWalletAsyncOpenError() {
    emit walletOpened(false);
}

void Wallet::emitWalletOpened() {
    emit walletOpened(true);
}

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    QDBusReply<bool> r = walletLauncher->getInterface().folderDoesNotExist(wallet, folder);
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    QDBusReply<bool> r = walletLauncher->getInterface().keyDoesNotExist(wallet, folder, key);
    if (!r.isValid())
    {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}


KWalletDLauncher::KWalletDLauncher()
    : m_wallet(0),
    m_cgroup(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet"))
{
    m_wallet = new org::kde::KWallet(QString::fromLatin1(s_kwalletdServiceName), "/modules/kwalletd", QDBusConnection::sessionBus());
}

KWalletDLauncher::~KWalletDLauncher()
{
    delete m_wallet;
}

org::kde::KWallet &KWalletDLauncher::getInterface()
{
    Q_ASSERT(m_wallet != 0);

    // check if kwalletd is already running
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName)))
    {
        // not running! check if it is enabled.
        bool walletEnabled = m_cgroup.readEntry("Enabled", true);
        if (walletEnabled) {
            // wallet is enabled! try launching it
            QString error;
            int ret = KToolInvocation::startServiceByDesktopPath("kwalletd.desktop", QStringList(), &error);
            if (ret > 0)
            {
                kError(285) << "Couldn't start kwalletd: " << error << endl;
            }

            if
                (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName))) {
                kDebug(285) << "The kwalletd service is still not registered";
            } else {
                kDebug(285) << "The kwalletd service has been registered";
            }
        } else {
            kError(285) << "The kwalletd service has been disabled";
        }
    }

    return *m_wallet;
}

} // namespace KWallet

#include "moc_kwallet.cpp"
