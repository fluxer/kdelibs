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

#include "kdevicedatabase.h"
#include "kstandarddirs.h"
#include "kdebug.h"

#include <QFile>

struct KDeviceEntry
{
    QByteArray vendorid;
    QByteArray deviceid;

    bool operator==(const KDeviceEntry &other) const
        { return vendorid == other.vendorid && deviceid == other.deviceid; }
};
inline uint qHash(const KDeviceEntry &kdeviceentry)
{
    return qHash(kdeviceentry.vendorid + kdeviceentry.deviceid);
}

struct KProtocolEntry
{
    QByteArray vendorid;
    QByteArray deviceid;
    QByteArray protocolid;

    bool operator==(const KProtocolEntry &other) const
        { return vendorid == other.vendorid && deviceid == other.deviceid && protocolid == other.protocolid; }
};
inline uint qHash(const KProtocolEntry &kprotocolentry)
{
    return qHash(kprotocolentry.vendorid + kprotocolentry.deviceid + kprotocolentry.protocolid);
}

typedef QHash<QByteArray, QString> KVendorEntryMap;
typedef QHash<KDeviceEntry, QString> KDeviceEntryMap;
typedef QHash<QByteArray, QString> KClassEntryMap;
typedef QHash<KDeviceEntry, QString> KSubClassEntryMap;
typedef QHash<KProtocolEntry, QString> KProtocolEntryMap;

static inline QByteArray normalizeID(const QByteArray &id, const int padding)
{
    // some but not all device vendor/product properties (e.g. ID_VENDOR_ID and ID_MODEL_ID on network
    // devices on Linux) have "0x" prefix, get rid of that inconsistency
    QByteArray result = id;
    if (result.startsWith("0x")) {
        result = result.mid(2, result.size() - 2);
    }
    while (result.size() < padding) {
        result.prepend('0');
    }
    return result;
}

static QList<QByteArray> extractEntry(const QByteArray &line)
{
    QList<QByteArray> result;
    const int doublespaceindex = line.indexOf("  ");
    if (doublespaceindex != 4 && doublespaceindex != 2) {
        return result;
    }
    result.append(line.left(doublespaceindex));
    result.append(line.mid(doublespaceindex + 2));
    return result;
}

static void extractIDs(QFile *idsfile,
                       KVendorEntryMap *vendormap, KDeviceEntryMap *devicemap,
                       KClassEntryMap *classmap, KSubClassEntryMap *subclassmap, KProtocolEntryMap *protocolmap)
{
    bool classessection = false;
    QByteArray lastvendorid;
    QByteArray lastdeviceid;
    while (!idsfile->atEnd()) {
        const QByteArray dbline = idsfile->readLine();
        const QByteArray trimmeddbline = dbline.trimmed();
        // qDebug() << Q_FUNC_INFO << dbline;
        if (dbline.startsWith("C ")) {
            // class on this line and after
            classessection = true;
        }
        if (trimmeddbline.isEmpty() || trimmeddbline.startsWith('#')) {
            if (classessection) {
                // undocumented entries after class
                break;
            }
            continue;
        }

        if (classessection) {
            // qDebug() << Q_FUNC_INFO << trimmeddbline;
            if (dbline.startsWith("\t\t")) {
                const QList<QByteArray> protocolentry = extractEntry(trimmeddbline);
                if (protocolentry.size() != 2) {
                    kWarning() << "Invalid protocol line" << trimmeddbline;
                    continue;
                }
                if (lastvendorid.isEmpty()) {
                    kWarning() << "Protocol line before class" << trimmeddbline;
                    continue;
                }
                if (lastdeviceid.isEmpty()) {
                    kWarning() << "Protocol line before sub-class" << trimmeddbline;
                    continue;
                }
                protocolmap->insert({ lastvendorid, lastdeviceid, protocolentry.at(0) }, QString::fromAscii(protocolentry.at(1)));
            } else if (dbline.startsWith("\t")) {
                const QList<QByteArray> subclassentry = extractEntry(trimmeddbline);
                if (subclassentry.size() != 2) {
                    kWarning() << "Invalid sub-class line" << trimmeddbline;
                    continue;
                }
                if (lastvendorid.isEmpty()) {
                    kWarning() << "Sub-class line before class" << trimmeddbline;
                    continue;
                }
                lastdeviceid = subclassentry.at(0);
                subclassmap->insert({ lastvendorid, subclassentry.at(0) }, QString::fromAscii(subclassentry.at(1)));
            } else {
                const QList<QByteArray> classentry = extractEntry(trimmeddbline.mid(2));
                if (classentry.size() != 2) {
                    kWarning() << "Invalid class line" << trimmeddbline;
                    continue;
                }
                lastvendorid = classentry.at(0);
                classmap->insert(lastvendorid, QString::fromAscii(classentry.at(1)));
            }
        } else {
            if (dbline.startsWith("\t\t")) {
                // sub-device
                continue;
            } else if (dbline.startsWith("\t")) {
                const QList<QByteArray> deviceentry = extractEntry(trimmeddbline);
                if (deviceentry.size() != 2) {
                    kWarning() << "Invalid device line" << trimmeddbline;
                    continue;
                }
                if (lastvendorid.isEmpty()) {
                    kWarning() << "Device line before vendor" << trimmeddbline;
                    continue;
                }
                devicemap->insert({ lastvendorid, deviceentry.at(0) }, QString::fromAscii(deviceentry.at(1)));
            } else {
                const QList<QByteArray> vendorentry = extractEntry(trimmeddbline);
                if (vendorentry.size() != 2) {
                    kWarning() << "Invalid vendor line" << trimmeddbline;
                    continue;
                }
                lastvendorid = vendorentry.at(0);
                vendormap->insert(lastvendorid, QString::fromAscii(vendorentry.at(1)));
            }
        }
    }
}

class KDeviceDatabasePrivate
{
public:
    KDeviceDatabasePrivate();

    bool readPCIDatabase();
    bool readUSBDatabase();

    KVendorEntryMap pcivendorsmap;
    KDeviceEntryMap pcidevicesmap;
    KClassEntryMap pciclassesmap;
    KSubClassEntryMap pcisubclassesmap;
    KProtocolEntryMap pciprotocolsmap;
    KVendorEntryMap usbvendorsmap;
    KDeviceEntryMap usbdevicesmap;
    KClassEntryMap usbclassesmap;
    KSubClassEntryMap usbsubclassesmap;
    KProtocolEntryMap usbprotocolsmap;

private:
    bool m_pcicached;
    bool m_usbcached;
};

KDeviceDatabasePrivate::KDeviceDatabasePrivate()
    : m_pcicached(false),
    m_usbcached(false)
{
}

bool KDeviceDatabasePrivate::readPCIDatabase()
{
    if (m_pcicached) {
        return true;
    }

    const QString pciids = KStandardDirs::locate("data", QString::fromLatin1("kdevicedatabase/pci.ids"));
    QFile pciidsfile(pciids);
    if (!pciidsfile.open(QFile::ReadOnly)) {
        kWarning() << "PCI IDs database not found";
        return false;
    }

    pcivendorsmap.clear();
    pcidevicesmap.clear();
    pciclassesmap.clear();
    pcisubclassesmap.clear();
    pciprotocolsmap.clear();
    extractIDs(&pciidsfile,
        &pcivendorsmap, &pcidevicesmap,
        &pciclassesmap, &pcisubclassesmap, &pciprotocolsmap
    );
    // qDebug() << Q_FUNC_INFO << pcivendorsmap;

    m_pcicached = true;
    return true;
}

bool KDeviceDatabasePrivate::readUSBDatabase()
{
    if (m_usbcached) {
        return true;
    }
    
    const QString usbids = KStandardDirs::locate("data", QString::fromLatin1("kdevicedatabase/usb.ids"));
    QFile usbidsfile(usbids);
    if (!usbidsfile.open(QFile::ReadOnly)) {
        kWarning() << "USB IDs database not found";
        return false;
    }

    usbvendorsmap.clear();
    usbdevicesmap.clear();
    usbclassesmap.clear();
    usbsubclassesmap.clear();
    usbprotocolsmap.clear();
    extractIDs(&usbidsfile,
        &usbvendorsmap, &usbdevicesmap,
        &usbclassesmap, &usbsubclassesmap, &usbprotocolsmap
    );
    // qDebug() << Q_FUNC_INFO << usbvendorsmap;

    m_usbcached = true;
    return true;
}

KDeviceDatabase::KDeviceDatabase()
    : d(new KDeviceDatabasePrivate())
{
}

QString KDeviceDatabase::lookupPCIVendor(const QByteArray &vendorid)
{
    if (!d->readPCIDatabase()) {
        return QString();
    }

    const QByteArray normalizedvendor = normalizeID(vendorid, 4);
    foreach (const QByteArray &deviceentry, d->pcivendorsmap.keys()) {
        if (qstrcmp(deviceentry.constData(), normalizedvendor.constData()) == 0) {
            return d->pcivendorsmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupPCIDevice(const QByteArray &vendorid, const QByteArray &deviceid)
{
    if (!d->readPCIDatabase()) {
        return QString();
    }

    const QByteArray normalizedvendor = normalizeID(vendorid, 4);
    const QByteArray normalizeddevice = normalizeID(deviceid, 4);
    foreach (const KDeviceEntry &deviceentry, d->pcidevicesmap.keys()) {
        if (qstrcmp(deviceentry.vendorid.constData(), normalizedvendor.constData()) == 0
            && qstrcmp(deviceentry.deviceid.constData(), normalizeddevice.constData()) == 0) {
            return d->pcidevicesmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupPCIClass(const QByteArray &classid)
{
    if (!d->readPCIDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    foreach (const QByteArray &deviceentry, d->pciclassesmap.keys()) {
        if (qstrcmp(deviceentry.constData(), normalizedclass.constData()) == 0) {
            return d->pciclassesmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupPCISubClass(const QByteArray &classid, const QByteArray &subclassid)
{
    if (!d->readPCIDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    const QByteArray normalizedsubclass = normalizeID(subclassid, 2);
    foreach (const KDeviceEntry &deviceentry, d->pcisubclassesmap.keys()) {
        if (qstrcmp(deviceentry.vendorid.constData(), normalizedclass.constData()) == 0
            && qstrcmp(deviceentry.deviceid.constData(), normalizedsubclass.constData()) == 0) {
            return d->pcisubclassesmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupPCIProtocol(const QByteArray &classid, const QByteArray &subclassid, const QByteArray &protocolid)
{
    if (!d->readPCIDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    const QByteArray normalizedsubclass = normalizeID(subclassid, 2);
    const QByteArray normalizedprotocol = normalizeID(protocolid, 2);
    foreach (const KProtocolEntry &protocolentry, d->pciprotocolsmap.keys()) {
        if (qstrcmp(protocolentry.vendorid.constData(), normalizedclass.constData()) == 0
            && qstrcmp(protocolentry.deviceid.constData(), normalizedsubclass.constData()) == 0
            && qstrcmp(protocolentry.protocolid.constData(), normalizedprotocol.constData()) == 0) {
            return d->pciprotocolsmap.value(protocolentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupUSBVendor(const QByteArray &vendorid)
{
    if (!d->readUSBDatabase()) {
        return QString();
    }

    const QByteArray normalizedvendor = normalizeID(vendorid, 4);
    foreach (const QByteArray &deviceentry, d->usbvendorsmap.keys()) {
        if (qstrcmp(deviceentry.constData(), normalizedvendor.constData()) == 0) {
            return d->usbvendorsmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupUSBDevice(const QByteArray &vendorid, const QByteArray &deviceid)
{
    if (!d->readUSBDatabase()) {
        return QString();
    }

    const QByteArray normalizedvendor = normalizeID(vendorid, 4);
    const QByteArray normalizeddevice = normalizeID(deviceid, 4);
    foreach (const KDeviceEntry &deviceentry, d->usbdevicesmap.keys()) {
        if (qstrcmp(deviceentry.vendorid.constData(), normalizedvendor.constData()) == 0
            && qstrcmp(deviceentry.deviceid.constData(), normalizeddevice.constData()) == 0) {
            return d->usbdevicesmap.value(deviceentry);
        }
    }
    return QString();
}



QString KDeviceDatabase::lookupUSBClass(const QByteArray &classid)
{
    if (!d->readUSBDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    foreach (const QByteArray &deviceentry, d->usbclassesmap.keys()) {
        if (qstrcmp(deviceentry.constData(), normalizedclass.constData()) == 0) {
            return d->usbclassesmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupUSBSubClass(const QByteArray &classid, const QByteArray &subclassid)
{
    if (!d->readUSBDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    const QByteArray normalizedsubclass = normalizeID(subclassid, 2);
    foreach (const KDeviceEntry &deviceentry, d->usbsubclassesmap.keys()) {
        if (qstrcmp(deviceentry.vendorid.constData(), normalizedclass.constData()) == 0
            && qstrcmp(deviceentry.deviceid.constData(), normalizedsubclass.constData()) == 0) {
            return d->usbsubclassesmap.value(deviceentry);
        }
    }
    return QString();
}

QString KDeviceDatabase::lookupUSBProtocol(const QByteArray &classid, const QByteArray &subclassid, const QByteArray &protocolid)
{
    if (!d->readUSBDatabase()) {
        return QString();
    }

    const QByteArray normalizedclass = normalizeID(classid, 2);
    const QByteArray normalizedsubclass = normalizeID(subclassid, 2);
    const QByteArray normalizedprotocol = normalizeID(protocolid, 2);
    foreach (const KProtocolEntry &protocolentry, d->usbprotocolsmap.keys()) {
        if (qstrcmp(protocolentry.vendorid.constData(), normalizedclass.constData()) == 0
            && qstrcmp(protocolentry.deviceid.constData(), normalizedsubclass.constData()) == 0
            && qstrcmp(protocolentry.protocolid.constData(), normalizedprotocol.constData()) == 0) {
            return d->usbprotocolsmap.value(protocolentry);
        }
    }
    return QString();
}
