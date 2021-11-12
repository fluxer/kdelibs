/*
    Copyright 2021 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef DEVDQT_H
#define DEVDQT_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QSocketNotifier>

#define DEVD_PIPE "/var/run/devd.seqpacket.pipe"

namespace DevdQt
{

class Device
{
public:
    Device();
    Device(const QByteArray &device);
    Device(const Device &other);
    ~Device();
    Device &operator=(const Device &other);

    bool isValid() const;

    QByteArray device() const;

private:
    QByteArray m_device;
};

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QObject *parent = 0);
    ~Client();

Q_SIGNALS:
    void deviceAdded(const DevdQt::Device &device);
    void deviceRemoved(const DevdQt::Device &device);
    void deviceChanged(const DevdQt::Device &device);

private Q_SLOTS:
    void monitorReadyRead(int fd);

private:
    int m_socket;
    QSocketNotifier *m_monitor;
};

} // namespace DevdQt

#endif // DEVDQT_H
