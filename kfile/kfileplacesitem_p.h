/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KFILEPLACESITEM_P_H
#define KFILEPLACESITEM_P_H


#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qstringlist.h>
#include <kbookmark.h>
#include <solid/device.h>

namespace Solid
{
class StorageAccess;
class StorageVolume;
class OpticalDisc;
class PortableMediaPlayer;
}

class KFilePlacesItem : public QObject
{
    Q_OBJECT
public:
    KFilePlacesItem(const KBookmark &bookmark,
                    const QString &udi = QString());
    ~KFilePlacesItem();

    QString id() const;

    bool isDevice() const;
    KBookmark bookmark() const;
    void setBookmark(const KBookmark &bookmark);
    Solid::Device device() const;
    QVariant data(int role) const;

    static KBookmark createBookmark(KBookmarkManager *manager,
                                    const QString &label,
                                    const KUrl &url,
                                    const QString &iconName,
                                    KFilePlacesItem *after = 0);
    static KBookmark createSystemBookmark(KBookmarkManager *manager,
                                          const QString &untranslatedLabel,
                                          const QString &translatedLabel,
                                          const KUrl &url,
                                          const QString &iconName);
    static KBookmark createDeviceBookmark(KBookmarkManager *manager,
                                          const QString &udi);

Q_SIGNALS:
    void itemChanged(const QString &id);

private Q_SLOTS:
    void onAccessibilityChanged(bool);
    void trashConfigChanged(const QString &config);

private:
    QVariant bookmarkData(int role) const;
    QVariant deviceData(int role) const;

    QString iconNameForBookmark(const KBookmark &bookmark) const;

    static QString generateNewId();

    KBookmark m_bookmark;
    bool m_isCdrom;
    bool m_isAccessible;
    bool m_trashIsEmpty;
    QString m_text;
    mutable Solid::Device m_device;
    mutable QPointer<Solid::StorageAccess> m_access;
    mutable QPointer<Solid::PortableMediaPlayer> m_mtp;
    QString m_iconPath;
    QStringList m_emblems;
};

#endif
