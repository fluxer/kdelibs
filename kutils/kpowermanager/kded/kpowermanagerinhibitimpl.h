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

#ifndef KPOWERMANAGERINHIBITIMPL_H
#define KPOWERMANAGERINHIBITIMPL_H

#include <QObject>
#include <QDBusInterface>

class KPowerManagerInhibitImpl : public QObject
{
    Q_OBJECT
public:
    KPowerManagerInhibitImpl(QObject *parent = nullptr);
    ~KPowerManagerInhibitImpl();

public Q_SLOTS:
    bool HasInhibit();
    uint Inhibit(const QString &application, const QString &reason);
    void UnInhibit(uint cookie);
Q_SIGNALS:
    void HasInhibitChanged(bool has_inhibit);

private Q_SLOTS:
    void slotPropertiesChanged(QString interface, QVariantMap changed_properties, QStringList invalidated_properties);

private:
    bool m_objectregistered;
    bool m_serviceregistered;
    QDBusInterface m_login1;
    QMap<uint, int> m_cookies;
    bool m_hasinhibit;
};

#endif // KPOWERMANAGERINHIBITIMPL_H
