/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Andreas Hartmetz <ahartmetz@gmail.com>
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

#include "sslui.h"

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <ksslcertificatemanager.h>
#include <ksslinfodialog.h>
#include <QSslCipher>
#include <QHostAddress>

namespace KIO {
namespace SslUi {

// TODO: remove private data class
class KSslErrorUiData::Private
{
public:
    static const KSslErrorUiData::Private *get(const KSslErrorUiData *uiData)
    { return uiData->d; }

    QList<QSslCertificate> certificateChain;
    QList<QSslError> sslErrors;   // parallel list to certificateChain
    QString ip;
    QString host;
    QString sslProtocol;
    QString cipher;
    int usedBits;
    int bits;
};


KSslErrorUiData::KSslErrorUiData()
 : d(new Private())
{
    d->usedBits = 0;
    d->bits = 0;
}

KSslErrorUiData::KSslErrorUiData(const QSslSocket *socket)
 : d(new Private())
{
    d->certificateChain = socket->peerCertificateChain();

    d->sslErrors = socket->sslErrors();

    d->ip = socket->peerAddress().toString();
    d->host = socket->peerName();
    if (socket->isEncrypted()) {
        d->sslProtocol = socket->sessionCipher().protocolString();
    }
    d->cipher = socket->sessionCipher().name();
    d->usedBits = socket->sessionCipher().usedBits();
    d->bits = socket->sessionCipher().supportedBits();
}


KSslErrorUiData::KSslErrorUiData(const KSslErrorUiData &other)
 : d(new Private(*other.d))
{}

KSslErrorUiData::~KSslErrorUiData()
{
    delete d;
}

KSslErrorUiData &KSslErrorUiData::operator=(const KSslErrorUiData &other)
{
    *d = *other.d;
    return *this;
}


bool askIgnoreSslErrors(const QSslSocket *socket, RulesStorage storedRules)
{
    KSslErrorUiData uiData(socket);
    return askIgnoreSslErrors(uiData, storedRules);
}


bool askIgnoreSslErrors(const KSslErrorUiData &uiData, RulesStorage storedRules)
{
    const KSslErrorUiData::Private *ud = KSslErrorUiData::Private::get(&uiData);
    if (ud->sslErrors.isEmpty()) {
        return true;
    }

    QList<QSslError> fatalErrors = KSslCertificateManager::nonIgnorableErrors(ud->sslErrors);
    if (!fatalErrors.isEmpty()) {
        //TODO message "sorry, fatal error, you can't override it"
        return false;
    }
    if (ud->certificateChain.isEmpty()) {
        // SSL without certificates is quite useless and should never happen
        KMessageBox::sorry(0, i18n("The remote host did not send any SSL certificates.\n"
                                   "Aborting because the identity of the host cannot be established."));
        return false;
    }

    KSslCertificateManager *const cm = KSslCertificateManager::self();
    KSslCertificateRule rule(ud->certificateChain.first(), ud->host);
    if (storedRules & RecallRules) {
        rule = cm->rule(ud->certificateChain.first(), ud->host);
        // remove previously seen and acknowledged errors
        QList<QSslError> remainingErrors = rule.filterErrors(ud->sslErrors);
        if (remainingErrors.isEmpty()) {
            kDebug(7029) << "Error list empty after removing errors to be ignored. Continuing.";
            return true;
        }
    }

    //### We don't ask to permanently reject the certificate

    QString message = i18n("The server failed the authenticity check (%1).\n\n", ud->host);
    foreach (const QSslError &err, ud->sslErrors) {
        message.append(err.errorString());
        message.append('\n');
    }
    message = message.trimmed();

    int msgResult;
    do {
        msgResult = KMessageBox::warningYesNoCancel(0, message, i18n("Server Authentication"),
                                                    KGuiItem(i18n("&Details"), "help-about"),
                                                    KGuiItem(i18n("Co&ntinue"), "arrow-right"));
        if (msgResult == KMessageBox::Yes) {
            //Details was chosen - show the certificate and error details


            QList<QList<QSslError::SslError> > meh;    // parallel list to cert list :/

            foreach (const QSslCertificate &cert, ud->certificateChain) {
                QList<QSslError::SslError> errors;
                foreach(const QSslError &error, ud->sslErrors) {
                    if (error.certificate() == cert) {
                        // we keep only the error code enum here
                        errors.append(error.error());
                    }
                }
                meh.append(errors);
            }


            KSslInfoDialog *dialog = new KSslInfoDialog();
            dialog->setSslInfo(ud->certificateChain, ud->ip, ud->host, ud->sslProtocol,
                               ud->cipher, ud->usedBits, ud->bits, meh);
            dialog->exec();
        } else if (msgResult == KMessageBox::Cancel) {
            return false;
        }
        //fall through on KMessageBox::No
    } while (msgResult == KMessageBox::Yes);


    if (storedRules & StoreRules) {
        //Save the user's choice to ignore the SSL errors.

        msgResult = KMessageBox::warningYesNo(0,
                                i18n("Would you like to accept this "
                                    "certificate forever without "
                                    "being prompted?"),
                                i18n("Server Authentication"),
                                KGuiItem(i18n("&Forever"), "flag-green"),
                                KGuiItem(i18n("&Current Session only"), "chronometer"));
        QDateTime ruleExpiry = QDateTime::currentDateTime();
        if (msgResult == KMessageBox::Yes) {
            //accept forever ("for a very long time")
            ruleExpiry = ruleExpiry.addYears(1000);
        } else {
            //accept "for a short time", half an hour.
            ruleExpiry = ruleExpiry.addSecs(30*60);
        }

        //TODO special cases for wildcard domain name in the certificate!
        //rule = KSslCertificateRule(d->socket.peerCertificateChain().first(), whatever);

        rule.setExpiryDateTime(ruleExpiry);
        rule.setIgnoredErrors(ud->sslErrors);
        cm->setRule(rule);
    }

    return true;
}

}
}
