/*****************************************************************************
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include <iostream>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kfilemetainfo.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kurl.h>

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QTimer>

using namespace std;

class KFileMetaDataReaderApplication : public QCoreApplication
{
    Q_OBJECT

public:
    KFileMetaDataReaderApplication(int& argc, char** argv);

private Q_SLOTS:
    void readAndSendMetaData();

private:
    void sendMetaData(const QHash<KUrl, QVariant>& data);
    QHash<KUrl, QVariant> readFileMetaData(const QList<KUrl>& urls) const;
    QHash<KUrl, QVariant> readFileAndContextMetaData(const QList<KUrl>& urls) const;
};



KFileMetaDataReaderApplication::KFileMetaDataReaderApplication(int& argc, char** argv) :
    QCoreApplication(argc, argv)
{
    QTimer::singleShot(0, this, SLOT(readAndSendMetaData()));
}

void KFileMetaDataReaderApplication::readAndSendMetaData()
{
    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUrl::List urls;
    for (int i = 0; i < args->count(); ++i) {
        urls.append(KUrl(args->arg(i)));
    }

    QHash<KUrl, QVariant> metaData;
    if (args->isSet("file")) {
        metaData = readFileMetaData(urls);
    } else {
        metaData = readFileAndContextMetaData(urls);
    }

    sendMetaData(metaData);

    quit();
}

void KFileMetaDataReaderApplication::sendMetaData(const QHash<KUrl, QVariant>& data)
{
    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::WriteOnly);

    QHashIterator<KUrl, QVariant> it(data);
    while (it.hasNext()) {
        it.next();

        out << it.key();
        out << it.value();
    }

    cout << byteArray.toBase64().constData();
}

QHash<KUrl, QVariant> KFileMetaDataReaderApplication::readFileMetaData(const QList<KUrl>& urls) const
{
    QHash<KUrl, QVariant> data;

    // Currently only the meta-data of one file is supported.
    // It might be an option to read all meta-data and show
    // ranges for each key.
    if (urls.count() == 1) {
        const QString path = urls.first().toLocalFile();
        KFileMetaInfo metaInfo(path, QString(), KFileMetaInfo::Fastest);
        const QHash<QString, KFileMetaInfoItem> metaInfoItems = metaInfo.items();
        foreach (const KFileMetaInfoItem& metaInfoItem, metaInfoItems) {
            const QString uriString = metaInfoItem.name();
            const QVariant value(metaInfoItem.value());
            // FIXME: Nepomuk::Utils::formatPropertyValue
            data.insert(uriString, value);
        }
    }

    return data;
}

QHash<KUrl, QVariant> KFileMetaDataReaderApplication::readFileAndContextMetaData(const QList<KUrl>& urls) const
{
    return readFileMetaData(urls);
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("kfilemetadatareader", "kio4", ki18n("KFileMetaDataReader"),
                         "1.0",
                         ki18n("KFileMetaDataReader can be used to read metadata from a file"),
                         KAboutData::License_GPL,
                         ki18n("(C) 2011, Peter Penz"));
    aboutData.addAuthor(ki18n("Peter Penz"), ki18n("Current maintainer"), "peter.penz19@gmail.com");
    KComponentData compData(&aboutData);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("file", ki18n("Only the meta data that is part of the file is read"));
    options.add("+[arg]", ki18n("List of URLs where the meta-data should be read from"));

    KCmdLineArgs::addCmdLineOptions(options);

    KFileMetaDataReaderApplication app(argc, argv);
    return app.exec();
}

#include "kfilemetadatareaderprocess.moc"
