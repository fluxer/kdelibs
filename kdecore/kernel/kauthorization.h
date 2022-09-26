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

    KAuthorization class can be used to implement helper program that does tasks which require
    priviledge elevation (e.g. executing command that requires the current user to be root as
    regular user).

    Helper class looks like this:
    @code
    class MyHelper : public KAuthorization
    {
        Q_OBJECT
    public Q_SLOTS:
        int mymethod(const QVariantMap &args)
        {
            kDebug() << args.value("foo").toString();
            return KAuthorization::NoError;
        }
    }

    K_AUTH_MAIN("org.kde.myhelper", MyHelper)
    @endcode

    Note that the return type of the method is integer which means that @p errno and process exit
    codes can be returned from it and used to present human-readable error string to the user if
    the standard status codes (@p KAuthorizationStatus) are not sufficient.

    The helper authorization policy files must be installed from the project build system via the
    @p kde4_install_auth_helper_files function.

    Helper method execution looks like this:
    @code
    QVariantMap myargs;
    myargs.insert("foo", "bar");
    int myresult = KAuthorization::execute("org.kde.myhelper", "mymethod", myargs);
    if (myresult != KAuthorization::NoError) {
        kWarning() << KAuthorization::errorString(myresult);
    }
    @endcode

    @since 4.22
*/
class KDECORE_EXPORT KAuthorization : public QObject
{
    Q_OBJECT

public:
    /*!
        @brief Standard status codes that @p KAuthorization::execute may return
        @note custom status codes are supported and should be positive integers
    */
    enum KAuthorizationStatus {
        NoError = 0,
        HelperError = -1,
        DBusError = -2,
        MethodError = -3,
        AuthorizationError = -4
    };

    KAuthorization(QObject *parent = nullptr);

    /*!
        @brief Returns @p true if the current user is allowed to execute @p helper methods,
        @p false otherwise
    */
    static bool isAuthorized(const QString &helper);
    /*!
        @brief Executes @p method of @p helper with arguments specified as @p arguments
        synchronously and returns its status (usually one of @p KAuthorizationStatus)
    */
    static int execute(const QString &helper, const QString &method, const QVariantMap &arguments);
    /*!
        @brief Returns string representation of the status code specified as @p status
    */
    static QString errorString(const int status);

    //! @internal
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