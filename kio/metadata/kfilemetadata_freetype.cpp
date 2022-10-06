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

#include "kfilemetadata_freetype.h"
#include "kpluginfactory.h"
#include "kdebug.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H

KFileMetaDataFreetypePlugin::KFileMetaDataFreetypePlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataFreetypePlugin::~KFileMetaDataFreetypePlugin()
{
}

QStringList KFileMetaDataFreetypePlugin::keys() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fontFamily")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright");
    return result;
}

QStringList KFileMetaDataFreetypePlugin::mimeTypes() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("font/ttf")
        << QString::fromLatin1("application/x-font-ttf")
        << QString::fromLatin1("application/x-font-type1")
        << QString::fromLatin1("font/otf")
        << QString::fromLatin1("application/x-font-otf")
        << QString::fromLatin1("application/x-font-afm")
        << QString::fromLatin1("application/x-font-ttx")
        << QString::fromLatin1("font/woff");
    return result;
}

QList<KFileMetaInfoItem> KFileMetaDataFreetypePlugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    FT_Library ftlibrary;
    FT_Init_FreeType(&ftlibrary);
    if (!ftlibrary) {
        kWarning() << "Could not initialize";
        return result;
    }
    FT_Face ftface;
    if (FT_New_Face(ftlibrary, urlpath.constData(), 0, &ftface) != 0) {
        kWarning() << "Could not open" << urlpath;
        FT_Done_FreeType(ftlibrary);
        return result;
    }
    const QString ftfamily = QString::fromUtf8(ftface->family_name);
    if (!ftfamily.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fontFamily"),
                ftfamily
            )
        );
    }
    PS_FontInfoRec ftfontinfo;
    if (FT_Get_PS_Font_Info(ftface, &ftfontinfo) == 0) {
        const QString ftnotice = QString::fromUtf8(ftfontinfo.notice);
        if (!ftnotice.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"),
                    ftnotice
                )
            );
        }
    }
    FT_Done_Face(ftface);
    FT_Done_FreeType(ftlibrary);
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataFreetypePluginFactory, registerPlugin<KFileMetaDataFreetypePlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataFreetypePluginFactory("kfilemetadata_freetype"))

#include "moc_kfilemetadata_freetype.cpp"
