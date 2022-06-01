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

#include "devinfomanager.h"
#include "devinfodevice.h"
#include "devinfoprocessor.h"
#include "devinfonetworkinterface.h"
#include "devinfographic.h"

#include <QDebug>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <devinfo.h>

using namespace Solid::Backends::Devinfo;

typedef QMap<DevinfoDevice::DeviceProperty, QByteArray> DevicePropertiesType;
typedef QPair<QByteArray, DevicePropertiesType> DeviceArgumentType;

int getDeviceProperties(struct devinfo_dev *dev, void *arg)
{
    DeviceArgumentType* typedarg = reinterpret_cast<DeviceArgumentType*>(arg);
    const QByteArray devname(dev->dd_name);
    if (!devname.isEmpty() && devname == typedarg->first) {
        DevicePropertiesType properties;
        properties[DevinfoDevice::DeviceName] = devname;
        struct devinfo_dev *parentdev = devinfo_handle_to_device(dev->dd_parent);
        if (parentdev) {
            properties[DevinfoDevice::DeviceParent] = parentdev->dd_name;
        }
        properties[DevinfoDevice::DeviceDescription] = dev->dd_desc;
        properties[DevinfoDevice::DeviceDriver] = dev->dd_drivername;
        properties[DevinfoDevice::DevicePnPInfo] = dev->dd_pnpinfo;
        properties[DevinfoDevice::DeviceLocation] = dev->dd_location;
        typedarg->second = properties;
        // device found, abort scan
        return -1;
    }
    return devinfo_foreach_device_child(dev, getDeviceProperties, arg);
}

static QByteArray getPnPInfo(const QByteArray &pnpinfo, const char* field)
{
    foreach (const QByteArray &subinfo, pnpinfo.split(' ')) {
        if (subinfo.startsWith(field)) {
            return subinfo.right(subinfo.size() - qstrlen(field) - 1);
        }
    }
    return QByteArray();
}

DevinfoDevice::DevinfoDevice(const QString &device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
    if (devinfo_init() != 0) {
        qWarning() << "could not initialize devinfo";
        return;
    }

    struct devinfo_dev *root = devinfo_handle_to_device(DEVINFO_ROOT_DEVICE);
    if (root) {
        const QByteArray devicename = m_device.right(m_device.size() - qstrlen(DEVINFO_UDI_PREFIX) - 1).toLatin1();
        DeviceArgumentType* getDeviceArg = new DeviceArgumentType(devicename, m_properties);

        devinfo_foreach_device_child(root, getDeviceProperties, getDeviceArg);

        m_properties = getDeviceArg->second;
        delete getDeviceArg;
    } else {
        qWarning() << "no root device";
        return;
    }

    const QByteArray pnpinfo = m_properties[DevinfoDevice::DevicePnPInfo];
    m_pnpinfo[DevinfoDevice::PnPVendor] = getPnPInfo(pnpinfo, "vendor");
    m_pnpinfo[DevinfoDevice::PnPDevice] = getPnPInfo(pnpinfo, "device");
    m_pnpinfo[DevinfoDevice::PnPSubVendor] = getPnPInfo(pnpinfo, "subvendor");
    m_pnpinfo[DevinfoDevice::PnPSubDevice] = getPnPInfo(pnpinfo, "subdevice");
    m_pnpinfo[DevinfoDevice::PnPClass] = getPnPInfo(pnpinfo, "class");

    devinfo_free();
}

DevinfoDevice::~DevinfoDevice()
{
}

QString DevinfoDevice::udi() const
{
    return QString::fromLatin1("%1/%2").arg(DEVINFO_UDI_PREFIX, m_device);
}

QString DevinfoDevice::parentUdi() const
{
    if (m_device == DEVINFO_ROOT_UDI) {
        // root0
        return QString();
    }
    const QByteArray parent = deviceProperty(DevinfoDevice::DeviceParent);
    if (parent.isEmpty()) {
        return QString(DEVINFO_ROOT_UDI);
    }
    return QString::fromLatin1("%1/%2").arg(DEVINFO_UDI_PREFIX, parent.constData());
}

QString DevinfoDevice::vendor() const
{
    const QByteArray pnpvendor = devicePnP(DevinfoDevice::PnPVendor);
    if (pnpvendor.size() < 2) {
        return QString();
    }

    const QByteArray parent = deviceProperty(DevinfoDevice::DeviceParent);
    QString result;
    if (parent.contains("pci")) {
        result = m_devicedb.lookupPCIVendor(pnpvendor);
    } else {
        result = m_devicedb.lookupUSBVendor(pnpvendor);
    }

    return result;
}

QString DevinfoDevice::product() const
{
    if (m_device == DEVINFO_ROOT_UDI) {
        return QLatin1String("Devices");
    }  else if(queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
        const NetworkInterface netIface(const_cast<DevinfoDevice *>(this));
        if (netIface.isLoopback()) {
            return QLatin1String("Loopback device Interface");
        }
    }  else if(queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        const QByteArray hwmodel = DevinfoDevice::stringByName("hw.model");
        return QString::fromAscii(hwmodel.constData(), hwmodel.size());
    }

    const QByteArray pnpvendor = devicePnP(DevinfoDevice::PnPVendor);
    const QByteArray pnpdevice = devicePnP(DevinfoDevice::PnPDevice);
    if (pnpvendor.size() < 2 || pnpdevice.size() < 2) {
        return QString();
    }

    const QByteArray parent = deviceProperty(DevinfoDevice::DeviceParent);
    QString result;
    if (parent.contains("pci")) {
        result = m_devicedb.lookupPCIDevice(pnpvendor, pnpdevice);
    } else {
        result = m_devicedb.lookupUSBDevice(pnpvendor, pnpdevice);
    }

    return result;
}

QString DevinfoDevice::icon() const
{
    if (m_device == DEVINFO_ROOT_UDI) {
        return QString("computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QLatin1String("cpu");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Graphic)) {
        return QLatin1String("video-display");
    }
    return QString();
}

QStringList DevinfoDevice::emblems() const
{
    // no implemented interface requires emblems
    return QStringList();
}

QString DevinfoDevice::description() const
{
    if (m_device == DEVINFO_ROOT_UDI || parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QObject::tr("Processor");
    } else if (queryDeviceInterface(Solid::DeviceInterface::NetworkInterface)) {
        const NetworkInterface networkIface(const_cast<DevinfoDevice *>(this));
        if (networkIface.isWireless()) {
            return QObject::tr("WLAN Interface");
        }
        return QObject::tr("Networking Interface");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Graphic)) {
        return QObject::tr("Graphic display");
    }
    return deviceProperty(DevinfoDevice::DeviceDescription);
}

bool DevinfoDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
        case Solid::DeviceInterface::Processor: {
            return (m_device.indexOf("/cpu") >= 0 && m_device.indexOf("freq") == -1);
        }
        case Solid::DeviceInterface::NetworkInterface: {
            return (m_device.indexOf("/em") >= 0 || m_device.indexOf("/wlan") >= 0);
        }
        case Solid::DeviceInterface::Graphic: {
            return (devicePnP(DevinfoDevice::PnPClass) == "0x030000"); // VGA controller
        }
        default: {
            return false;
        }
    }
}

QObject *DevinfoDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }
    switch (type) {
        case Solid::DeviceInterface::Processor: {
            return new Processor(this);
        }
        case Solid::DeviceInterface::NetworkInterface: {
            return new NetworkInterface(this);
        }
        case Solid::DeviceInterface::Graphic: {
            return new Graphic(this);
        }
        default: {
            Q_ASSERT(false);
            return 0;
        }
    }
}

QByteArray DevinfoDevice::deviceProperty(const DeviceProperty property) const
{
    return  m_properties.value(property);
}

QByteArray DevinfoDevice::devicePnP(const PnPInfo pnp) const
{
    return m_pnpinfo.value(pnp);
}

QByteArray DevinfoDevice::deviceCtl(const char* field) const
{
    const QByteArray devicename = deviceProperty(DevinfoDevice::DeviceName);
    int devicenumberpos = 0;
    const char* devicedata = devicename.constData();
    while (*devicedata) {
        if (::isdigit(*devicedata)) {
            break;
        }
        devicedata++;
        devicenumberpos++;
    }
    QByteArray sysctldevicename = "dev.";
    sysctldevicename += QByteArray(devicename.constData(), devicenumberpos);
    sysctldevicename += '.';
    sysctldevicename += devicedata;
    sysctldevicename += '.';
    sysctldevicename += field;

    return DevinfoDevice::stringByName(sysctldevicename.constData());
}

QByteArray DevinfoDevice::stringByName(const char* sysctlname)
{
    size_t sysctlbuffsize = 1024;
    char sysctlbuff[sysctlbuffsize];
    ::memset(sysctlbuff, '\0', sysctlbuffsize * sizeof(char));
    const int sysctlresult = ::sysctlbyname(sysctlname, sysctlbuff, &sysctlbuffsize, NULL, 0);
    if (sysctlresult == -1) {
        // qWarning() << "sysctlbyname failed" << sysctlname;
        return QByteArray();
    }
    QByteArray result(sysctlbuff, sysctlbuffsize);
    // chop non-printable character at the end, whatever it is
    if (!result.isEmpty() && !::isprint(result.at(result.size() - 1))) {
        result.chop(1);
    }
    return result;
}

qlonglong DevinfoDevice::integerByName(const char* sysctlname)
{
    size_t sysctlbuffsize = sizeof(qlonglong);
    qlonglong sysctlbuff = 0;
    const int sysctlresult = ::sysctlbyname(sysctlname, &sysctlbuff, &sysctlbuffsize, NULL, 0);
    if (sysctlresult == -1) {
        // qWarning() << "sysctlbyname failed" << sysctlname;
        return -1;
    }
    return sysctlbuff;
}
