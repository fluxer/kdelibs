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

#include "kded_kpasswdstore.h"
#include "kpluginfactory.h"
#include "kstandarddirs.h"
#include "kconfiggroup.h"
#include "kpassworddialog.h"
#include "knewpassworddialog.h"
#include "kmessagebox.h"
#include "klocale.h"
#include "kdebug.h"

#include <QElapsedTimer>
#include <QCryptographicHash>

#if defined(HAVE_OPENSSL)
#  include <openssl/evp.h>
#  include <openssl/err.h>
#endif

static const int kpasswdstore_buffsize = 1024;
static const int kpasswdstore_passretries = 3;
static const qint64 kpasswdstore_passtimeout = 2 * 60000;

// EVP_CIPHER_CTX_key_length() and EVP_CIPHER_CTX_iv_length() cannot be called
// prior to EVP_EncryptInit() and EVP_DecryptInit() so hardcoding these
static const int kpasswdstore_keylen = 32;
static const int kpasswdstore_ivlen = 16;

static inline QWidget* widgetForWindowID(const qlonglong windowid)
{
    return QWidget::find(windowid);
}

static inline QByteArray hashForBytes(const QByteArray &bytes)
{
#if QT_VERSION >= 0x041200
    return QCryptographicHash::hash(bytes, QCryptographicHash::KAT).toHex();
#else
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
#endif
}

class KPasswdStorePrivate
{
public:
    KPasswdStorePrivate(const QString &id);
    ~KPasswdStorePrivate();

    QString storeID() const;

    bool openStore(const qlonglong windowid);
    bool closeStore();

    void setCacheOnly(const bool cacheonly);
    bool cacheOnly() const;

    QString getPasswd(const QByteArray &key, const qlonglong windowid);
    bool storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid);

private:
    bool ensurePasswd(const qlonglong windowid, const bool showerror, bool *cancel);
    bool hasPasswd() const;
    void clearPasswd();

    QString decryptPasswd(const QString &passwd, bool *ok);
    QString encryptPasswd(const QString &passwd, bool *ok);

    bool m_cacheonly;
    QString m_storeid;
    QString m_passwdstore;
    QMap<QByteArray, QString> m_cachemap;

#if defined(HAVE_OPENSSL)
    static QByteArray genBytes(const QByteArray &data, const int length);

    QByteArray m_passwd;
    QByteArray m_passwdiv;
    QElapsedTimer m_passwdtimer;
#endif
};

KPasswdStorePrivate::KPasswdStorePrivate(const QString &id)
    : m_cacheonly(false),
    m_storeid(id),
    m_passwdstore(KStandardDirs::locateLocal("data", "kpasswdstore.ini"))
{
#if defined(HAVE_OPENSSL)
    ERR_load_ERR_strings();
    EVP_add_cipher(EVP_aes_256_cbc());
#endif
}

KPasswdStorePrivate::~KPasswdStorePrivate()
{
}

QString KPasswdStorePrivate::storeID() const
{
    return m_storeid;
}

bool KPasswdStorePrivate::openStore(const qlonglong windowid)
{
    if (m_cacheonly) {
        return false;
    }

    bool cancel = false;
    int retry = kpasswdstore_passretries;
    while (retry > 0 && !ensurePasswd(windowid, retry < kpasswdstore_passretries, &cancel)) {
        retry--;
        if (cancel) {
            break;
        }
    }
    if (!hasPasswd()) {
        KMessageBox::error(widgetForWindowID(windowid), i18n("The storage could not be open, no passwords will be permanently stored"));
        setCacheOnly(true);
        return false;
    }
    return true;
}

bool KPasswdStorePrivate::closeStore()
{
    clearPasswd();
    return true;
}

void KPasswdStorePrivate::setCacheOnly(const bool cacheonly)
{
    m_cacheonly = cacheonly;
    m_cachemap.clear();
}

bool KPasswdStorePrivate::cacheOnly() const
{
    return m_cacheonly;
}

QString KPasswdStorePrivate::getPasswd(const QByteArray &key, const qlonglong windowid)
{
    if (m_cacheonly) {
        return m_cachemap.value(key, QString());
    }

    if (!openStore(windowid)) {
        return QString();
    }

    bool ok = false;
    KConfig kconfig(m_passwdstore);
    const QString passwd = kconfig.group(m_storeid).readEntry(key.constData(), QString());
    if (passwd.isEmpty()) {
        return QString();
    }
    return decryptPasswd(passwd, &ok);
}

bool KPasswdStorePrivate::storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid)
{
    if (m_cacheonly) {
        m_cachemap.insert(key, passwd);
        return true;
    }

    if (!openStore(windowid)) {
        return storePasswd(key, passwd, windowid);
    }

    if (passwd.isEmpty()) {
        return false;
    }

    bool ok = false;
    KConfig kconfig(m_passwdstore);
    KConfigGroup kconfiggroup = kconfig.group(m_storeid);
    kconfiggroup.writeEntry(key.constData(), encryptPasswd(passwd, &ok));
    kconfiggroup.sync();
    return ok;
}

bool KPasswdStorePrivate::ensurePasswd(const qlonglong windowid, const bool showerror, bool *cancel)
{
    Q_ASSERT(!cacheonly);

#if defined(HAVE_OPENSSL)
    if (!m_passwd.isEmpty() && m_passwdtimer.elapsed() >= kpasswdstore_passtimeout) {
        m_passwd.clear();
    }
    m_passwdtimer.restart();

    if (m_passwd.isEmpty()) {
        QByteArray kpasswddialogpass;
        // the only reason to encrypt and decrypt passwords is to obscure them
        // for the naked eye, if one can overwrite, delete or otherwise alter
        // the password store then there are more possibilities for havoc
        KConfig kconfig(m_passwdstore);
        KConfigGroup kconfiggroup = kconfig.group("KPasswdStore");
        const QString storepasswdhash = kconfiggroup.readEntry(m_storeid, QString());
        if (storepasswdhash.isEmpty()) {
            KNewPasswordDialog knewpasswddialog(widgetForWindowID(windowid));
            knewpasswddialog.setPrompt(i18n("Enter a password for <b>%1</b> password storage", m_storeid));
            knewpasswddialog.setAllowEmptyPasswords(false);
            if (knewpasswddialog.exec() != KNewPasswordDialog::Accepted) {
                kDebug() << "New password dialog not accepted";
                clearPasswd();
                *cancel = true;
                return false;
            }
            kpasswddialogpass = knewpasswddialog.password().toUtf8();
        } else {
            KPasswordDialog kpasswddialog(widgetForWindowID(windowid));
            kpasswddialog.setPrompt(i18n("Enter a password for <b>%1</b> password storage", m_storeid));
            if (showerror) {
                kpasswddialog.showErrorMessage(i18n("Incorrect password"));
            }
            if (kpasswddialog.exec() != KPasswordDialog::Accepted) {
                kDebug() << "Password dialog not accepted";
                clearPasswd();
                *cancel = true;
                return false;
            }
            kpasswddialogpass = kpasswddialog.password().toUtf8();
        }

        if (kpasswddialogpass.isEmpty()) {
            kWarning() << "Password is empty";
            clearPasswd();
            return false;
        }
        m_passwd = KPasswdStorePrivate::genBytes(kpasswddialogpass, kpasswdstore_keylen);
        if (m_passwd.isEmpty()) {
            kWarning() << "Password bytes is empty";
            clearPasswd();
            return false;
        }
        const QByteArray passhash = hashForBytes(m_passwd);
        m_passwdiv = KPasswdStorePrivate::genBytes(passhash, kpasswdstore_ivlen);
        if (m_passwdiv.isEmpty()) {
            kWarning() << "Password initialization vector is empty";
            clearPasswd();
            return false;
        }

        if (storepasswdhash.isEmpty()) {
            kconfiggroup.writeEntry(m_storeid, passhash);
            return true;
        }
        if (passhash != storepasswdhash) {
            kWarning() << "Password hash does not match";
            clearPasswd();
            return false;
        }
        return true;
    }
    return hasPasswd();
#else
    // not used
    return true;
#endif
}

bool KPasswdStorePrivate::hasPasswd() const
{
    return !m_passwd.isEmpty();
}

void KPasswdStorePrivate::clearPasswd()
{
    m_passwd.clear();
    m_passwdiv.clear();
}

QString KPasswdStorePrivate::decryptPasswd(const QString &passwd, bool *ok)
{
#if defined(HAVE_OPENSSL)
    EVP_CIPHER_CTX *opensslctx = EVP_CIPHER_CTX_new();
    if (Q_UNLIKELY(!opensslctx)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        return QString();
    }

    int opensslresult = EVP_DecryptInit(
        opensslctx, EVP_aes_256_cbc(),
        reinterpret_cast<const uchar*>(m_passwd.constData()),
        reinterpret_cast<const uchar*>(m_passwdiv.constData())
    );
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    // qDebug() << Q_FUNC_INFO << EVP_CIPHER_CTX_key_length(opensslctx);
    // qDebug() << Q_FUNC_INFO << EVP_CIPHER_CTX_iv_length(opensslctx);
    Q_ASSERT(EVP_CIPHER_CTX_key_length(opensslctx) == kpasswdstore_keylen);
    Q_ASSERT(EVP_CIPHER_CTX_iv_length(opensslctx) == kpasswdstore_ivlen);

    const QByteArray passwdbytes = QByteArray::fromHex(passwd.toLatin1());
    const int opensslbuffersize = (kpasswdstore_buffsize * EVP_CIPHER_CTX_block_size(opensslctx));
    uchar opensslbuffer[opensslbuffersize];
    ::memset(opensslbuffer, 0, opensslbuffersize * sizeof(uchar));
    int opensslbufferpos = 0;
    int openssloutputsize = 0;
    opensslresult = EVP_DecryptUpdate(
        opensslctx,
        opensslbuffer, &opensslbufferpos,
        reinterpret_cast<const uchar*>(passwdbytes.constData()), passwdbytes.size()
    );
    openssloutputsize = opensslbufferpos;
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    opensslresult = EVP_DecryptFinal(
        opensslctx,
        opensslbuffer + opensslbufferpos, &opensslbufferpos
    );
    openssloutputsize += opensslbufferpos;
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    const QString result = QString::fromUtf8(reinterpret_cast<char*>(opensslbuffer), openssloutputsize);
    EVP_CIPHER_CTX_free(opensslctx);
    *ok = !result.isEmpty();
    return result;
#else
    const QByteArray decrypted = QByteArray::fromBase64(passwd.toUtf8());
    const QString result = QString::fromLatin1(decrypted.constData(), decrypted.size());
    *ok = !result.isEmpty();
    return result;
#endif
}

QString KPasswdStorePrivate::encryptPasswd(const QString &passwd, bool *ok)
{
#if defined(HAVE_OPENSSL)
    EVP_CIPHER_CTX *opensslctx = EVP_CIPHER_CTX_new();
    if (Q_UNLIKELY(!opensslctx)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        return QString();
    }

    int opensslresult = EVP_EncryptInit(
        opensslctx, EVP_aes_256_cbc(),
        reinterpret_cast<const uchar*>(m_passwd.constData()),
        reinterpret_cast<const uchar*>(m_passwdiv.constData())
    );
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    Q_ASSERT(EVP_CIPHER_CTX_key_length(opensslctx) == kpasswdstore_keylen);
    Q_ASSERT(EVP_CIPHER_CTX_iv_length(opensslctx) == kpasswdstore_ivlen);

    const QByteArray passwdbytes = passwd.toUtf8();
    const int opensslbuffersize = (kpasswdstore_buffsize * EVP_CIPHER_CTX_block_size(opensslctx));
    uchar opensslbuffer[opensslbuffersize];
    ::memset(opensslbuffer, 0, opensslbuffersize * sizeof(uchar));
    int opensslbufferpos = 0;
    int openssloutputsize = 0;
    opensslresult = EVP_EncryptUpdate(
        opensslctx,
        opensslbuffer, &opensslbufferpos,
        reinterpret_cast<const uchar*>(passwdbytes.constData()), passwdbytes.size()
    );
    openssloutputsize = opensslbufferpos;
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    opensslresult = EVP_EncryptFinal(
        opensslctx,
        opensslbuffer + opensslbufferpos, &opensslbufferpos
    );
    openssloutputsize += opensslbufferpos;
    if (Q_UNLIKELY(opensslresult != 1)) {
        kWarning() << ERR_error_string(ERR_get_error(), NULL);
        EVP_CIPHER_CTX_free(opensslctx);
        return QString();
    }

    const QString result = QString::fromLatin1(QByteArray(reinterpret_cast<char*>(opensslbuffer), openssloutputsize).toHex());
    EVP_CIPHER_CTX_free(opensslctx);
    *ok = !result.isEmpty();
    return result;
#else
    const QString result = passwd.toUtf8().toBase64();
    *ok = !result.isEmpty();
    return result;
#endif
}

#if defined(HAVE_OPENSSL)
QByteArray KPasswdStorePrivate::genBytes(const QByteArray &data, const int length)
{
    const QByteArray result = data + hashForBytes(data);
    Q_ASSERT(result.size() >= length);
    return result.mid(0, length);
}
#endif


K_PLUGIN_FACTORY(KPasswdStoreModuleFactory, registerPlugin<KPasswdStoreModule>();)
K_EXPORT_PLUGIN(KPasswdStoreModuleFactory("kpasswdstore"))

KPasswdStoreModule::KPasswdStoreModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
}

KPasswdStoreModule::~KPasswdStoreModule()
{
    qDeleteAll(m_stores);
}

bool KPasswdStoreModule::openStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            return store->openStore(windowid);
        }
        it++;
    }
    KPasswdStorePrivate *store = new KPasswdStorePrivate(storeid);
    m_stores.insert(cookie, store);
    return store->openStore(windowid);
}

bool KPasswdStoreModule::closeStore(const QByteArray &cookie, const QString &storeid, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            return store->closeStore();
        }
        it++;
    }
    return false;
}

void KPasswdStoreModule::setCacheOnly(const QByteArray &cookie, const QString &storeid, const bool cacheonly)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            store->setCacheOnly(cacheonly);
        }
        it++;
    }
}

bool KPasswdStoreModule::cacheOnly(const QByteArray &cookie, const QString &storeid) const
{
    KPasswdStoreMap::const_iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        const KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            return store->cacheOnly();
        }
        it++;
    }
    return false;
}

QString KPasswdStoreModule::getPasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            return store->getPasswd(key, windowid);
        }
        it++;
    }
    KPasswdStorePrivate *store = new KPasswdStorePrivate(storeid);
    m_stores.insert(cookie, store);
    return store->getPasswd(key, windowid);
}

bool KPasswdStoreModule::storePasswd(const QByteArray &cookie, const QString &storeid, const QByteArray &key, const QString &passwd, const qlonglong windowid)
{
    KPasswdStoreMap::iterator it = m_stores.begin();
    while (it != m_stores.end()) {
        if (it.key() != cookie) {
            it++;
            continue;
        }
        KPasswdStorePrivate *store = it.value();
        if (store->storeID() == storeid) {
            return store->storePasswd(key, passwd, windowid);
        }
        it++;
    }
    KPasswdStorePrivate *store = new KPasswdStorePrivate(storeid);
    m_stores.insert(cookie, store);
    return store->storePasswd(key, passwd, windowid);
}

#include "moc_kded_kpasswdstore.cpp"
