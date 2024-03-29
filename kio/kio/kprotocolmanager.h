/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

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
#ifndef KPROTOCOLMANAGER_H
#define KPROTOCOLMANAGER_H

#include <QtCore/QStringList>

#include <kio/global.h>
#include "kprotocolinfo.h"

class KSharedConfig;
template<class T>
class KSharedPtr;
typedef KSharedPtr<KSharedConfig> KSharedConfigPtr;
namespace KIO
{
    class SlaveConfigPrivate;
} // namespace KIO

/**
 * Provides information about I/O (Internet, etc.) settings chosen/set
 * by the end user.
 *
 * KProtocolManager has a heap of static functions that allows only read
 * access to KDE's IO related settings. These include proxy, file
 * transfer resumption, timeout and user-agent related settings.
 *
 * The information provided by this class is generic enough to be applicable
 * to any application that makes use of KDE's IO sub-system.  Note that this
 * mean the proxy, timeout etc. settings are saved in a separate user-specific
 * config file and not in the config file of the application.
 *
 * Original author:
 * @author Torben Weis <weis@kde.org>
 *
 * Revised by:
 * @author Waldo Bastain <bastain@kde.org>
 * @author Dawit Alemayehu <adawit@kde.org>
 * @see KPAC
 */
class KIO_EXPORT KProtocolManager
{
public:


/*=========================== USER-AGENT SETTINGS ===========================*/


  /**
   * Returns the default user-agent string used for web browsing.
   *
   * @return the default user-agent string
   */
  static QString defaultUserAgent();

  /**
   * Returns the default user-agent value used for web browsing, for example
   * "Mozilla/5.0 (compatible; Konqueror/4.0; Linux; X11; i686; en_US) KHTML/4.0.1 (like Gecko)"
   *
   * @param keys can be any of the following:
   * @li 'o'	Show OS
   * @li 'v'	Show OS Version
   * @li 'p'	Show platform (only for X11)
   * @li 'm'	Show machine architecture
   * @li 'l'	Show language
   * @return the default user-agent value with the given @p keys
   */
  static QString defaultUserAgent(const QString &keys);

  /*
   * Returns system name, version and machine type, for example "Windows", "5.1", "i686".
   * This information can be used for constructing custom user-agent strings.
   *
   * @param systemName system name
   * @param systemVersion system version
   * @param machine machine type

   * @return true if system name, version and machine type has been provided
   *
   * @since 4.1
   */
  static bool getSystemNameVersionAndMachine(
    QString& systemName, QString& systemVersion, QString& machine );


/*=========================== TIMEOUT CONFIG ================================*/


  /**
   * Returns the preferred timeout value for reading from
   * remote connections in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int readTimeout();

  /**
   * Returns the preferred timeout value for remote connections
   * in seconds.
   *
   * @return timeout value for remote connection in secs.
   */
  static int connectTimeout();

  /**
   * Returns the preferred timeout value for proxy connections
   * in seconds.
   *
   * @return timeout value for proxy connection in secs.
   */
  static int proxyConnectTimeout();

  /**
   * Returns the preferred response timeout value for
   * remote connecting in seconds.
   *
   * @return timeout value for remote connection in seconds.
   */
  static int responseTimeout();


/*=============================== PROXY CONFIG ==============================*/


  /**
   * Returns whether or not the user specified the
   * use of proxy server to make connections.
   * @return true to use a proxy
   */
  static bool useProxy();

  /**
   * Returns whether or not the proxy server
   * lookup should be reversed or not.
   * @return true to use a reversed proxy
   */
  static bool useReverseProxy();

  /**
   * Types of proxy configuration
   * @li NoProxy     - No proxy is used
   * @li ManualProxy - Proxies are manually configured
   * @li EnvVarProxy - Use the proxy values set through environment variables.
   */
  enum ProxyType
  {
      NoProxy,
      ManualProxy,
      EnvVarProxy
  };

  /**
   * Returns the type of proxy configuration that is used.
   * @return the proxy type
   */
  static ProxyType proxyType();

  /**
   * Returns the strings for hosts that should contacted
   * DIRECTLY, bypassing any proxy settings.
   * @return a list of (comma-separated) hostnames or partial host
   *         names
   */
  static QString noProxyFor();

  /**
   * Returns the proxy server address for a given
   * protocol.
   *
   * @param protocol the protocol whose proxy info is needed
   * @returns the proxy server address if one is available,
   *          or QString() if not available
   */
  static QString proxyFor( const QString& protocol );

  /**
   * Returns the Proxy server address for a given URL.
   *
   * @ref proxyFor is used to find the proxy to use for the given url.
   *
   * If this function returns an empty string, then the request to a proxy server
   * must be denied. For a direct connection, without the use of a proxy, this
   * function will return "DIRECT".
   *
   * @param url the URL whose proxy info is needed
   * @returns the proxy server address if one is available, otherwise a QString().
   */
  static QString proxyForUrl( const KUrl& url );

  /**
   * Returns all the possible proxy server addresses for @p url.
   *
   * @ref proxyFor is used to find the proxy to use for the given url.
   *
   * If this function returns empty list, then the request is to a proxy server
   * must be denied. For a direct connection, this function will return a single
   * entry of "DIRECT".
   *
   * @since 4.7
   *
   * @param url the URL whose proxy info is needed
   * @returns the proxy server address if one is available, otherwise an empty list .
   */
  static QStringList proxiesForUrl( const KUrl& url );

  /**
   * Marks this proxy as bad (down). It will not be used for the
   * next 30 minutes. (The script may supply an alternate proxy)
   * @param proxy the proxy to mark as bad (as URL)
   */
  static void badProxy( const QString & proxy );

/*============================ DOWNLOAD CONFIG ==============================*/

  /**
   * Returns true if partial downloads should be
   * automatically resumed.
   * @return true to resume partial downloads
   */
  static bool autoResume();

  /**
   * Returns true if partial downloads should be marked
   * with a ".part" extension.
   * @return true if partial downloads should get an ".part" extension
   */
  static bool markPartial();

  /**
   * Returns the minimum file size for keeping aborted
   * downloads.
   *
   * Any data downloaded that does not meet this minimum
   * requirement will simply be discarded. The default size
   * is 5 KB.
   *
   * @return the minimum keep size for aborted downloads in bytes
   */
  static int minimumKeepSize();

  /*===================== PROTOCOL CAPABILITIES ===============================*/

  /**
   * Returns whether the protocol can list files/objects.
   * If a protocol supports listing it can be browsed in e.g. file-dialogs
   * and konqueror.
   *
   * Whether a protocol supports listing is determined by the "listing="
   * field in the protocol description file.
   * If the protocol support listing it should list the fields it provides in
   * this field. If the protocol does not support listing this field should
   * remain empty (default.)
   *
   * @param url the url to check
   * @return true if the protocol support listing
   * @see listing()
   */
  static bool supportsListing( const KUrl &url );

  /**
   * Returns whether the protocol can retrieve data from URLs.
   *
   * This corresponds to the "reading=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if it is possible to read from the URL
   */
  static bool supportsReading( const KUrl &url );

  /**
   * Returns whether the protocol can store data to URLs.
   *
   * This corresponds to the "writing=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports writing
   */
  static bool supportsWriting( const KUrl &url );

  /**
   * Returns whether the protocol can create directories/folders.
   *
   * This corresponds to the "makedir=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can create directories
   */
  static bool supportsMakeDir( const KUrl &url );

  /**
   * Returns whether the protocol can delete files/objects.
   *
   * This corresponds to the "deleting=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports deleting
   */
  static bool supportsDeleting( const KUrl &url );

  /**
   * Returns whether the protocol can create links between files/objects.
   *
   * This corresponds to the "linking=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports linking
   */
  static bool supportsLinking( const KUrl &url );

  /**
   * Returns whether the protocol can move files/objects between different
   * locations.
   *
   * This corresponds to the "moving=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol supports moving
   */
  static bool supportsMoving( const KUrl &url );

  /**
   * Returns whether the protocol can copy files/objects directly from the
   * filesystem itself. If not, the application will read files from the
   * filesystem using the file-protocol and pass the data on to the destination
   * protocol.
   *
   * This corresponds to the "copyFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files from the local file system
   */
  static bool canCopyFromFile( const KUrl &url );

  /**
   * Returns whether the protocol can copy files/objects directly to the
   * filesystem itself. If not, the application will receive the data from
   * the source protocol and store it in the filesystem using the
   * file-protocol.
   *
   * This corresponds to the "copyToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can copy files to the local file system
   */
  static bool canCopyToFile( const KUrl &url );

  /**
   * Returns whether the protocol can rename (i.e. move fast) files/objects
   * directly from the filesystem itself. If not, the application will read
   * files from the filesystem using the file-protocol and pass the data on
   * to the destination protocol.
   *
   * This corresponds to the "renameFromFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can rename/move files from the local file system
   */
  static bool canRenameFromFile( const KUrl &url );

  /**
   * Returns whether the protocol can rename (i.e. move fast) files/objects
   * directly to the filesystem itself. If not, the application will receive
   * the data from the source protocol and store it in the filesystem using the
   * file-protocol.
   *
   * This corresponds to the "renameToFile=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can rename files to the local file system
   */
  static bool canRenameToFile( const KUrl &url );

  /**
   * Returns whether the protocol can recursively delete directories by itself.
   * If not (the usual case) then KIO will list the directory and delete files
   * and empty directories one by one.
   *
   * This corresponds to the "deleteRecursive=" field in the protocol description file.
   * Valid values for this field are "true" or "false" (default).
   *
   * @param url the url to check
   * @return true if the protocol can delete non-empty directories by itself.
   */
  static bool canDeleteRecursive( const KUrl &url );

  /**
   * This setting defines the strategy to use for generating a filename, when
   * copying a file or directory to another directory. By default the destination
   * filename is made out of the filename in the source URL. However if the
   * ioslave displays names that are different from the filename of the URL
   * (e.g. kio_trash shows foo.txt and uses some internal URL), using Name means
   * that the display name (UDS_NAME) will be used to as the filename in the
   * destination directory.
   *
   * This corresponds to the "fileNameUsedForCopying=" field in the protocol description file.
   * Valid values for this field are "Name" or "FromURL" (default).
   *
   * @param url the url to check
   * @return how to generate the filename in the destination directory when copying/moving
   */
  static KProtocolInfo::FileNameUsedForCopying fileNameUsedForCopying( const KUrl &url );

  /**
   * Returns default mimetype for this URL based on the protocol.
   *
   * This corresponds to the "defaultMimetype=" field in the protocol description file.
   *
   * @param url the url to check
   * @return the default mime type of the protocol, or null if unknown
   */
  static QString defaultMimetype( const KUrl& url );

  /**
   * Returns whether the protocol can act as a source protocol.
   *
   * A source protocol retrieves data from or stores data to the
   * location specified by a URL.
   * A source protocol is the opposite of a filter protocol.
   *
   * The "source=" field in the protocol description file determines
   * whether a protocol is a source protocol or a filter protocol.
   * @param url the url to check
   * @return true if the protocol is a source of data (e.g. http), false if the
   *         protocol is a filter (e.g. gzip)
   */
  static bool isSourceProtocol( const KUrl &url );

  /*=============================== OTHERS ====================================*/


  /**
   * Force a reload of the general config file of
   * io-slaves ( kioslaverc).
   */
  static void reparseConfiguration();

  /**
   * Return the protocol to use in order to handle the given @p url
   * It's usually the same, except that FTP, when handled by a proxy,
   * needs an HTTP ioslave.
   *
   * When a proxy is to be used, proxy contains the URL for the proxy.
   * @param url the url to check
   * @param proxy the URL of the proxy to use
   * @return the slave protocol (e.g. 'http'), can be null if unknown
   */
  static QString slaveProtocol(const KUrl &url, QString &proxy);

  /**
   * Overloaded function that returns a list of all available proxy servers.
   *
   * @since 4.7
   */
  static QString slaveProtocol(const KUrl &url, QStringList &proxy);

  /**
   * Return Accept-Languages header built up according to user's desktop
   * language settings.
   * @return Accept-Languages header string
   */
  static QString acceptLanguagesHeader();

  /**
   * Returns the charset to use for the specified @ref url.
   *
   * @since 4.10
   */
  static QString charsetFor(const KUrl& url);

private:
  friend class KIO::SlaveConfigPrivate;

  /**
   * @internal
   * (Shared with SlaveConfig)
   */
  KIO_NO_EXPORT static KSharedConfigPtr config();
};
#endif
