/*  This file is part of the KDE libraries
 *  Copyright 2007, 2010 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kmimeglobsfileparser_p.h"
#include <kglobal.h>
#include <kdeversion.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include "kmimetyperepository_p.h"
#include <kdebug.h>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

KMimeGlobsFileParser::KMimeGlobsFileParser()
{
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobs()
{
    const QStringList globFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", QString::fromLatin1("globs"));
    //kDebug() << globFiles;
    return parseGlobs(globFiles);
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobs(const QStringList& globFiles)
{
    QStringList parsedFiles;
    return parseGlobFiles(globFiles, parsedFiles);
}

KMimeGlobsFileParser::AllGlobs KMimeGlobsFileParser::parseGlobFiles(const QStringList& globFiles, QStringList& parsedFiles)
{
    KMimeGlobsFileParser::AllGlobs allGlobs;
    QListIterator<QString> globIter(globFiles);
    globIter.toBack();
    // At each level, we must be able to override (not just add to) the information that we read at higher levels
    // (if glob-deleteall is used).
    while (globIter.hasPrevious()) { // global first, then local
        Format format = OldGlobs;
        QString fileName = globIter.previous();
        QString fileNamev2 = fileName + QLatin1Char('2'); // NOTE: this relies on u-m-d always generating the old globs file
        if (QFile::exists(fileNamev2)) {
            fileName = fileNamev2;
            format = Globs2WithWeight;
        }
        parsedFiles << fileName;
        QFile globFile(fileName);
        //kDebug() << "Now parsing" << fileName;
        parseGlobFile(&globFile, format, allGlobs);
    }
    return allGlobs;
}

static void filterEmptyFromList(QList<QByteArray>* bytelist)
{
    QList<QByteArray>::iterator fieldsit = bytelist->begin();
    while (fieldsit != bytelist->end()) {
        if (fieldsit->isEmpty()) {
            fieldsit = bytelist->erase(fieldsit);
        } else {
            fieldsit++;
        }
    }
}

// uses a QIODevice to make unit tests possible
bool KMimeGlobsFileParser::parseGlobFile(QIODevice* file, Format format, AllGlobs& globs)
{
    Q_ASSERT(file);
    if (!file->open(QIODevice::ReadOnly)) {
        return false;
    }

    // for reference:
    // https://specifications.freedesktop.org/shared-mime-info-spec/latest/ar01s02.html
    // NOTE: the file is supposed to be in UTF-8 encoding however in practise no mime-type entry
    // contains non-latin1 characters
    QByteArray lastMime, lastPattern;
    QByteArray line;
    while (!file->atEnd()) {
        line = file->readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QList<QByteArray> fields = line.split(':');
        filterEmptyFromList(&fields);
        if (fields.count() < 2) // syntax error
            continue;

        //kDebug() << "line=" << line;

        QByteArray mimeTypeName, pattern;
        QList<QByteArray> flagList;
        int weight = 50;
        if (format == Globs2WithWeight) {
            if (fields.count() < 3) // syntax error
                continue;
            weight = fields.at(0).toInt();
            mimeTypeName = fields.at(1);
            pattern = fields.at(2);
            const QByteArray flagsStr = fields.value(3); // could be empty
            flagList = flagsStr.split(',');
            filterEmptyFromList(&flagList);
        } else {
            mimeTypeName = fields.at(0);
            pattern = fields.at(1);
        }
        Q_ASSERT(!pattern.isEmpty());
        Q_ASSERT(!pattern.contains(':'));

        //kDebug() << " got:" << mimeTypeName << pattern;

        if (lastMime == mimeTypeName && lastPattern == pattern) {
            // Ignore duplicates, especially important for those with no flags after a line with flags:
            // 50:text/x-csrc:*.c:cs
            // 50:text/x-csrc:*.c
            continue;
        }

        bool caseSensitive = flagList.contains(QByteArray("cs"));

        const QString mimeTypeNameStr = QString::fromLatin1(mimeTypeName.constData(), mimeTypeName.size());
        if (pattern == "__NOGLOBS__") {
            //kDebug() << "removing" << mimeTypeName;
            globs.removeMime(mimeTypeNameStr);
            lastMime.clear();
        } else {
            //if (mimeTypeName == "text/plain")
            //    kDebug() << "Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file, with weight" << weight;
            //if (pattern.toLower() == "*.c")
            //    kDebug() << " Adding pattern" << pattern << "to mimetype" << mimeTypeName << "from globs file, with weight" << weight << "flags" << flags;
            const QString patternStr = QString::fromLatin1(pattern.constData(), pattern.size());
            globs.addGlob(Glob(mimeTypeNameStr, weight, patternStr, caseSensitive));
            lastMime = mimeTypeName;
            lastPattern = pattern;
        }
    }
    return true;
}

void KMimeGlobsFileParser::AllGlobs::addGlob(const Glob& glob)
{
    // Note that in each case, we check for duplicates to avoid inserting duplicated patterns.
    // This can happen when installing kde.xml and freedesktop.org.xml
    // in the same prefix, and they both have text/plain:*.txt

    const QString &pattern = glob.pattern;
    Q_ASSERT(!pattern.isEmpty());
    Q_UNUSED(pattern);

    //kDebug() << "pattern" << pattern << "glob.weight=" << glob.weight << glob.flags;

    // Store each patterns into either m_fastPatternDict (*.txt, *.html etc. with default weight 50)
    // or for the rest, like core.*, *.tar.bz2, *~, into highWeightPatternOffset (>50)
    // or lowWeightPatternOffset (<=50)

    Glob adjustedGlob(glob);
    if (!adjustedGlob.casesensitive)
        adjustedGlob.pattern = adjustedGlob.pattern.toLower();
    if (adjustedGlob.weight >= 50) {
        if (!m_highWeightGlobs.hasPattern(adjustedGlob.mimeType, adjustedGlob.pattern))
            m_highWeightGlobs.append(adjustedGlob);
    } else {
        if (!m_lowWeightGlobs.hasPattern(adjustedGlob.mimeType, adjustedGlob.pattern))
            m_lowWeightGlobs.append(adjustedGlob);
    }
}

KMimeGlobsFileParser::PatternsMap KMimeGlobsFileParser::AllGlobs::patternsMap() const
{
    PatternsMap patMap;
    patMap.reserve(m_highWeightGlobs.size() + m_lowWeightGlobs.size());

    // This is just to fill in KMimeType::patterns. This has no real effect
    // on the actual mimetype matching.

    Q_FOREACH(const Glob& glob, m_highWeightGlobs)
        patMap[glob.mimeType].append(glob.pattern);

    Q_FOREACH(const Glob& glob, m_lowWeightGlobs)
        patMap[glob.mimeType].append(glob.pattern);

    return patMap;
}

void KMimeGlobsFileParser::AllGlobs::removeMime(const QString& mime)
{
    m_highWeightGlobs.removeMime(mime);
    m_lowWeightGlobs.removeMime(mime);
}
