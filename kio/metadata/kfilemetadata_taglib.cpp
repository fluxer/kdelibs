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

#include "kfilemetadata_taglib.h"
#include "kpluginfactory.h"
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>

KFileMetaDataTagLibPlugin::KFileMetaDataTagLibPlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataTagLibPlugin::~KFileMetaDataTagLibPlugin()
{
}

QList<KFileMetaInfoItem> KFileMetaDataTagLibPlugin::metaData(const KUrl &url, const KFileMetaInfo::WhatFlags flags)
{
    Q_UNUSED(flags);
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    TagLib::FileRef taglibfile(urlpath);
    if (taglibfile.isNull()) {
        kWarning() << "Could not open" << urlpath;
        return result;
    }
    TagLib::Tag *taglibtag = taglibfile.tag();
    if (!taglibtag) {
        kDebug() << "Null tag for" << urlpath;
    } else {
        const QString taglibtitle = QString::fromStdString(taglibtag->title().to8Bit(true));
        if (!taglibtitle.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                    taglibtitle
                )
            );
        }
        const QString taglibartist = QString::fromStdString(taglibtag->artist().to8Bit(true));
        if (!taglibartist.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist"),
                    taglibartist
                )
            );
        }
        const QString taglibalbum = QString::fromStdString(taglibtag->album().to8Bit(true));
        if (!taglibalbum.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum"),
                    taglibalbum
                )
            );
        }
        const QString taglibcomment = QString::fromStdString(taglibtag->comment().to8Bit(true));
        if (!taglibcomment.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment"),
                    taglibcomment
                )
            );
        }
        const QString taglibgenre = QString::fromStdString(taglibtag->genre().to8Bit(true));
        if (!taglibgenre.isEmpty()) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre"),
                    taglibgenre
                )
            );
        }
        const QString taglibyear = QString::number(taglibtag->year());
        if (!taglibyear.isEmpty() && taglibtag->year() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear"),
                    taglibyear
                )
            );
        }
        const QString taglibtrack = QString::number(taglibtag->track());
        if (!taglibtrack.isEmpty() && taglibtag->track() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber"),
                    taglibtrack
                )
            );
        }
    }
    TagLib::AudioProperties *taglibaudio = taglibfile.audioProperties();
    if (!taglibaudio) {
        kDebug() << "Null audio properties for" << urlpath;
    } else {
        const QString tagliblength = KGlobal::locale()->formatTime(
            QTime().addSecs(taglibaudio->length()),
            true, true
        );
        if (!tagliblength.isEmpty() && taglibaudio->length() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration"),
                    tagliblength
                )
            );
        }
        const QString taglibbitrate = i18nc("kfilemetadata", "%1 kb/s", taglibaudio->bitrate());
        if (!taglibbitrate.isEmpty() && taglibaudio->bitrate() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate"),
                    taglibbitrate
                )
            );
        }
        const QString taglibsamplerate = i18nc("kfilemetadata", "%1 Hz", taglibaudio->sampleRate());
        if (!taglibsamplerate.isEmpty() && taglibaudio->sampleRate() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate"),
                    taglibsamplerate
                )
            );
        }
        const QString taglibchannels = QString::number(taglibaudio->channels());
        if (!taglibchannels.isEmpty() && taglibaudio->channels() > 0) {
            result.append(
                KFileMetaInfoItem(
                    QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels"),
                    taglibchannels
                )
            );
        }
    }
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataTagLibPluginFactory, registerPlugin<KFileMetaDataTagLibPlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataTagLibPluginFactory("kfilemetadata_taglib"))

#include "moc_kfilemetadata_taglib.cpp"
