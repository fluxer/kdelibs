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

#include "kfilemetadata_epub.h"
#include "kpluginfactory.h"
#include "kdebug.h"

#include <epub.h>

static QString getEPubMetadata(struct epub *epubdocument, enum epub_metadata epubmetadata)
{
    QString result;
    int epubdatasize = 0;
    uchar **epubdata = epub_get_metadata(epubdocument, epubmetadata, &epubdatasize);
    if (!epubdata) {
        return result;
    }
    for (int i = 0; i < epubdatasize; i++) {
        if (i == 0) {
            result = QString::fromUtf8(reinterpret_cast<char*>(epubdata[i]));
        } else {
            result.append(QLatin1String(", "));
            result.append(QString::fromUtf8(reinterpret_cast<char*>(epubdata[i])));
        }
        ::free(epubdata[i]);
    }
    ::free(epubdata);
    return result;
}

KFileMetaDataEPubPlugin::KFileMetaDataEPubPlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataEPubPlugin::~KFileMetaDataEPubPlugin()
{
}

QStringList KFileMetaDataEPubPlugin::keys() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright");
    return result;
}

QList<KFileMetaInfoItem> KFileMetaDataEPubPlugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    struct epub *epubdocument = epub_open(urlpath.constData(), 1);
    if (!epubdocument) {
        kWarning() << "Could not open" << urlpath;
        return result;
    }
    const QString epubid = getEPubMetadata(epubdocument, EPUB_ID);
    if (!epubid.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#uniqueFileIdentifier"),
                epubid
            )
        );
    }
    const QString epubtitle = getEPubMetadata(epubdocument, EPUB_TITLE);
    if (!epubtitle.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                epubtitle
            )
        );
    }
    const QString epubcreator = getEPubMetadata(epubdocument, EPUB_CREATOR);
    if (!epubcreator.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"),
                epubcreator
            )
        );
    }
    const QString epubcontrib = getEPubMetadata(epubdocument, EPUB_CONTRIB);
    if (!epubcontrib.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor"),
                epubcontrib
            )
        );
    }
    const QString epubsubject = getEPubMetadata(epubdocument, EPUB_SUBJECT);
    if (!epubsubject.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"),
                epubsubject
            )
        );
    }
    const QString epubpublisher = getEPubMetadata(epubdocument, EPUB_PUBLISHER);
    if (!epubpublisher.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher"),
                epubpublisher
            )
        );
    }
    const QString epubdescription = getEPubMetadata(epubdocument, EPUB_DESCRIPTION);
    if (!epubdescription.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description"),
                epubdescription
            )
        );
    }
    const QString epubdate = getEPubMetadata(epubdocument, EPUB_DATE);
    if (!epubdate.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                epubdate
            )
        );
    }
    const QString epubrights = getEPubMetadata(epubdocument, EPUB_RIGHTS);
    if (!epubrights.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"),
                epubrights
            )
        );
    }
    epub_close(epubdocument);
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataEPubPluginFactory, registerPlugin<KFileMetaDataEPubPlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataEPubPluginFactory("kfilemetadata_epub"))

#include "moc_kfilemetadata_epub.cpp"
