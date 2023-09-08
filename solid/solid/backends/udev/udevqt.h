/*
    Copyright 2009 Benjamin K. Stuhl <bks24@cornell.edu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UDEVQT_H
#define UDEVQT_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QSocketNotifier>

extern "C"
{
#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE
#include <libudev.h>
}

namespace UdevQt
{

class Device
{
public:
    Device();
    Device(struct udev_device *device, bool ref = true);
    Device(const Device &other);
    ~Device();
    Device &operator= (const Device &other);

    bool isValid() const;
    QByteArray subsystem() const;
    QString devType() const;
    QString name() const;
    QString sysfsPath() const;
    int sysfsNumber() const;
    QByteArray driver() const;
    QStringList alternateDeviceSymlinks() const;
    QStringList deviceProperties() const;
    Device parent() const;

    QString deviceProperty(const QByteArray &name) const;
    QString sysfsProperty(const QByteArray &name) const;

private:
    struct udev_device *m_device;
};

typedef QList<Device> DeviceList;

class Client : public QObject
{
    Q_OBJECT
public:
    Client(const QList<QByteArray> &subsystems, QObject *parent = nullptr);
    ~Client();

    DeviceList allDevices();
    Device deviceBySysfsPath(const QString &sysfsPath);

Q_SIGNALS:
    void deviceAdded(const UdevQt::Device &dev);
    void deviceRemoved(const UdevQt::Device &dev);
    void deviceChanged(const UdevQt::Device &dev);

private Q_SLOTS:
    void monitorReadyRead(int fd);

private:
    struct udev *m_udev;
    struct udev_monitor *m_monitor;
    QSocketNotifier *m_monitorNotifier;
};


static inline QStringList listFromListEntry(struct udev_list_entry *list)
{
    QStringList ret;
    struct udev_list_entry *entry;
    udev_list_entry_foreach(entry, list) {
        ret << QString::fromUtf8(udev_list_entry_get_name(entry));
    }
    return ret;
}

}

#endif
