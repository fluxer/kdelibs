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

#include "kfilemetadata_poppler.h"
#include "kpluginfactory.h"
#include "kglobal.h"
#include "klocale.h"
#include "kdatetime.h"
#include "kdebug.h"

#include <QDateTime>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-version.h>
#include <sys/types.h>

#if POPPLER_VERSION_MAJOR >= 22 && POPPLER_VERSION_MINOR >= 5
typedef time_t popplertimetype;
#else
typedef poppler::time_type popplertimetype;
#endif

static QString getString(const poppler::ustring &popplerstring)
{
    const poppler::byte_array popplerbytes = popplerstring.to_utf8();
    return QString::fromUtf8(popplerbytes.data(), popplerbytes.size());
}

static QString getTime(const popplertimetype &popplertime)
{
    const KDateTime kdatetime(QDateTime::fromTime_t(popplertime));
    return KGlobal::locale()->formatDateTime(kdatetime, KLocale::FancyLongDate);
}

KFileMetaDataPopplerPlugin::KFileMetaDataPopplerPlugin(QObject* parent, const QVariantList &args)
    : KFileMetaDataPlugin(parent)
{
    Q_UNUSED(args);
}

KFileMetaDataPopplerPlugin::~KFileMetaDataPopplerPlugin()
{
}

QList<KFileMetaInfoItem> KFileMetaDataPopplerPlugin::metaData(const KUrl &url)
{
    QList<KFileMetaInfoItem> result;
    const QByteArray urlpath = url.toLocalFile().toLocal8Bit();
    poppler::document *popplerdocument = poppler::document::load_from_file(std::string(urlpath.constData(), urlpath.size()));
    if (!popplerdocument) {
        kWarning() << "Could not open" << urlpath;
        return result;
    }
    const QString popplertitle = getString(popplerdocument->get_title());
    if (!popplertitle.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title"),
                popplertitle
            )
        );
    }
    const QString popplerauthor = getString(popplerdocument->get_author());
    if (!popplerauthor.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter"),
                popplerauthor
            )
        );
    }
    const QString popplersubject = getString(popplerdocument->get_subject());
    if (!popplersubject.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject"),
                popplersubject
            )
        );
    }
    const QString popplerkeywords = getString(popplerdocument->get_keywords());
    if (!popplerkeywords.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword"),
                popplerkeywords
            )
        );
    }
    const QString popplercreator = getString(popplerdocument->get_creator());
    if (!popplercreator.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator"),
                popplercreator
            )
        );
    }
    const QString popplerproducer = getString(popplerdocument->get_producer());
    if (!popplerproducer.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator"),
                popplerproducer
            )
        );
    }
#if POPPLER_VERSION_MAJOR >= 22 && POPPLER_VERSION_MINOR >= 5
    const QString popplercreationdate = getTime(popplerdocument->get_creation_date_t());
#else
    const QString popplercreationdate = getTime(popplerdocument->get_creation_date());
#endif
    if (!popplercreationdate.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated"),
                popplercreationdate
            )
        );
    }
#if POPPLER_VERSION_MAJOR >= 22 && POPPLER_VERSION_MINOR >= 5
    const QString popplermodificationdate = getTime(popplerdocument->get_modification_date_t());
#else
    const QString popplermodificationdate = getTime(popplerdocument->get_modification_date());
#endif
    if (!popplermodificationdate.isEmpty()) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified"),
                popplermodificationdate
            )
        );
    }
    const int popplerpages = popplerdocument->pages();
    if (popplerpages > 0) {
        result.append(
            KFileMetaInfoItem(
                QString::fromLatin1("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount"),
                QString::number(popplerpages)
            )
        );
    }
    delete popplerdocument;
    return result;
}

K_PLUGIN_FACTORY(KFileMetaDataPopplerPluginFactory, registerPlugin<KFileMetaDataPopplerPlugin>();)
K_EXPORT_PLUGIN(KFileMetaDataPopplerPluginFactory("kfilemetadata_poppler"))

#include "moc_kfilemetadata_poppler.cpp"
