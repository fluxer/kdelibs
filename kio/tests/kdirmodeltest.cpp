/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kdirmodeltest.h"
#include <kdirnotify.h>
#include <kio/copyjob.h>
#include <kio/chmodjob.h>
#include <kprotocolinfo.h>
#include <kdirmodel.h>
#include <kdirlister.h>

#include <qtest_kde.h>

#include <kdebug.h>
#include <kio/deletejob.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kdirwatch.h>
#include "kiotesthelper.h"

QTEST_KDEMAIN(KDirModelTest, NoGUI)

Q_DECLARE_METATYPE(KFileItemList)

void KDirModelTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
    qRegisterMetaType<KFileItemList>("KFileItemList");

    m_tempDir = new KTempDir();
    m_dirModel = new KDirModel();

    qDebug() << Q_FUNC_INFO << m_tempDir->name();

    KDirLister* lister = m_dirModel->dirLister();
    lister->openUrl(KUrl(m_tempDir->name()));
    QTest::kWaitForSignal(lister, SIGNAL(completed()), 5000);
}

void KDirModelTest::cleanupTestCase()
{
    delete m_dirModel;
    m_dirModel = nullptr;
    delete m_tempDir;
    m_tempDir = nullptr;
}

void KDirModelTest::testItemForIndex()
{
    // root item
    KFileItem rootItem = m_dirModel->itemForIndex(QModelIndex());
    QVERIFY(!rootItem.isNull());
    QCOMPARE(rootItem.name(), QString("."));
}

void KDirModelTest::testIndexForItem()
{
    KFileItem rootItem = m_dirModel->itemForIndex(QModelIndex());
    QModelIndex rootIndex = m_dirModel->indexForItem(rootItem);
    QVERIFY(!rootIndex.isValid());
}

#include "moc_kdirmodeltest.cpp"
