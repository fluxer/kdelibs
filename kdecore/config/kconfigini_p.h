/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIGINI_P_H
#define KCONFIGINI_P_H

#include <kdecore_export.h>
#include <klockfile.h>
#include <kconfigdata.h>
#include <kconfigbase.h>
#include <kcomponentdata.h>
#include <QFile>
#include <QDateTime>

class KConfigIniBackend: public QObject, public QSharedData
{
    Q_OBJECT
    Q_FLAGS(ParseOption)
    Q_FLAGS(WriteOption)

private:
    KLockFile* m_lockfile;

public:
    class BufferFragment;

    KConfigIniBackend();
    ~KConfigIniBackend();

    /** Allows the behaviour of parseConfig() to be tuned */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4 /// entries are allowed to be marked as @em expandable
    };
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** Allows the behaviour of writeConfig() to be tuned */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig() */
    enum ParseInfo {
        ParseOk, /// the configuration was opened read/write
        ParseImmutable, /// the configuration is @em immutable
        ParseOpenError /// the configuration could not be opened
    };

    ParseInfo parseConfig(const QByteArray& locale,
                          KEntryMap& entryMap,
                          ParseOptions options);
    ParseInfo parseConfig(const QByteArray& locale,
                          KEntryMap& entryMap,
                          ParseOptions options,
                          bool merging);
    bool writeConfig(const QByteArray& locale, KEntryMap& entryMap,
                     WriteOptions options);

    bool isWritable() const;
    QString nonWritableErrorMessage() const;
    KConfigBase::AccessMode accessMode() const;
    void createEnclosing();
    void setFilePath(const QString& path);

    bool lock(const KComponentData& componentData);
    void unlock();
    bool isLocked() const;

    /** @return the absolute path to the object */
    QString filePath() const;
    /** @return the date and time when the object was last modified */
    QDateTime lastModified() const;
    /** @return the size of the object */
    qint64 size() const;

protected:
    void setLocalFilePath(const QString& file);
    void setLastModified(const QDateTime& dt);
    void setSize(qint64 sz);

    enum StringType {
        GroupString = 0,
        KeyString = 1,
        ValueString = 2
    };
    // Warning: this modifies data in-place. Other BufferFragment objects referencing the same buffer 
    // fragment will get their data modified too.
    static void printableToString(BufferFragment* aString, const QFile& file, int line);
    static QByteArray stringToPrintable(const QByteArray& aString, StringType type);
    static char charFromHex(const char *str, const QFile& file, int line);
    static QString warningProlog(const QFile& file, int line);

    void writeEntries(const QByteArray& locale, QFile& file, const KEntryMap& map);
    void writeEntries(const QByteArray& locale, QFile& file, const KEntryMap& map,
                      bool defaultGroup, bool &firstEntry);

private:
    QDateTime m_lastModified;
    qint64 m_size;
    QString m_localFileName;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIniBackend::ParseOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIniBackend::WriteOptions)

#endif // KCONFIGINI_P_H
