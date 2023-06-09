 /* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
#include "browserextension.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/qmetaobject.h>
#include <QtCore/QRegExp>
#include <QtCore/QBitArray>
#include <QtGui/QTextDocument>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurifilter.h>
#include <kglobal.h>

using namespace KParts;


class OpenUrlEvent::OpenUrlEventPrivate
{
public:
  OpenUrlEventPrivate( ReadOnlyPart *part,
                       const KUrl &url,
                       const OpenUrlArguments &args,
                       const BrowserArguments &browserArgs )
    : m_part( part )
    , m_url( url )
    , m_args(args)
    , m_browserArgs(browserArgs)
  {
  }
  ~OpenUrlEventPrivate()
  {
  }
  static const char *s_strOpenUrlEvent;
  ReadOnlyPart *m_part;
  KUrl m_url;
  OpenUrlArguments m_args;
  BrowserArguments m_browserArgs;
};

const char *OpenUrlEvent::OpenUrlEventPrivate::s_strOpenUrlEvent =
                        "KParts/BrowserExtension/OpenURLevent";

OpenUrlEvent::OpenUrlEvent( ReadOnlyPart *part, const KUrl &url,
                            const OpenUrlArguments &args,
                            const BrowserArguments &browserArgs )
    : Event( OpenUrlEventPrivate::s_strOpenUrlEvent )
    , d( new OpenUrlEventPrivate(part, url, args, browserArgs) )
{
}

OpenUrlEvent::~OpenUrlEvent()
{
    delete d;
}

ReadOnlyPart *OpenUrlEvent::part() const
{
    return d->m_part;
}

KUrl OpenUrlEvent::url() const
{
    return d->m_url;
}

OpenUrlArguments OpenUrlEvent::arguments() const
{
    return d->m_args;
}

BrowserArguments OpenUrlEvent::browserArguments() const
{
    return d->m_browserArgs;
}

bool OpenUrlEvent::test( const QEvent *event )
{
    return Event::test( event, OpenUrlEventPrivate::s_strOpenUrlEvent );
}

namespace KParts
{

struct BrowserArgumentsPrivate
{
    BrowserArgumentsPrivate() {
      doPost = false;
      redirectedRequest = false;
      lockHistory = false;
      newTab = false;
      forcesNewWindow = false;
    }
    QString contentType; // for POST
    bool doPost;
    bool redirectedRequest;
    bool lockHistory;
    bool newTab;
    bool forcesNewWindow;
};

}

BrowserArguments::BrowserArguments()
{
  softReload = false;
  trustedSource = false;
  d = 0; // Let's build it on demand for now
}

BrowserArguments::BrowserArguments( const BrowserArguments &args )
{
  d = 0;
  (*this) = args;
}

BrowserArguments &BrowserArguments::operator=(const BrowserArguments &args)
{
  if (this == &args) return *this;

  delete d; d= 0;

  softReload = args.softReload;
  postData = args.postData;
  frameName = args.frameName;
  docState = args.docState;
  trustedSource = args.trustedSource;

  if ( args.d )
      d = new BrowserArgumentsPrivate( * args.d );

  return *this;
}

BrowserArguments::~BrowserArguments()
{
  delete d;
  d = 0;
}

void BrowserArguments::setContentType( const QString & contentType )
{
  if (!d)
    d = new BrowserArgumentsPrivate;
  d->contentType = contentType;
}

void BrowserArguments::setRedirectedRequest( bool redirected )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->redirectedRequest = redirected;
}

bool BrowserArguments::redirectedRequest () const
{
  return d ? d->redirectedRequest : false;
}

QString BrowserArguments::contentType() const
{
  return d ? d->contentType : QString();
}

void BrowserArguments::setDoPost( bool enable )
{
    if ( !d )
        d = new BrowserArgumentsPrivate;
    d->doPost = enable;
}

bool BrowserArguments::doPost() const
{
    return d ? d->doPost : false;
}

void BrowserArguments::setLockHistory( bool lock )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->lockHistory = lock;
}

bool BrowserArguments::lockHistory() const
{
    return d ? d->lockHistory : false;
}

void BrowserArguments::setNewTab( bool newTab )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->newTab = newTab;
}

bool BrowserArguments::newTab() const
{
    return d ? d->newTab : false;
}

void BrowserArguments::setForcesNewWindow( bool forcesNewWindow )
{
  if (!d)
     d = new BrowserArgumentsPrivate;
  d->forcesNewWindow = forcesNewWindow;
}

bool BrowserArguments::forcesNewWindow() const
{
    return d ? d->forcesNewWindow : false;
}

namespace KParts
{

class BrowserExtension::BrowserExtensionPrivate
{
public:
  BrowserExtensionPrivate( KParts::ReadOnlyPart *parent )
    : m_urlDropHandlingEnabled(false),
      m_part( parent )
  {}

  struct DelayedRequest {
    KUrl m_delayedURL;
    KParts::OpenUrlArguments m_delayedArgs;
    KParts::BrowserArguments m_delayedBrowserArgs;
  };

  QList<DelayedRequest> m_requests;
  bool m_urlDropHandlingEnabled;
  QBitArray m_actionStatus;
  QMap<int, QString> m_actionText;

  static void createActionSlotMap();

  KParts::ReadOnlyPart *m_part;
    OpenUrlArguments m_args;
    BrowserArguments m_browserArgs;
};

K_GLOBAL_STATIC(BrowserExtension::ActionSlotMap, s_actionSlotMap)
K_GLOBAL_STATIC(BrowserExtension::ActionNumberMap, s_actionNumberMap)

void BrowserExtension::BrowserExtensionPrivate::createActionSlotMap()
{
    s_actionSlotMap->insert( "cut", SLOT(cut()) );
    s_actionSlotMap->insert( "copy", SLOT(copy()) );
    s_actionSlotMap->insert( "paste", SLOT(paste()) );
    s_actionSlotMap->insert( "print", SLOT(print()) );
    // Tricky. Those aren't actions in fact, but simply methods that a browserextension
    // can have or not. No need to return them here.
    //s_actionSlotMap->insert( "reparseConfiguration", SLOT(reparseConfiguration()) );
    //s_actionSlotMap->insert( "refreshMimeTypes", SLOT(refreshMimeTypes()) );

    // Create the action-number map
    ActionSlotMap::ConstIterator it = s_actionSlotMap->constBegin();
    ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->constEnd();
    for ( int i=0 ; it != itEnd ; ++it, ++i )
    {
        // kDebug() << " action " << it.key() << " number " << i;
        s_actionNumberMap->insert( it.key(), i );
    }
}

}

BrowserExtension::BrowserExtension( KParts::ReadOnlyPart *parent )
: QObject( parent ), d( new BrowserExtensionPrivate(parent) )
{
  //kDebug() << "BrowserExtension::BrowserExtension() " << this;

  if (s_actionSlotMap->isEmpty())
      // Create the action-slot map
      BrowserExtensionPrivate::createActionSlotMap();

  // Build list with this extension's slot names.
  QList<QByteArray> slotNames;
  int methodCount = metaObject()->methodCount();
  int methodOffset = metaObject()->methodOffset();
  for ( int i=0 ; i < methodCount; ++i )
  {
      QMetaMethod method = metaObject()->method( methodOffset + i );
      if ( method.methodType() == QMetaMethod::Slot )
          slotNames.append( method.signature() );
  }

  // Set the initial status of the actions depending on whether
  // they're supported or not
  ActionSlotMap::ConstIterator it = s_actionSlotMap->constBegin();
  ActionSlotMap::ConstIterator itEnd = s_actionSlotMap->constEnd();
  for ( int i=0 ; it != itEnd ; ++it, ++i )
  {
      // Does the extension have a slot with the name of this action ?
      // ######### KDE4 TODO: use QMetaObject::indexOfMethod() #######
      d->m_actionStatus.setBit( i, slotNames.contains( it.key()+"()" ) );
  }

  connect( d->m_part, SIGNAL(completed()),
           this, SLOT(slotCompleted()) );
  connect( this, SIGNAL(openUrlRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
           this, SLOT(slotOpenUrlRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );
  connect( this, SIGNAL(enableAction(const char*,bool)),
           this, SLOT(slotEnableAction(const char*,bool)) );
  connect( this, SIGNAL(setActionText(const char*,QString)),
           this, SLOT(slotSetActionText(const char*,QString)) );
}

BrowserExtension::~BrowserExtension()
{
  //kDebug() << "BrowserExtension::~BrowserExtension() " << this;
  delete d;
}

void BrowserExtension::setBrowserArguments( const BrowserArguments &args )
{
  d->m_browserArgs = args;
}

BrowserArguments BrowserExtension::browserArguments() const
{
  return d->m_browserArgs;
}

int BrowserExtension::xOffset()
{
  return 0;
}

int BrowserExtension::yOffset()
{
  return 0;
}

void BrowserExtension::saveState( QDataStream &stream )
{
    // TODO add d->m_part->mimeType()
  stream << d->m_part->url() << (qint32)xOffset() << (qint32)yOffset();
}

void BrowserExtension::restoreState( QDataStream &stream )
{
  KUrl u;
  qint32 xOfs, yOfs;
  stream >> u >> xOfs >> yOfs;

    OpenUrlArguments args;
    args.setXOffset(xOfs);
    args.setYOffset(yOfs);
    // TODO add args.setMimeType
    d->m_part->setArguments(args);
    d->m_part->openUrl(u);
}

bool BrowserExtension::isURLDropHandlingEnabled() const
{
    return d->m_urlDropHandlingEnabled;
}

void BrowserExtension::setURLDropHandlingEnabled( bool enable )
{
    d->m_urlDropHandlingEnabled = enable;
}

void BrowserExtension::slotCompleted()
{
  //empty the argument stuff, to avoid bogus/invalid values when opening a new url
    setBrowserArguments( BrowserArguments() );
}

void BrowserExtension::pasteRequest()
{
    QString plain( "plain" );
    QString url = QApplication::clipboard()->text(plain, QClipboard::Selection).trimmed();
    // Remove linefeeds and any whitespace surrounding it.
    url.remove(QRegExp("[\\ ]*\\n+[\\ ]*"));

    // Check if it's a URL
    QStringList filters = KUriFilter::self()->pluginNames();
    filters.removeAll( "kuriikwsfilter" );
    filters.removeAll( "localdomainurifilter" );
    KUriFilterData filterData;
    filterData.setData( url );
    filterData.setCheckForExecutables( false );
    if ( KUriFilter::self()->filterUri( filterData, filters ) )
    {
        switch ( filterData.uriType() )
	{
	    case KUriFilterData::LocalFile:
	    case KUriFilterData::LocalDir:
	    case KUriFilterData::NetProtocol:
	        slotOpenUrlRequest( filterData.uri() );
		break;
	    case KUriFilterData::Error:
		KMessageBox::sorry( d->m_part->widget(), filterData.errorMsg() );
		break;
	    default:
		break;
	}
    }
    else if ( KUriFilter::self()->filterUri( filterData,
                    QStringList( QLatin1String( "kuriikwsfilter" ) ) ) &&
              url.length() < 250 )
    {
        if ( KMessageBox::questionYesNo( d->m_part->widget(),
		    i18n( "<qt>Do you want to search the Internet for <b>%1</b>?</qt>" ,  Qt::escape(url) ),
		    i18n( "Internet Search" ), KGuiItem( i18n( "&Search" ), "edit-find"),
		    KStandardGuiItem::cancel(), "MiddleClickSearch" ) == KMessageBox::Yes)
          slotOpenUrlRequest( filterData.uri() );
    }
}

void BrowserExtension::slotOpenUrlRequest( const KUrl &url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments &browserArgs )
{
    //kDebug() << this << " BrowserExtension::slotOpenURLRequest(): url=" << url.url();
    BrowserExtensionPrivate::DelayedRequest req;
    req.m_delayedURL = url;
    req.m_delayedArgs = args;
    req.m_delayedBrowserArgs = browserArgs;
    d->m_requests.append( req );
    QTimer::singleShot( 0, this, SLOT(slotEmitOpenUrlRequestDelayed()) );
}

void BrowserExtension::slotEmitOpenUrlRequestDelayed()
{
    if (d->m_requests.isEmpty()) return;
    BrowserExtensionPrivate::DelayedRequest req = d->m_requests.front();
    d->m_requests.pop_front();
    emit openUrlRequestDelayed( req.m_delayedURL, req.m_delayedArgs, req.m_delayedBrowserArgs );
    // tricky: do not do anything here! (no access to member variables, etc.)
}

void BrowserExtension::slotEnableAction( const char * name, bool enabled )
{
    //kDebug() << "BrowserExtension::slotEnableAction " << name << " " << enabled;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->constFind( name );
    if ( it != s_actionNumberMap->constEnd() )
    {
        d->m_actionStatus.setBit( it.value(), enabled );
        //kDebug() << "BrowserExtension::slotEnableAction setting bit " << it.data() << " to " << enabled;
    }
    else
        kWarning() << "BrowserExtension::slotEnableAction unknown action " << name;
}

bool BrowserExtension::isActionEnabled( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    return d->m_actionStatus[ actionNumber ];
}

void BrowserExtension::slotSetActionText( const char * name, const QString& text )
{
    //kDebug() << "BrowserExtension::slotSetActionText " << name << " " << text;
    ActionNumberMap::ConstIterator it = s_actionNumberMap->constFind( name );
    if ( it != s_actionNumberMap->constEnd() )
    {
        d->m_actionText[ it.value() ] = text;
    }
    else
        kWarning() << "BrowserExtension::slotSetActionText unknown action " << name;
}

QString BrowserExtension::actionText( const char * name ) const
{
    int actionNumber = (*s_actionNumberMap)[ name ];
    QMap<int, QString>::ConstIterator it = d->m_actionText.constFind( actionNumber );
    if ( it != d->m_actionText.constEnd() )
        return *it;
    return QString();
}

// for compatibility
BrowserExtension::ActionSlotMap BrowserExtension::actionSlotMap()
{
    return *actionSlotMapPtr();
}

BrowserExtension::ActionSlotMap * BrowserExtension::actionSlotMapPtr()
{
    if (s_actionSlotMap->isEmpty())
        BrowserExtensionPrivate::createActionSlotMap();
    return s_actionSlotMap;
}

BrowserExtension *BrowserExtension::childObject( QObject *obj )
{
    return KGlobal::findDirectChild<KParts::BrowserExtension *>(obj);
}

#include "moc_browserextension.cpp"
