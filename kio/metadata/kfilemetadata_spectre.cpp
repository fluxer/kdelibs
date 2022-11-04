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

#include "kfilemetadata_spectre.h"
#include "kpluginfactory.h"
#include "kdebug.h"

#include <libspectre/spectre.h>

KFileMetaDataSpectrePlugin::KFileMetaDataSpectrePlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataSpectrePlugin::~KFileMetaDataSpectrePlugin()
{
}

QStringList KFileMetaDataSpectrePlugin::keys() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount");
    return result;
}

QList<KFileMetaInfoItem> KFileMetaDataSpectrePlugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    SpectreDocument *spectredocument = spectre_document_new();
    if (!spectredocument) {
        kWarning() << "Could not create document";
        return result;
    }
    spectre_document_load(spectredocument, urlpath.constData());
    const SpectreStatus spectrestatus = spectre_document_status(spectredocument);
    if (spectrestatus != SPECTRE_STATUS_SUCCESS) {
        kWarning() << "Could not open" << urlpath;
        spectre_document_free(spectredocument);
        return result;
    }
    const QString spectretitle = QString::fromUtf8(spectre_document_get_title(spectredocument));
    if (!spectretitle.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                spectretitle
            )
        );
    }
    const QString spectrecreator = QString::fromUtf8(spectre_document_get_creator(spectredocument));
    if (!spectrecreator.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"),
                spectrecreator
            )
        );
    }
    const QString spectrecreationdate = QString::fromUtf8(spectre_document_get_creation_date(spectredocument));
    if (!spectrecreationdate.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                spectrecreationdate
            )
        );
    }
    const uint spectrenpages = spectre_document_get_n_pages(spectredocument);
    if (spectrenpages > 0) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount"),
                QString::number(spectrenpages)
            )
        );
    }
    spectre_document_free(spectredocument);
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataSpectrePluginFactory, registerPlugin<KFileMetaDataSpectrePlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataSpectrePluginFactory("kfilemetadata_spectre"))

#include "moc_kfilemetadata_spectre.cpp"
