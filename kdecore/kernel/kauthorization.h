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

#ifndef KAUTHORIZATION_H
#define KAUTHORIZATION_H

#include <kdecore_export.h>

#include <QObject>
#include <QVariant>
#include <QCoreApplication>

/*!
    Authorization class

    @since 4.22
*/
class KDECORE_EXPORT KAuthorization : public QObject
{
    Q_OBJECT

public:
    enum KAuthorizationStatus {
        NoError = 0,
        HelperError = -1,
        DBusError = -2,
        MethodError = -3,
        AuthorizationError = -4
    };

    KAuthorization(QObject *parent = nullptr);

    static bool isAuthorized(const QString &helper);
    static int execute(const QString &helper, const QString &method, const QVariantMap &arguments);
    static QString errorString(const int status);

    static void helperMain(const char* const helper, KAuthorization *object);

private:
    Q_DISABLE_COPY(KAuthorization);
};

// application instance created before class to ensure D-Bus connection is not made before it
#define K_AUTH_MAIN(HELPER, CLASS) \
    int main(int argc, char **argv) { \
        QCoreApplication app(argc, argv); \
        KAuthorization::helperMain(HELPER, new CLASS()); \
        return app.exec(); \
    }

#endif // KAUTHORIZATION_H