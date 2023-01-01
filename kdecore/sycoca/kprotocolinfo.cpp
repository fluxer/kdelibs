/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kprotocolinfo.h"
#include "kprotocolinfo_p.h"
#include "kprotocolinfofactory.h"

#include <kmimetypetrader.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kconfiggroup.h>

//
// Internal functions:
//
KProtocolInfo::KProtocolInfo(const QString &path)
    : KSycocaEntry(*new KProtocolInfoPrivate(path, this))
{
    Q_D(KProtocolInfo);
    QString fullPath = KStandardDirs::locate("services", path);

    KConfig sconfig( fullPath );
    KConfigGroup config(&sconfig, "Protocol" );

    m_name = config.readEntry( "protocol" );
    m_exec = config.readPathEntry( "exec", QString() );
    m_isSourceProtocol = config.readEntry( "source", true );
    m_isHelperProtocol = config.readEntry( "helper", false );
    m_supportsListing = config.readEntry( "listing", false );
    m_supportsReading = config.readEntry( "reading", false );
    m_supportsWriting = config.readEntry( "writing", false );
    m_supportsMakeDir = config.readEntry( "makedir", false );
    m_supportsDeleting = config.readEntry( "deleting", false );
    m_supportsLinking = config.readEntry( "linking", false );
    m_supportsMoving = config.readEntry( "moving", false );
    m_supportsOpening = config.readEntry( "opening", false );
    m_canCopyFromFile = config.readEntry( "copyFromFile", false );
    m_canCopyToFile = config.readEntry( "copyToFile", false );
    m_defaultMimetype = config.readEntry( "defaultMimetype" );
    m_determineMimetypeFromExtension = config.readEntry( "determineMimetypeFromExtension", true );
    m_icon = config.readEntry( "Icon" );
    m_config = config.readEntry( "config", m_name );
    m_maxSlaves = config.readEntry( "maxInstances", 1);

    d->canRenameFromFile = config.readEntry( "renameFromFile", false );
    d->canRenameToFile = config.readEntry( "renameToFile", false );
    d->canDeleteRecursive = config.readEntry( "deleteRecursive", false );
    const QString fnu = config.readEntry( "fileNameUsedForCopying", "FromURL" );
    d->fileNameUsedForCopying = FromUrl;
    if (fnu == QLatin1String("Name")) {
        d->fileNameUsedForCopying = Name;
    } else if (fnu == QLatin1String("DisplayName")) {
        d->fileNameUsedForCopying = DisplayName;
    }
    d->maxSlavesPerHost = config.readEntry( "maxInstancesPerHost", 0);
    d->docPath = config.readPathEntry( "X-DocPath", QString() );
    d->protClass = config.readEntry( "Class" ).toLower();
    if (d->protClass[0] != QLatin1Char(':')) {
        d->protClass.prepend(QLatin1Char(':'));
    }
    d->showPreviews = config.readEntry( "ShowPreviews", d->protClass == QLatin1String(":local") );
    d->proxyProtocol = config.readEntry( "ProxiedBy" );
}

KProtocolInfo::KProtocolInfo( QDataStream& _str, int offset)
    : KSycocaEntry(*new KProtocolInfoPrivate( _str, offset, this) )
{
    load( _str );
}

KProtocolInfo::~KProtocolInfo()
{
}

void KProtocolInfo::load( QDataStream& _str)
{
    Q_D(KProtocolInfo);
    // NOTE: make sure to update the version number in ksycoca.cpp
    qint8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_supportsOpening,
          i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile, i_showPreviews,
          i_canRenameFromFile, i_canRenameToFile,
          i_canDeleteRecursive, i_fileNameUsedForCopying;

    _str >> m_name >> m_exec >> m_defaultMimetype
         >> i_determineMimetypeFromExtension
         >> m_icon
         >> i_isSourceProtocol >> i_isHelperProtocol
         >> i_supportsListing >> i_supportsReading
         >> i_supportsWriting >> i_supportsMakeDir
         >> i_supportsDeleting >> i_supportsLinking
         >> i_supportsMoving >> i_supportsOpening
         >> i_canCopyFromFile >> i_canCopyToFile
         >> m_config >> m_maxSlaves >> d->docPath >> d->protClass
         >> i_showPreviews
         >> d->proxyProtocol
         >> i_canRenameFromFile >> i_canRenameToFile
         >> i_canDeleteRecursive >> i_fileNameUsedForCopying
         >> d->maxSlavesPerHost;

    m_isSourceProtocol = (i_isSourceProtocol != 0);
    m_isHelperProtocol = (i_isHelperProtocol != 0);
    m_supportsListing = (i_supportsListing != 0);
    m_supportsReading = (i_supportsReading != 0);
    m_supportsWriting = (i_supportsWriting != 0);
    m_supportsMakeDir = (i_supportsMakeDir != 0);
    m_supportsDeleting = (i_supportsDeleting != 0);
    m_supportsLinking = (i_supportsLinking != 0);
    m_supportsMoving = (i_supportsMoving != 0);
    m_supportsOpening = (i_supportsOpening != 0);
    m_canCopyFromFile = (i_canCopyFromFile != 0);
    m_canCopyToFile = (i_canCopyToFile != 0);
    d->canRenameFromFile = (i_canRenameFromFile != 0);
    d->canRenameToFile = (i_canRenameToFile != 0);
    d->canDeleteRecursive = (i_canDeleteRecursive != 0);
    d->fileNameUsedForCopying = FileNameUsedForCopying(i_fileNameUsedForCopying);
    m_determineMimetypeFromExtension = (i_determineMimetypeFromExtension != 0);
    d->showPreviews = (i_showPreviews != 0);
}

void
KProtocolInfoPrivate::save( QDataStream& _str)
{
    KSycocaEntryPrivate::save( _str );

    // NOTE: make sure to update the version number in ksycoca.cpp
    qint8 i_isSourceProtocol, i_isHelperProtocol,
          i_supportsListing, i_supportsReading,
          i_supportsWriting, i_supportsMakeDir,
          i_supportsDeleting, i_supportsLinking,
          i_supportsMoving, i_supportsOpening,
          i_determineMimetypeFromExtension,
          i_canCopyFromFile, i_canCopyToFile, i_showPreviews,
          i_canRenameFromFile, i_canRenameToFile,
          i_canDeleteRecursive, i_fileNameUsedForCopying;

    i_isSourceProtocol = q->m_isSourceProtocol ? 1 : 0;
    i_isHelperProtocol = q->m_isHelperProtocol ? 1 : 0;
    i_supportsListing = q->m_supportsListing ? 1 : 0;
    i_supportsReading = q->m_supportsReading ? 1 : 0;
    i_supportsWriting = q->m_supportsWriting ? 1 : 0;
    i_supportsMakeDir = q->m_supportsMakeDir ? 1 : 0;
    i_supportsDeleting = q->m_supportsDeleting ? 1 : 0;
    i_supportsLinking = q->m_supportsLinking ? 1 : 0;
    i_supportsMoving = q->m_supportsMoving ? 1 : 0;
    i_supportsOpening = q->m_supportsOpening ? 1 : 0;
    i_canCopyFromFile = q->m_canCopyFromFile ? 1 : 0;
    i_canCopyToFile = q->m_canCopyToFile ? 1 : 0;
    i_canRenameFromFile = canRenameFromFile ? 1 : 0;
    i_canRenameToFile = canRenameToFile ? 1 : 0;
    i_canDeleteRecursive = canDeleteRecursive ? 1 : 0;
    i_fileNameUsedForCopying = int(fileNameUsedForCopying);
    i_determineMimetypeFromExtension = q->m_determineMimetypeFromExtension ? 1 : 0;
    i_showPreviews = showPreviews ? 1 : 0;

    _str << q->m_name << q->m_exec << q->m_defaultMimetype
         << i_determineMimetypeFromExtension
         << q->m_icon
         << i_isSourceProtocol << i_isHelperProtocol
         << i_supportsListing << i_supportsReading
         << i_supportsWriting << i_supportsMakeDir
         << i_supportsDeleting << i_supportsLinking
         << i_supportsMoving << i_supportsOpening
         << i_canCopyFromFile << i_canCopyToFile
         << q->m_config << q->m_maxSlaves << docPath << protClass
         << i_showPreviews
         << proxyProtocol
         << i_canRenameFromFile << i_canRenameToFile
         << i_canDeleteRecursive << i_fileNameUsedForCopying
         << maxSlavesPerHost;
}

//
// Static functions:
//
QStringList KProtocolInfo::protocols()
{
    return KProtocolInfoFactory::self()->protocols();
}

bool KProtocolInfo::isFilterProtocol( const QString& _protocol )
{
    // call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return false;
    }
    return !prot->m_isSourceProtocol;
}

void KProtocolInfo::selectServiceOrHelper(const QString& protocol, KProtocolInfo::Ptr& returnProtocol, KService::Ptr& returnService)
{
    // have up to two sources of data:
    // 1) the exec line of the .protocol file, if there's one (could be a kioslave or a helper app)
    // 2) the application associated with x-scheme-handler/<protocol> if there's one

    // if both exist, then:
    //  A) if the .protocol file says "launch an application", then the new-style handler-app has priority
    //  B) but if the .protocol file is for a kioslave (e.g. kio_http) then this has priority over
    //     firefox or chromium saying x-scheme-handler/http. Gnome people want to send all HTTP urls
    //     to a webbrowser, but we want mimetype-determination-in-calling-application by default
    //     (the user can configure a BrowserApplication though)

    const KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
    const KService::Ptr service = KMimeTypeTrader::self()->preferredService(QString::fromLatin1("x-scheme-handler/") + protocol);
    if (service && prot && prot->m_isHelperProtocol) {
        // for helper protocols, the handler app has priority over the hardcoded one (see A above)
        returnService = service;
        return;
    }
    if (prot) {
        returnProtocol = prot;
    } else {
        // no protocol file, use handler app if any
        returnService = service;
    }
}

QString KProtocolInfo::icon(const QString& protocol)
{
    KProtocolInfo::Ptr prot;
    KService::Ptr service;
    selectServiceOrHelper(protocol, prot, service);
    if (service) {
        return service->icon();
    } else if (prot) {
        return prot->m_icon;
    }
    return QString();
}

QString KProtocolInfo::config( const QString& _protocol )
{
    // call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return QString();
    }
    return QString::fromLatin1("kio_%1rc").arg(prot->m_config);
}

int KProtocolInfo::maxSlaves( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return 1;
    }
    return prot->m_maxSlaves;
}

int KProtocolInfo::maxSlavesPerHost( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return 0;
    }
    return prot->d_func()->maxSlavesPerHost;
}

bool KProtocolInfo::determineMimetypeFromExtension( const QString &_protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _protocol );
    if ( !prot ) {
        return true;
    }
    return prot->m_determineMimetypeFromExtension;
}

QString KProtocolInfo::exec(const QString& protocol)
{
    KProtocolInfo::Ptr prot;
    KService::Ptr service;
    selectServiceOrHelper(protocol, prot, service);
    if (service) {
        return service->exec();
    } else if (prot) {
        return prot->m_exec;
    }
    return QString();
}

QString KProtocolInfo::docPath( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return QString();
    }
    return prot->d_func()->docPath;
}

QString KProtocolInfo::protocolClass( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return QString();
    }
    return prot->d_func()->protClass;
}

bool KProtocolInfo::showFilePreview( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return false;
    }
    return prot->d_func()->showPreviews;
}

QString KProtocolInfo::proxiedBy( const QString& _protocol )
{
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(_protocol);
    if ( !prot ) {
        return QString();
    }
    return prot->d_func()->proxyProtocol;
}

QString KProtocolInfo::defaultMimeType() const
{
    return m_defaultMimetype;
}

bool KProtocolInfo::supportsListing() const
{
    return m_supportsListing;
}

bool KProtocolInfo::canRenameFromFile() const
{
    Q_D(const KProtocolInfo);
    return d->canRenameFromFile;
}

bool KProtocolInfo::canRenameToFile() const
{
    Q_D(const KProtocolInfo);
    return d->canRenameToFile;
}

bool KProtocolInfo::canDeleteRecursive() const
{
    Q_D(const KProtocolInfo);
    return d->canDeleteRecursive;
}

KProtocolInfo::FileNameUsedForCopying KProtocolInfo::fileNameUsedForCopying() const
{
    Q_D(const KProtocolInfo);
    return d->fileNameUsedForCopying;
}

bool KProtocolInfo::isFilterProtocol( const KUrl &url )
{
    return isFilterProtocol (url.protocol());
}

bool KProtocolInfo::isHelperProtocol( const KUrl &url )
{
    return isHelperProtocol (url.protocol());
}

bool KProtocolInfo::isHelperProtocol( const QString &protocol )
{
    // call the findProtocol directly (not via KProtocolManager) to bypass any proxy settings.
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
    if ( prot ) {
        return prot->m_isHelperProtocol;
    }
    const KService::Ptr service = KMimeTypeTrader::self()->preferredService(QString::fromLatin1("x-scheme-handler/") + protocol);
    return !service.isNull();
}

bool KProtocolInfo::isKnownProtocol( const KUrl &url )
{
    return isKnownProtocol (url.protocol());
}

bool KProtocolInfo::isKnownProtocol( const QString &protocol )
{
    // call the findProtocol (const QString&) to bypass any proxy settings.
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol(protocol);
    return prot || isHelperProtocol(protocol);
}
