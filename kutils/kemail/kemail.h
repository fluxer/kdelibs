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

#ifndef KEMAIL_H
#define KEMAIL_H

#include "kemail_export.h"

#include <QObject>
#include <QStringList>
#include <kurl.h>

class KEMailPrivate;

/*!
    Class to send mail.
    Example:
    \code
    KEMail kemail;
    kemail.setServer(KUrl("smtp://myuser@myhost.com:123"));
    kemail.setUser("myuser");
    kemail.setPassword("mypass");
    kemail.setFrom("myuser@myhost.com");
    kemail.setTo(QStringList() << "otheruser@otherhost.com");
    qDebug() << kemail.send("hello");
    \endcode

    @since 4.22
    @warning the API is subject to change

    @see KEMailSettings
*/
class KEMAIL_EXPORT KEMail : public QObject
{
    Q_OBJECT
public:
    /*!
        @brief Contructs object with @p parent
    */
    KEMail(QObject *parent = nullptr);
    ~KEMail();

    KUrl server() const;
    bool setServer(const KUrl &server);
    QString user() const;
    bool setUser(const QString &user);
    QString password() const;
    bool setPassword(const QString &password);

    QString from() const;
    bool setFrom(const QString &from);
    QStringList to() const;
    bool setTo(const QStringList &to);

    bool send(const QString &subject, const QString &message, const KUrl::List &attach = KUrl::List());

    //! @brief Returns human-readable description of the error that occured
    QString errorString() const;

private:
    Q_DISABLE_COPY(KEMail);
    KEMailPrivate *d;
};

#endif // KEMAIL_H
