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

#include "kfilemetadata_ffmpeg.h"
#include "kpluginfactory.h"
#include "kmimetype.h"
#include "kdebug.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

KFileMetaDataFFmpegPlugin::KFileMetaDataFFmpegPlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
}

KFileMetaDataFFmpegPlugin::~KFileMetaDataFFmpegPlugin()
{
}

QStringList KFileMetaDataFFmpegPlugin::keys() const
{
    static const QStringList result = QStringList()
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber")
        << QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate");
    return result;
}

QStringList KFileMetaDataFFmpegPlugin::mimeTypes() const
{
    QStringList result;
    const AVCodecDescriptor* ffmpegcodec = avcodec_descriptor_next(NULL);
    while (ffmpegcodec) {
        if (ffmpegcodec->mime_types) {
            int ffmpegcodeccounter = 0;
            while (ffmpegcodec->mime_types[ffmpegcodeccounter]) {
                // qDebug() << Q_FUNC_INFO << ffmpegcodec->mime_types[ffmpegcodeccounter];
                result.append(QString::fromLatin1(ffmpegcodec->mime_types[ffmpegcodeccounter]));
                ffmpegcodeccounter++;
            }
        }
        ffmpegcodec = avcodec_descriptor_next(ffmpegcodec);
    }
    // most of the above MIME types are for images or null so manually adding all audio and video
    // MIME types
    foreach (const KMimeType::Ptr &kmimetype, KMimeType::allMimeTypes()) {
        if (kmimetype->name().startsWith("audio/") || kmimetype->name().startsWith("video/")) {
            result.append(kmimetype->name());
        }
    }
    result.removeDuplicates();
    // qDebug() << Q_FUNC_INFO << result;
    return result;
}

QList<KFileMetaInfoItem> KFileMetaDataFFmpegPlugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    AVFormatContext *ffmpegcontext = NULL;
    const int ffmpegresult = avformat_open_input(&ffmpegcontext, urlpath.constData(), NULL, NULL);
    if (ffmpegresult != 0 || !ffmpegcontext) {
        kWarning() << "Could not open" << urlpath;
        return result;
    }
    AVDictionaryEntry *ffmpegentry = av_dict_get(ffmpegcontext->metadata, "album", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "artist", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "comment", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "composer", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "copyright", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "date", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "encoder", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encoder"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "encoded_by", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "genre", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "performer", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "publisher", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "title", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "track", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    ffmpegentry = av_dict_get(ffmpegcontext->metadata, "variant_bitrate", NULL, 0);
    if (ffmpegentry) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate"),
                QString::fromUtf8(ffmpegentry->value)
            )
        );
    }
    avformat_free_context(ffmpegcontext);
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataFFmpegPluginFactory, registerPlugin<KFileMetaDataFFmpegPlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataFFmpegPluginFactory("kfilemetadata_ffmpeg"))

#include "moc_kfilemetadata_ffmpeg.cpp"
