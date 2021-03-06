/* This file is part of the KDE desktop environment

   Copyright (C) 2001, 2002 Michael Brade <brade@kde.org>

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

#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QDir>

#include <kapplication.h>
#include <kdirlister.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include "kdirlistertest_gui.h"

#include <cstdlib>


KDirListerTest::KDirListerTest( QWidget *parent )
  : QWidget( parent )
{
  lister = new KDirLister(this);
  debug = new PrintSignals;

  QVBoxLayout* layout = new QVBoxLayout( this );

  QPushButton* startH = new QPushButton( "Start listing Home", this );
  QPushButton* startR= new QPushButton( "Start listing Root", this );
  QPushButton* test = new QPushButton( "Many", this );
  QPushButton* startT = new QPushButton( "tarfile", this );

  layout->addWidget( startH );
  layout->addWidget( startR );
  layout->addWidget( startT );
  layout->addWidget( test );
  resize( layout->sizeHint() );

  connect( startR, SIGNAL(clicked()), SLOT(startRoot()) );
  connect( startH, SIGNAL(clicked()), SLOT(startHome()) );
  connect( startT, SIGNAL(clicked()), SLOT(startTar()) );
  connect( test, SIGNAL(clicked()), SLOT(test()) );

  connect( lister, SIGNAL(started(KUrl)),
           debug,  SLOT(started(KUrl)) );
  connect( lister, SIGNAL(completed()),
           debug,  SLOT(completed()) );
  connect( lister, SIGNAL(completed(KUrl)),
           debug,  SLOT(completed(KUrl)) );
  connect( lister, SIGNAL(canceled()),
           debug,  SLOT(canceled()) );
  connect( lister, SIGNAL(canceled(KUrl)),
           debug,  SLOT(canceled(KUrl)) );
  connect( lister, SIGNAL(redirection(KUrl)),
           debug,  SLOT(redirection(KUrl)) );
  connect( lister, SIGNAL(redirection(KUrl,KUrl)),
           debug,  SLOT(redirection(KUrl,KUrl)) );
  connect( lister, SIGNAL(clear()),
           debug,  SLOT(clear()) );
  connect( lister, SIGNAL(newItems(KFileItemList)),
           debug,  SLOT(newItems(KFileItemList)) );
  connect( lister, SIGNAL(itemsFilteredByMime(KFileItemList)),
           debug,  SLOT(itemsFilteredByMime(KFileItemList)) );
  connect( lister, SIGNAL(itemsDeleted(KFileItemList)),
           debug,  SLOT(itemsDeleted(KFileItemList)) );
  connect( lister, SIGNAL(refreshItems(QList<QPair<KFileItem,KFileItem> >)),
           debug,  SLOT(refreshItems(QList<QPair<KFileItem,KFileItem> >)) );
  connect( lister, SIGNAL(infoMessage(QString)),
           debug,  SLOT(infoMessage(QString)) );
  connect( lister, SIGNAL(percent(int)),
           debug,  SLOT(percent(int)) );
  connect( lister, SIGNAL(totalSize(KIO::filesize_t)),
           debug,  SLOT(totalSize(KIO::filesize_t)) );
  connect( lister, SIGNAL(processedSize(KIO::filesize_t)),
           debug,  SLOT(processedSize(KIO::filesize_t)) );
  connect( lister, SIGNAL(speed(int)),
           debug,  SLOT(speed(int)) );

  connect( lister, SIGNAL(completed()),
           this,  SLOT(completed()) );
}

KDirListerTest::~KDirListerTest()
{
}

void KDirListerTest::startHome()
{
  KUrl home( QDir::homePath() );
  lister->openUrl( home, KDirLister::NoFlags );
//  lister->stop();
}

void KDirListerTest::startRoot()
{
  KUrl root( QDir::rootPath() );
  lister->openUrl( root, KDirLister::Keep | KDirLister::Reload );
// lister->stop( root );
}

void KDirListerTest::startTar()
{
  KUrl root( QDir::homePath()+"/aclocal_1.tgz" );
  lister->openUrl( root, KDirLister::Keep | KDirLister::Reload );
// lister->stop( root );
}

void KDirListerTest::test()
{
  KUrl home( QDir::homePath() );
  KUrl root( QDir::rootPath() );
/*  lister->openUrl( home, KDirLister::Keep );
  lister->openUrl( root, KDirLister::Keep | KDirLister::Reload );
  lister->openUrl( KUrl("file:/etc"), KDirLister::Keep | KDirLister::Reload );
  lister->openUrl( root, KDirLister::Keep | KDirLister::Reload );
  lister->openUrl( KUrl("file:/dev"), KDirLister::Keep | KDirLister::Reload );
  lister->openUrl( KUrl("file:/tmp"), KDirLister::Keep | KDirLister::Reload );
  lister->openUrl( KUrl("file:/usr/include"), KDirLister::Keep | KDirLister::Reload );
  lister->updateDirectory( KUrl("file:/usr/include") );
  lister->updateDirectory( KUrl("file:/usr/include") );
  lister->openUrl( KUrl("file:/usr/"), KDirLister::Keep | KDirLister::Reload );
*/
  lister->openUrl( KUrl("file:/dev"), KDirLister::Keep | KDirLister::Reload );
}

void KDirListerTest::completed()
{
    if ( lister->url().toLocalFile() == QDir::rootPath() )
    {
        const KFileItem item = lister->findByUrl( KUrl( QDir::tempPath() ) );
        if ( !item.isNull() )
            kDebug() << "Found " << QDir::tempPath() << ": " << item.name();
        else
            kWarning() << QDir::tempPath() << " not found! Bug in findByURL?";
    }
}

int main ( int argc, char *argv[] )
{
  KCmdLineArgs::init( argc, argv, "kdirlistertest", 0, ki18n("kdirlistertest"), 0);
  KApplication app;

  KDirListerTest *test = new KDirListerTest( 0 );
  test->show();
  return app.exec();
}

#include "moc_kdirlistertest_gui.cpp"
