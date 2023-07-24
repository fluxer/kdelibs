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
#include "kglobal.h"
#include "klocale.h"
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

QList<KFileMetaInfoItem> KFileMetaDataFFmpegPlugin::metaData(const KUrl &url)
{
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    AVFormatContext *ffmpegcontext = NULL;
    const int ffmpegresult = avformat_open_input(&ffmpegcontext, urlpath.constData(), NULL, NULL);
    if (ffmpegresult != 0 || !ffmpegcontext) {
        kWarning() << "Could not open" << urlpath;
        return result;
    }
    for (uint i = 0; i < ffmpegcontext->nb_streams; i++) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 33, 100)
        const AVCodecParameters *ffmpegcodec = ffmpegcontext->streams[i]->codecpar;
#else
        const AVCodecContext *ffmpegcodec = ffmpegcontext->streams[i]->codec;
#endif
        if (ffmpegcodec) {
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 11, 0)
            const char *ffmpegcodecname = avcodec_get_name(ffmpegcodec->codec_id);
#else
            const char *ffmpegcodecname = ffmpegcodec->codec_name;
#endif
            if (ffmpegcodec->codec_type == AVMEDIA_TYPE_VIDEO) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoCodec"),
                        QString::fromUtf8(ffmpegcodecname)
                    )
                );
                if (ffmpegcodec->width > 0) {
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width"),
                            QString::number(ffmpegcodec->width)
                        )
                    );
                }
                if (ffmpegcodec->height > 0) {
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height"),
                            QString::number(ffmpegcodec->height)
                        )
                    );
                }
                if (ffmpegcodec->bit_rate > 0) {
                    const QString ffmpegbitrate = i18nc("kfilemetadata", "%1 kb/s", ffmpegcodec->bit_rate / 1000);
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#videoBitRate"),
                            ffmpegbitrate
                        )
                    );
                }
                if (ffmpegcontext->streams[i]->avg_frame_rate.num > 0 && ffmpegcontext->streams[i]->avg_frame_rate.den > 0) {
                    const QString ffmpegbitrate = QString::number(av_q2d(ffmpegcontext->streams[i]->avg_frame_rate));
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate"),
                            ffmpegbitrate
                        )
                    );
                }
            } else if (ffmpegcodec->codec_type == AVMEDIA_TYPE_AUDIO) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioCodec"),
                        QString::fromUtf8(ffmpegcodecname)
                    )
                );
                if (ffmpegcodec->sample_rate > 0) {
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate"),
                            QString::number(ffmpegcodec->sample_rate)
                        )
                    );
                }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 24, 100)
                if (ffmpegcodec->ch_layout.nb_channels > 0) {
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels"),
                            QString::number(ffmpegcodec->ch_layout.nb_channels)
                        )
                    );
                }
#else
                if (ffmpegcodec->channels > 0) {
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels"),
                            QString::number(ffmpegcodec->channels)
                        )
                    );
                }
#endif
                if (ffmpegcodec->bit_rate > 0) {
                    const QString ffmpegbitrate = i18nc("kfilemetadata", "%1 kb/s", ffmpegcodec->bit_rate / 1000);
                    result.append(
                        KFileMetaInfoItem(
                            QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#audioBitRate"),
                            ffmpegbitrate
                        )
                    );
                }
            } else if (ffmpegcodec->codec_type == AVMEDIA_TYPE_SUBTITLE) {
                result.append(
                    KFileMetaInfoItem(
                        QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#subtitleCodec"),
                        QString::fromUtf8(ffmpegcodecname)
                    )
                );
            }
        }
    }
    if (ffmpegcontext->duration > 0) {
        const int ffmpegduration = (ffmpegcontext->duration / AV_TIME_BASE);
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration"),
                KGlobal::locale()->formatDuration(ffmpegduration * 1000)
            )
        );
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
