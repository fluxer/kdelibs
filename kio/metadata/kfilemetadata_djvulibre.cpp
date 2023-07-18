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

#include "kfilemetadata_djvulibre.h"
#include "kpluginfactory.h"
#include "kdebug.h"

#include <QCoreApplication>
#include <QThread>

#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>

static const int s_eventstime = 250;
static const int s_sleeptime = 100;

KFileMetaDataDjVuLibrePlugin::KFileMetaDataDjVuLibrePlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataDjVuLibrePlugin::~KFileMetaDataDjVuLibrePlugin()
{
}

QList<KFileMetaInfoItem> KFileMetaDataDjVuLibrePlugin::metaData(const KUrl &url)
{
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toUtf8();
    ddjvu_context_t* djvuctx = ddjvu_context_create("kfilemetadata_djvulibre");
    if (!djvuctx) {
        kWarning() << "Could not create DjVu context";
        return result;
    }
    ddjvu_document_t* djvudoc = ddjvu_document_create_by_filename_utf8(djvuctx, urlpath.constData(), FALSE);
    if (!djvudoc) {
        kWarning() << "Could not create DjVu document";
        ddjvu_context_release(djvuctx);
        return result;
    }
    kDebug() << "Waiting for document decoding to complete";
    while (!ddjvu_document_decoding_done(djvudoc)) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    kDebug() << "Done waiting for document decoding to complete";
    const int djvulibrepages = ddjvu_document_get_pagenum(djvudoc);
    if (djvulibrepages > 0) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount"),
                QString::number(djvulibrepages)
            )
        );
    }
    kDebug() << "Waiting for document annotation decoding to complete";
    miniexp_t djvulibreannotation = ddjvu_document_get_anno(djvudoc, 1);
    while (djvulibreannotation == miniexp_dummy) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, s_eventstime);
        QThread::msleep(s_sleeptime);
    }
    kDebug() << "Done waiting for document annotation decoding to complete";
    miniexp_t* djvulibremetadatakeys = ddjvu_anno_get_metadata_keys(djvulibreannotation);
    if (djvulibremetadatakeys) {
        int counter = 0;
        while (djvulibremetadatakeys[counter]) {
            // for reference:
            // https://linux.die.net/man/1/djvused
            const char* djvulibrekeyname = miniexp_to_name(djvulibremetadatakeys[counter]);
            const char* djvulibremetadata = ddjvu_anno_get_metadata(djvulibreannotation, djvulibremetadatakeys[counter]);
            // qDebug() << Q_FUNC_INFO << djvulibrekeyname << djvulibremetadata;
            if (qstricmp(djvulibrekeyname, "title") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else if (qstricmp(djvulibrekeyname, "author") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else if (qstricmp(djvulibrekeyname, "creator") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else if (qstricmp(djvulibrekeyname, "subject") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else if (qstricmp(djvulibrekeyname, "creationdate") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else if (qstricmp(djvulibrekeyname, "moddate") == 0) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified"),
                        QString::fromUtf8(djvulibremetadata)
                    )
                );
            } else {
                kDebug() << "Unknown DjVu metadata key" << djvulibrekeyname;
            }
            counter++;
        }
        ::free(djvulibremetadatakeys);
    }
    ddjvu_document_release(djvudoc);
    ddjvu_context_release(djvuctx);
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataDjVuLibrePluginFactory, registerPlugin<KFileMetaDataDjVuLibrePlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataDjVuLibrePluginFactory("kfilemetadata_djvulibre"))

#include "moc_kfilemetadata_djvulibre.cpp"
