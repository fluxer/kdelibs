/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>
    Copyright 2007 Will Stephenosn <wstephenson@kde.org>

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

#include "fakevideo.h"

using namespace Solid::Backends::Fake;

FakeVideo::FakeVideo(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeVideo::~FakeVideo()
{
}

QStringList FakeVideo::supportedProtocols() const
{
    QStringList res;

    res << QLatin1String("video4linux");

    return res;
}

QStringList FakeVideo::supportedDrivers(QString /*protocol*/) const
{
    QStringList res;

    if (fakeDevice()->property("v4l2Support").toBool()) {
        res << "video4linux2";
    } else {
        res << "video4linux1";
    }

    return res;
}

QVariant Solid::Backends::Fake::FakeVideo::driverHandle(const QString &driver) const
{
    return fakeDevice()->property("driverHandle").toString();
}

#include "backends/fakehw/moc_fakevideo.cpp"
