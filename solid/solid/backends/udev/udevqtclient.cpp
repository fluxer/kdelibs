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

namespace UdevQt {

Client::Client(const QList<QByteArray>& subsystems, QObject *parent)
    : QObject(parent), m_udev(udev_new()), m_monitor(0), m_monitorNotifier(0)
{
    // create a listener
    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (Q_UNLIKELY(!m_monitor)) {
        qWarning("UdevQt: unable to create udev monitor connection");
        return;
    }

    // apply subsystem filters
    foreach (const QByteArray &it, subsystems) {
        udev_monitor_filter_add_match_subsystem_devtype(m_monitor, it.constData(), NULL);
    }

    // start the monitor receiving
    udev_monitor_enable_receiving(m_monitor);
    m_monitorNotifier = new QSocketNotifier(udev_monitor_get_fd(m_monitor), QSocketNotifier::Read);
    QObject::connect(m_monitorNotifier, SIGNAL(activated(int)), this, SLOT(monitorReadyRead(int)));
}

Client::~Client()
{
    delete m_monitorNotifier;
    if (m_monitor) {
        udev_monitor_unref(m_monitor);
    }
    udev_unref(m_udev);
}

DeviceList Client::allDevices()
{
    DeviceList ret;

    struct udev_enumerate *en = udev_enumerate_new(m_udev);
    udev_enumerate_scan_devices(en);

    struct udev_list_entry *entry;
    struct udev_list_entry *list = udev_enumerate_get_list_entry(en);
    udev_list_entry_foreach(entry, list) {
        struct udev_device *ud = udev_device_new_from_syspath(udev_enumerate_get_udev(en),
                                        udev_list_entry_get_name(entry));

        if (!ud) {
            continue;
        }
        ret << Device(ud, false);
    }

    udev_enumerate_unref(en);

    return ret;
}

Device Client::deviceBySysfsPath(const QString &sysfsPath)
{
    const QByteArray sysfsPathBytes = sysfsPath.toLatin1();
    struct udev_device *ud = udev_device_new_from_syspath(m_udev, sysfsPathBytes.constData());
    if (!ud) {
        return Device();
    }
    return Device(ud, false);
}


void Client::monitorReadyRead(int fd)
{
    Q_UNUSED(fd);
    m_monitorNotifier->setEnabled(false);
    struct udev_device *dev = udev_monitor_receive_device(m_monitor);
    m_monitorNotifier->setEnabled(true);

    if (!dev) {
        return;
    }
    Device device(dev, false);

    QByteArray action(udev_device_get_action(dev));
    if (action == "add") {
        emit deviceAdded(device);
    } else if (action == "remove") {
        emit deviceRemoved(device);
    } else if (action == "change") {
        emit deviceChanged(device);
    } else if (action == "online") {
        emit deviceOnlined(device);
    } else  if (action == "offline") {
        emit deviceOfflined(device);
    /*
        bind/unbind are driver changing for device type of event, on some systems it appears to be
        broken and doing it all the time thus ignore the actions
    */
    } else if (Q_UNLIKELY(action != "bind" && action != "unbind")) {
        qWarning("UdevQt: unhandled device action \"%s\"", action.constData());
    }
}

}

#include "moc_udevqt.cpp"
