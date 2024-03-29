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

#include "kpasswdstoreimpl.h"
#include "kstandarddirs.h"
#include "ksettings.h"
#include "kpassworddialog.h"
#include "knewpassworddialog.h"
#include "kmessagebox.h"
#include "klocale.h"
#include "kdebug.h"

#include <QCryptographicHash>

#if defined(HAVE_OPENSSL)
#  include <openssl/evp.h>
#  include <openssl/err.h>
#  include <openssl/opensslv.h>
#endif

static const int kpasswdstore_buffsize = 1024;
static const int kpasswdstore_passretries = 3;
static const qint64 kpasswdstore_passtimeout = 2; // minutes

static inline QWidget* widgetForWindowID(const qlonglong windowid)
{
    return QWidget::find(windowid);
}

static inline QByteArray hashForBytes(const QByteArray &bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::KAT).toHex();
}

static inline QByteArray genBytes(const QByteArray &data, const int length)
{
    const QByteArray result = data + hashForBytes(data);
    Q_ASSERT(result.size() >= length);
    return result.mid(0, length);
}

KPasswdStoreImpl::KPasswdStoreImpl(const QString &id)
    : m_retries(kpasswdstore_passretries),
    m_timeout(kpasswdstore_passtimeout * 60000),
    m_cacheonly(false),
    m_storeid(id),
    m_passwdstore(KStandardDirs::locateLocal("data", "kpasswdstore.ini"))
#if defined(HAVE_OPENSSL)
    , m_opensslkeylen(0),
    m_opensslivlen(0),
    m_opensslblocklen(0)
#endif
{
#if defined(HAVE_OPENSSL)
    // deprecated since v1.1.1
#if OPENSSL_VERSION_NUMBER < 0x10101000L
    ERR_load_ERR_strings();
#endif
    EVP_add_cipher(EVP_aes_256_cbc());

    m_opensslkeylen = EVP_CIPHER_key_length(EVP_aes_256_cbc());
    m_opensslivlen = EVP_CIPHER_iv_length(EVP_aes_256_cbc());
    m_opensslblocklen = EVP_CIPHER_block_size(EVP_aes_256_cbc());
#endif

    KSettings ksettings("kpasswdstorerc", KSettings::SimpleConfig);
    m_retries = ksettings.integer("KPasswdStore/Retries", kpasswdstore_passretries);
    m_timeout = (ksettings.integer("KPasswdStore/Timeout", kpasswdstore_passtimeout) * 60000);
}

KPasswdStoreImpl::~KPasswdStoreImpl()
{
}

QString KPasswdStoreImpl::storeID() const
{
    return m_storeid;
}

bool KPasswdStoreImpl::openStore(const qlonglong windowid)
{
    if (m_cacheonly) {
        return false;
    }

    bool cancel = false;
    quint8 retry = m_retries;
    while (retry > 0 && !ensurePasswd(windowid, retry < m_retries, &cancel)) {
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

bool KPasswdStoreImpl::closeStore()
{
    clearPasswd();
    return true;
}

void KPasswdStoreImpl::setCacheOnly(const bool cacheonly)
{
    m_cacheonly = cacheonly;
    m_cachemap.clear();
}

bool KPasswdStoreImpl::cacheOnly() const
{
    return m_cacheonly;
}

QString KPasswdStoreImpl::getPasswd(const QByteArray &key, const qlonglong windowid)
{
    if (m_cacheonly) {
        return m_cachemap.value(key, QString());
    }

    if (!openStore(windowid)) {
        return QString();
    }

    bool ok = false;
    KSettings ksettings(m_passwdstore, KSettings::SimpleConfig);
    QString storekey = m_storeid;
    storekey.append(QLatin1Char('/'));
    storekey.append(QString::fromLatin1(key.constData(), key.size()));
    const QString passwd = ksettings.string(storekey);
    if (passwd.isEmpty()) {
        return QString();
    }
    return decryptPasswd(passwd, &ok);
}

bool KPasswdStoreImpl::storePasswd(const QByteArray &key, const QString &passwd, const qlonglong windowid)
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
    KSettings ksettings(m_passwdstore, KSettings::SimpleConfig);
    QString storekey = m_storeid;
    storekey.append(QLatin1Char('/'));
    storekey.append(QString::fromLatin1(key.constData(), key.size()));
    ksettings.setString(storekey, encryptPasswd(passwd, &ok));
    return ok;
}

bool KPasswdStoreImpl::ensurePasswd(const qlonglong windowid, const bool showerror, bool *cancel)
{
    Q_ASSERT(!cacheonly);

#if defined(HAVE_OPENSSL)
    if (!m_passwd.isEmpty() && m_passwdtimer.elapsed() >= m_timeout) {
        m_passwd.clear();
    }
    m_passwdtimer.restart();

    if (m_passwd.isEmpty()) {
        QByteArray kpasswddialogpass;
        // the only reason to encrypt and decrypt passwords is to obscure them
        // for the naked eye, if one can overwrite, delete or otherwise alter
        // the password store then there are more possibilities for havoc
        KSettings ksettings(m_passwdstore, KSettings::SimpleConfig);
        QString storekey = QString::fromLatin1("KPasswdStore/");
        storekey.append(m_storeid);
        const QString storepasswdhash = ksettings.string(storekey);
        if (storepasswdhash.isEmpty()) {
            KNewPasswordDialog knewpasswddialog(widgetForWindowID(windowid));
            knewpasswddialog.setPrompt(i18n("Enter a password for <b>%1</b> password storage", m_storeid));
            knewpasswddialog.setAllowEmptyPasswords(false);
            knewpasswddialog.setMaximumPasswordLength(m_opensslkeylen);
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
        m_passwd = genBytes(kpasswddialogpass, m_opensslkeylen);
        if (m_passwd.isEmpty()) {
            kWarning() << "Password bytes is empty";
            clearPasswd();
            return false;
        }
        const QByteArray passhash = hashForBytes(m_passwd);
        m_passwdiv = genBytes(passhash, m_opensslivlen);
        if (m_passwdiv.isEmpty()) {
            kWarning() << "Password initialization vector is empty";
            clearPasswd();
            return false;
        }

        if (storepasswdhash.isEmpty()) {
            ksettings.setString(storekey, passhash);
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

bool KPasswdStoreImpl::hasPasswd() const
{
#if defined(HAVE_OPENSSL)
    return !m_passwd.isEmpty();
#else
    return false;
#endif
}

void KPasswdStoreImpl::clearPasswd()
{
#if defined(HAVE_OPENSSL)
    m_passwd.clear();
    m_passwdiv.clear();
#endif
}

QString KPasswdStoreImpl::encryptPasswd(const QString &passwd, bool *ok) const
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

    Q_ASSERT(EVP_CIPHER_CTX_key_length(opensslctx) == m_opensslkeylen);
    Q_ASSERT(EVP_CIPHER_CTX_iv_length(opensslctx) == m_opensslivlen);

    const QByteArray passwdbytes = passwd.toUtf8();
    const int opensslbuffersize = (kpasswdstore_buffsize * m_opensslblocklen);
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

QString KPasswdStoreImpl::decryptPasswd(const QString &passwd, bool *ok) const
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

    Q_ASSERT(EVP_CIPHER_CTX_key_length(opensslctx) == m_opensslkeylen);
    Q_ASSERT(EVP_CIPHER_CTX_iv_length(opensslctx) == m_opensslivlen);

    const QByteArray passwdbytes = QByteArray::fromHex(passwd.toLatin1());
    const int opensslbuffersize = (kpasswdstore_buffsize * m_opensslblocklen);
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
    const QString result = QString::fromUtf8(decrypted.constData(), decrypted.size());
    *ok = !result.isEmpty();
    return result;
#endif
}
