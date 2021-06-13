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

#include "udevqt.h"
#include "udevqt_p.h"

#include <QtCore/QSocketNotifier>
#include <qplatformdefs.h>

namespace UdevQt {

ClientPrivate::ClientPrivate(Client *q_, const QStringList &subsystemList)
    : udev(udev_new()), monitor(0), q(q_), monitorNotifier(0)
{
    // create a listener
    monitor = udev_monitor_new_from_netlink(udev, "udev");

    if (!monitor) {
        qWarning("UdevQt: unable to create udev monitor connection");
        return;
    }

    // apply subsystem filters
    foreach (const QString &subsysDevtype, subsystemList) {
        udev_monitor_filter_add_match_subsystem_devtype(monitor, subsysDevtype.toLatin1().constData(), NULL);
    }

    // start the monitor receiving
    udev_monitor_enable_receiving(monitor);
    monitorNotifier = new QSocketNotifier(udev_monitor_get_fd(monitor), QSocketNotifier::Read);
    QObject::connect(monitorNotifier, SIGNAL(activated(int)), q, SLOT(_uq_monitorReadyRead(int)));
}

ClientPrivate::~ClientPrivate()
{
    udev_unref(udev);
    delete monitorNotifier;

    if (monitor)
        udev_monitor_unref(monitor);
}

void ClientPrivate::_uq_monitorReadyRead(int fd)
{
    Q_UNUSED(fd);
    monitorNotifier->setEnabled(false);
    struct udev_device *dev = udev_monitor_receive_device(monitor);
    monitorNotifier->setEnabled(true);

    if (!dev)
        return;

    Device device(new DevicePrivate(dev, false));

    QByteArray action(udev_device_get_action(dev));
    if (action == "add") {
        emit q->deviceAdded(device);
    } else if (action == "remove") {
        emit q->deviceRemoved(device);
    } else if (action == "change") {
        emit q->deviceChanged(device);
    } else if (action == "online") {
        emit q->deviceOnlined(device);
    } else  if (action == "offline") {
        emit q->deviceOfflined(device);
    /*
        bind/unbind are driver changing for device type of event, on some systems it appears to be
        broken and doing it all the time thus ignore the actions
    */
    } else if (action != "bind" && action != "unbind") {
        qWarning("UdevQt: unhandled device action \"%s\"", action.constData());
    }
}

Client::Client(const QStringList& subsystemList, QObject *parent)
    : QObject(parent)
    , d(new ClientPrivate(this, subsystemList))
{
}

Client::~Client()
{
    delete d;
}

DeviceList Client::allDevices()
{
    DeviceList ret;

    struct udev_enumerate *en = udev_enumerate_new(d->udev);
    udev_enumerate_scan_devices(en);

    struct udev_list_entry *entry;
    struct udev_list_entry *list = udev_enumerate_get_list_entry(en);
    udev_list_entry_foreach(entry, list) {
        struct udev_device *ud = udev_device_new_from_syspath(udev_enumerate_get_udev(en),
                                        udev_list_entry_get_name(entry));

        if (!ud)
            continue;

        ret << Device(new DevicePrivate(ud, false));
    }

    udev_enumerate_unref(en);

    return ret;
}

Device Client::deviceBySysfsPath(const QString &sysfsPath)
{
    struct udev_device *ud = udev_device_new_from_syspath(d->udev, sysfsPath.toLatin1().constData());

    if (!ud)
        return Device();

    return Device(new DevicePrivate(ud, false));
}

}

#include "moc_udevqt.cpp"
