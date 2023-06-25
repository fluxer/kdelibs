/*
 *  Copyright (C) 2004 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
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

#ifndef KURLCOMPLETIONTEST_H
#define KURLCOMPLETIONTEST_H

#include <QObject>
#include <kurlcompletion.h>
#include <ktempdir.h>

class KUrlCompletionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();


    void testLocalRelativePath();
    void testLocalAbsolutePath();
    void testLocalURL();
    void testEmptyCwd();

private:
    void waitForCompletion();

    KUrlCompletion* m_completion;
    KTempDir* m_tempDir;
    KUrl m_dirURL;
    QString m_dir;
    KUrlCompletion* m_completionEmptyCwd;
};

#endif // KURLCOMPLETIONTEST_H
