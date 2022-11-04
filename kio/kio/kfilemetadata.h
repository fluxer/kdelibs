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

#ifndef KFILEMETADATA_H
#define KFILEMETADATA_H

#include "kio_export.h"
#include "kurl.h"
#include "kfilemetainfo.h"
#include "kfilemetainfoitem.h"

#include <QObject>
#include <QStringList>

/*!
    Base class for plugins to retrieve file metadata.

    There are two template strings that are localized and can be used by plugins where appropriate:
    @code
    const QString bitratestring = i18nc("kfilemetadata", "%1 kb/s", bitrate);
    const QString sampleratestring = i18nc("kfilemetadata", "%1 Hz", samplerate);
    @endcode

    @since 4.21
    @note all virtual methods, despite not being pure-virtual, must be reimplemented
*/
class KIO_EXPORT KFileMetaDataPlugin : public QObject
{
    Q_OBJECT
public:
    KFileMetaDataPlugin(QObject *parent = nullptr);
    ~KFileMetaDataPlugin();

    virtual QList<KFileMetaInfoItem> metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags);
};

#endif // KFILEMETADATA_H
