/*
    Copyright (C) 1999 Torben Weis <weis@kde.org>
    Copyright (C) 2005-2006 David Faure <faure@kde.org>

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

/*
 * The currently active RFC for URL/URIs is RFC3986
 * Previous (and now deprecated) RFCs are RFC1738 and RFC2396
 */

#include "kurl.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kshell.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <QtNetwork/QHostInfo>

// static const int kurlDebugArea = 181; // see kdebug.areas

static const char s_kdeUriListMime[] = "application/x-kde4-urilist";

static QByteArray uriListData(const KUrl::List &urls)
{
    QByteArray uriListData;
    foreach(const KUrl &uit, urls) {
        // Get each URL encoded in utf8 - and since we get it in escaped
        // form on top of that, .toLatin1() is fine.
        uriListData += uit.toMimeDataString().toLatin1();
        uriListData += "\r\n";
    }
    return uriListData;
}

static QString trailingSlash(KUrl::AdjustPathOption trailing, const QString &path)
{
    if (trailing == KUrl::LeaveTrailingSlash) {
        return path;
    }

    QString result = path;
    if (trailing == KUrl::AddTrailingSlash) {
        int len = result.length();
        if (len > 0 && result[len - 1] != QLatin1Char('/')) {
            result += QLatin1Char('/');
        }
        return result;
    } else if (trailing == KUrl::RemoveTrailingSlash) {
        if (result == QLatin1String("/")) {
            return result;
        }
        int len = result.length();
        while (len > 1 && result[len - 1] == QLatin1Char('/')) {
            len--;
        }
        result.truncate(len);
        return result;
    }
    Q_ASSERT(false);
    return result;
}

static QString toPrettyPercentEncoding(const QString &input, bool forFragment)
{
    QString result;
    result.reserve(input.length());
    for (int i = 0; i < input.length(); ++i) {
        const QChar c = input.at(i);
        ushort u = c.unicode();
        if (u < 0x20
            || (!forFragment && u == '?') // don't escape '?' in fragments, not needed and wrong (#173101)
            || u == '#' || u == '%'
            || (u == ' ' && (i+1 == input.length() || input.at(i+1).unicode() == ' ')))
        {
            static const char hexdigits[] = "0123456789ABCDEF";
            result += QLatin1Char('%');
            result += QLatin1Char(hexdigits[(u & 0xf0) >> 4]);
            result += QLatin1Char(hexdigits[u & 0xf]);
        } else {
            result += c;
        }
    }

    return result;
}

static QString _relativePath(const QString &base_dir, const QString &path, bool &isParent)
{
    QString _base_dir(QDir::cleanPath(base_dir));
    QString _path(QDir::cleanPath(path.isEmpty() || QDir::isRelativePath(path) ? _base_dir+QLatin1Char('/') + path : path));

    if (_base_dir.isEmpty()) {
        return _path;
    }

    if (_base_dir[_base_dir.length() - 1] != QLatin1Char('/')) {
        _base_dir.append(QLatin1Char('/'));
    }

    const QStringList list1 = _base_dir.split(QLatin1Char('/'), QString::SkipEmptyParts);
    const QStringList list2 = _path.split(QLatin1Char('/'), QString::SkipEmptyParts);

    // Find where they meet
    int level = 0;
    int maxLevel = qMin(list1.count(), list2.count());
    while (level < maxLevel && list1[level] == list2[level]) {
        level++;
    }

    QString result;
    // Need to go down out of the first path to the common branch.
    for(int i = level; i < list1.count(); i++) {
        result.append(QLatin1String("../"));
    }

    // Now up up from the common branch to the second path.
    for(int i = level; i < list2.count(); i++) {
        result.append(list2[i]).append(QLatin1Char('/'));
    }

    if ((level < list2.count()) && (path[path.length() - 1] != QLatin1Char('/'))) {
        result.truncate(result.length() - 1);
    }

    isParent = (level == list1.count());

    return result;
}

bool KUrl::isRelativeUrl(const QString &_url)
{
    int len = _url.length();
    if (!len) {
        return true; // Very short relative URL.
    }
    const QChar *str = _url.unicode();

    // Absolute URL must start with alpha-character
    if (!isalpha(str[0].toLatin1())) {
         // Relative URL
        return true;
    }

    for(int i = 1; i < len; i++) {
        char c = str[i].toLatin1(); // Note: non-latin1 chars return 0!
        if (c == ':') {
            // Absolute URL
            return false;
        }

        // Protocol part may only contain alpha, digit, + or -
        if (!isalpha(c) && !isdigit(c) && (c != '+') && (c != '-')) {
             // Relative URL
            return true;
        }
    }
    // Relative URL, did not contain ':'
    return true;
}

KUrl::List::List(const KUrl &url)
{
    append(url);
}

KUrl::List::List(const QList<KUrl> &list)
    : QList<KUrl>(list)
{
}

KUrl::List::List(const QList<QUrl> &list)
{
    foreach(const QUrl &url, list) {
        append(KUrl(url));
    }
}

KUrl::List::List(const QStringList &list)
{
    reserve(size() + list.size());
    foreach (const QString &str, list) {
        append(KUrl(str));
    }
}

QStringList KUrl::List::toStringList() const
{
    return toStringList(KUrl::LeaveTrailingSlash);
}

QStringList KUrl::List::toStringList(KUrl::AdjustPathOption trailing) const
{
    QStringList lst;
    lst.reserve(size());
    for (KUrl::List::ConstIterator it = constBegin(); it != constEnd(); ++it) {
        lst.append(it->url(trailing));
    }
    return lst;
}

void KUrl::List::populateMimeData(QMimeData* mimeData,
                                  const KUrl::MetaDataMap& metaData,
                                  MimeDataFlags flags) const
{
    mimeData->setData(QString::fromLatin1("text/uri-list"), uriListData(*this));

    if ((flags & KUrl::NoTextExport) == 0) {
        QStringList prettyURLsList;
        KUrl::List::ConstIterator uit = constBegin();
        const KUrl::List::ConstIterator uEnd = constEnd();
        for ( ; uit != uEnd ; ++uit ) {
            QString prettyURL = (*uit).prettyUrl();
            if ((*uit).protocol() == QLatin1String("mailto")) {
                 // remove mailto: when pasting into konsole
                prettyURL = (*uit).path();
            }
            prettyURLsList.append(prettyURL);
        }

        QByteArray plainTextData = prettyURLsList.join(QString(QLatin1Char('\n'))).toLocal8Bit();
        if (count() > 1 ) {
            // terminate last line, unless it's the only line
            plainTextData.append("\n");
        }
        mimeData->setData( QString::fromLatin1("text/plain"), plainTextData );
    }

    if (!metaData.isEmpty()) {
        QByteArray metaDataData; // :)
        QMapIterator<QString, QString> it(metaData);
        while(it.hasNext()) {
            it.next();
            metaDataData += it.key().toUtf8();
            metaDataData += "$@@$";
            metaDataData += it.value().toUtf8();
            metaDataData += "$@@$";
        }
        mimeData->setData(QString::fromLatin1("application/x-kio-metadata"), metaDataData);
    }
}

void KUrl::List::populateMimeData(const KUrl::List& mostLocalUrls,
                                  QMimeData* mimeData,
                                  const KUrl::MetaDataMap& metaData,
                                  MimeDataFlags flags) const
{
    // Export the most local urls as text/uri-list and plain text.
    mostLocalUrls.populateMimeData(mimeData, metaData, flags);
    mimeData->setData(QString::fromLatin1(s_kdeUriListMime), uriListData(*this));
}

bool KUrl::List::canDecode( const QMimeData *mimeData )
{
    return (
        mimeData->hasFormat(QString::fromLatin1("text/uri-list")) ||
        mimeData->hasFormat(QString::fromLatin1(s_kdeUriListMime))
    );
}

QStringList KUrl::List::mimeDataTypes()
{
    return QStringList() << QString::fromLatin1(s_kdeUriListMime) << QString::fromLatin1("text/uri-list");
}

KUrl::List KUrl::List::fromMimeData(const QMimeData *mimeData,
                                    KUrl::MetaDataMap* metaData,
                                    DecodeOptions decodeOptions)
{

    KUrl::List uris;
    const char* firstMimeType = s_kdeUriListMime;
    const char* secondMimeType = "text/uri-list";
    if (decodeOptions == PreferLocalUrls) {
        qSwap(firstMimeType, secondMimeType);
    }
    QByteArray payload = mimeData->data(QString::fromLatin1(firstMimeType));
    if (payload.isEmpty()) {
        payload = mimeData->data(QString::fromLatin1(secondMimeType));
    }
    if (!payload.isEmpty()) {
        int c = 0;
        const char* d = payload.constData();
        while (c < payload.size() && d[c]) {
            int f = c;
            // Find line end
            while (c < payload.size() && d[c] && d[c]!='\r' && d[c] != '\n') {
                c++;
            }
            QByteArray s( d+f, c-f );
            if (s[0] != '#') {
                // non-comment?
                uris.append(KUrl::fromMimeDataByteArray(s));
            }
            // Skip junk
            while (c < payload.size() && d[c] && (d[c] == '\n' || d[c] == '\r')) {
                ++c;
            }
        }
    }
    if (metaData) {
        const QByteArray metaDataPayload = mimeData->data(QLatin1String("application/x-kio-metadata"));
        if (!metaDataPayload.isEmpty()) {
            QString str = QString::fromUtf8(metaDataPayload);
            Q_ASSERT(str.endsWith(QLatin1String("$@@$")));
            str.truncate( str.length() - 4 );
            const QStringList lst = str.split(QLatin1String("$@@$"));
            bool readingKey = true; // true, then false, then true, etc.
            QString key;
            foreach(const QString &it, lst) {
                if (readingKey) {
                    key = it;
                } else {
                    metaData->insert(key, it);
                }
                readingKey = !readingKey;
            }
            Q_ASSERT(readingKey); // an odd number of items would be, well, odd ;-)
        }
    }

    return uris;
}

KUrl::List::operator QVariant() const
{
    return qVariantFromValue(*this);
}

KUrl::List::operator QList<QUrl>() const
{
    QList<QUrl> list;
    list.reserve(size());
    foreach(const KUrl &url, *this) {
        list << url;
    }
    return list;
}

///

KUrl::KUrl()
    : QUrl()
{
}

KUrl::~KUrl()
{
}

KUrl::KUrl(const QString &str)
  : QUrl()
{
    if (!str.isEmpty()) {
        if (str[0] == QLatin1Char('/') || str[0] == QLatin1Char('~')) {
            setPath(str);
        } else {
            setUrl(str);
        }
    }
}

KUrl::KUrl(const char *str)
  : QUrl()
{
    if (str && str[0]) {
        if (str[0] == '/' || str[0] == '~') {
            setPath(QString::fromUtf8(str));
        } else {
            setUrl(QString::fromUtf8(str), QUrl::TolerantMode);
        }
    }
}

KUrl::KUrl(const QByteArray &str)
   : QUrl()
{
    if (!str.isEmpty()) {
        if (str[0] == '/' || str[0] == '~') {
            setPath(QString::fromUtf8(str));
        } else {
            setUrl(QString::fromUtf8(str.constData(), str.size()), QUrl::TolerantMode);
        }
    }
}

KUrl::KUrl(const KUrl &u)
    : QUrl(u)
{
}

KUrl::KUrl(const QUrl &u)
    : QUrl(u)
{
}

KUrl::KUrl(const KUrl &u, const QString &relurl)
   : QUrl(u.resolved(relurl))
{
}

KUrl& KUrl::operator=(const KUrl &u)
{
    QUrl::operator=(u);
    return *this;
}

bool KUrl::operator==(const KUrl &u) const
{
    return QUrl::operator==(u);
}

bool KUrl::operator==(const QString &_u) const
{
    KUrl u(_u);
    return (*this == u);
}

KUrl::operator QVariant() const
{
    return qVariantFromValue(*this);
}

bool KUrl::equals(const KUrl &_u, const EqualsOptions &options) const
{
    if (!isValid() || !_u.isValid()) {
        return false;
    }

    if (options & CompareWithoutTrailingSlash || options & CompareWithoutFragment) {
        QString path1 = path((options & CompareWithoutTrailingSlash) ? RemoveTrailingSlash : LeaveTrailingSlash);
        QString path2 = _u.path((options & CompareWithoutTrailingSlash) ? RemoveTrailingSlash : LeaveTrailingSlash);

        if (options & AllowEmptyPath) {
            if (path1 == QLatin1String("/")) {
                path1.clear();
            }
            if (path2 == QLatin1String("/")) {
                path2.clear();
            }
        }

        if (path1 != path2) {
            return false;
        }

        if (scheme() == _u.scheme() &&
            authority() == _u.authority() && // user+pass+host+port
            query() == _u.query() &&
            (fragment() == _u.fragment() || options & CompareWithoutFragment))
        {
            return true;
        }
        return false;
    }

    return (*this == _u);
}

KUrl KUrl::fromPath(const QString &text)
{
    KUrl u;
    u.setPath(text);
    return u;
}

void KUrl::setFileName(const QString &_txt)
{
    setFragment(QString());
    int i = 0;
    while (i < _txt.length() && _txt[i] == QLatin1Char('/')) {
        ++i;
    }

    QString tmp = (i ? _txt.mid(i) : _txt);
    QString path = this->path();
    if (path.isEmpty()) {
        path = QDir::rootPath();
    } else {
        int lastSlash = path.lastIndexOf( QLatin1Char('/') );
        if (lastSlash == -1) {
            // there's only the file name, remove it
            path.clear();
        } else if (!path.endsWith(QLatin1Char('/'))) {
            // keep the "/"
            path.truncate(lastSlash + 1);
        }
    }

    path += tmp;
    setPath(path);

    cleanPath();
}

void KUrl::cleanPath()
{
    const QString newPath = QDir::cleanPath(path());
    if (path() != newPath) {
        setPath(newPath);
    }
}

void KUrl::adjustPath(AdjustPathOption trailing)
{
    const QString newPath = trailingSlash(trailing, path());
    if (path() != newPath) {
        setPath(newPath);
    }
}

QString KUrl::path(AdjustPathOption trailing) const
{
  return trailingSlash(trailing, QUrl::path());
}

QString KUrl::toLocalFile(AdjustPathOption trailing) const
{
    if (hasHost() && isLocalFile()) {
        KUrl urlWithoutHost(*this);
        urlWithoutHost.setHost(QString());
        return trailingSlash(trailing, urlWithoutHost.toLocalFile());
    }
#warning FIXME: Remove condition below once QTBUG-20322 is fixed, also see BR# 194746.
    if (isLocalFile()) {
        return trailingSlash(trailing, QUrl::path());
    }
    return trailingSlash(trailing, QUrl::toLocalFile());
}

bool KUrl::isLocalFile() const
{
    if (scheme().compare(QLatin1String("file"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    if (host().isEmpty() || (host() == QLatin1String("localhost"))) {
        return true;
    }
    return (host() == QHostInfo::localHostName().toLower());
}

void KUrl::setFileEncoding(const QString &encoding)
{
    if (!isLocalFile()) {
        return;
    }
    addQueryItem(QLatin1String("charset"), encoding);
}

QString KUrl::fileEncoding() const
{
    if (!isLocalFile()) {
        return QString();
    }
    return queryItem(QLatin1String("charset"));
}

QString KUrl::url(AdjustPathOption trailing) const
{
    if (QString::compare(scheme(), QLatin1String("mailto"), Qt::CaseInsensitive) == 0) {
        // mailto urls should be prettified, see the url183433 testcase.
        return prettyUrl(trailing);
    }
    if (trailing == AddTrailingSlash && !path().endsWith(QLatin1Char('/'))) {
        // -1 and 0 are provided by QUrl, but not +1, so that one is a bit tricky.
        // To avoid reimplementing toEncoded() all over again, I just use another QUrl
        // Let's hope this is fast, or not called often...
        QUrl newUrl(*this);
        newUrl.setPath(path() + QLatin1Char('/'));
        return QString::fromLatin1(newUrl.toEncoded());
    } else if ( trailing == RemoveTrailingSlash) {
        const QString cleanedPath = trailingSlash(trailing, path());
        if (cleanedPath == QLatin1String("/")) {
            if (path() != QLatin1String("/")) {
                QUrl fixedUrl = *this;
                fixedUrl.setPath(cleanedPath);
                return QString::fromLatin1(fixedUrl.toEncoded(None));
            }
            return QString::fromLatin1(toEncoded(None));
        }
    }
    return QString::fromLatin1(toEncoded(trailing == RemoveTrailingSlash ? StripTrailingSlash : None));
}

QString KUrl::prettyUrl(AdjustPathOption trailing) const
{
    // reconstruct the URL in a "pretty" form
    // a "pretty" URL is NOT suitable for data transfer. It's only for showing data to the user.
    // however, it must be parseable back to its original state, since
    // notably Konqueror displays it in the Location address.

    // A pretty URL is the same as a normal URL, except that:
    // - the password is removed
    // - the hostname is shown in Unicode (as opposed to ACE/Punycode)
    // - the pathname and fragment parts are shown in Unicode (as opposed to %-encoding)
    QString result = scheme();
    if (!result.isEmpty()) {
        if (!authority().isEmpty() || result == QLatin1String("file") || (path().isEmpty()
            && scheme().compare(QLatin1String("mailto"), Qt::CaseInsensitive) != 0))
        {
            result += QLatin1String("://");
        } else {
            result += QLatin1Char(':');
        }
    }

    QString tmp = userName();
    if (!tmp.isEmpty()) {
        result += QString::fromLatin1(QUrl::toPercentEncoding(tmp));
        result += QLatin1Char('@');
    }

    // Check if host is an ipv6 address
    tmp = host();
    if (tmp.contains(QLatin1Char(':'))) {
        result += QLatin1Char('[') + tmp + QLatin1Char(']');
    } else {
        result += tmp;
    }

    if (port() != -1) {
        result += QLatin1Char(':');
        result += QString::number(port());
    }

    tmp = path();
    result += toPrettyPercentEncoding(tmp, false);

    // adjust the trailing slash, if necessary
    if (trailing == AddTrailingSlash && !tmp.endsWith(QLatin1Char('/'))) {
        result += QLatin1Char('/');
    } else if (trailing == RemoveTrailingSlash && tmp.length() > 1 && tmp.endsWith(QLatin1Char('/'))) {
        result.chop(1);
    }

    if (hasQuery()) {
        result += QLatin1Char('?');
        result += query();
    }

    if (hasFragment()) {
        result += QLatin1Char('#');
        result += toPrettyPercentEncoding(fragment(), true);
    }

    return result;
}

QString KUrl::pathOrUrl(AdjustPathOption trailing) const
{
    if (isLocalFile() && fragment().isNull() && !hasQuery()) {
        return toLocalFile(trailing);
    }
    return prettyUrl(trailing);
}

// used for text/uri-list in the mime data. don't fold this into populateMimeData, it's also needed
// by other code like konqdrag
QString KUrl::toMimeDataString() const
{
    if (isLocalFile()) {
        return url();
    }

    if (hasPass()) {
        KUrl safeUrl(*this);
        safeUrl.setPassword(QString());
        return safeUrl.url();
    }
    return url();
}

KUrl KUrl::fromMimeDataByteArray(const QByteArray &str)
{
    if (str.startsWith("file:")) {
        return KUrl(str);
    }
    return KUrl(str);
}

QString KUrl::fileName(const DirectoryOptions &options) const
{
    Q_ASSERT(options != 0); // Disallow options == false
    QString fname;
    const QString path = this->path();

    int len = path.length();
    if (len == 0) {
        return fname;
    }

    if (!(options & ObeyTrailingSlash)) {
        while (len >= 1 && path[len - 1] == QLatin1Char('/')) {
            len--;
        }
    } else if (path[len - 1] == QLatin1Char('/')) {
        return fname;
    }

    // Does the path only consist of '/' characters ?
    if (len == 1 && path[0] == QLatin1Char('/')) {
        return fname;
    }

    // Skip last n slashes
    int n = 1;
    int i = len;
    do {
        i = path.lastIndexOf(QLatin1Char('/'), i - 1);
    } while (--n && i > 0);

    // If ( i == -1 ) => the first character is not a '/'
    // So it's some URL like file:blah.tgz, return the whole path
    if (i == -1) {
        if (len == (int)path.length()) {
            fname = path;
        } else {
            // Might get here if _strip_trailing_slash is true
            fname = path.left(len);
        }
    } else {
        fname = path.mid(i + 1, len - i - 1); // TO CHECK
    }
    return fname;
}

void KUrl::addPath(const QString &_txt)
{
    if (_txt.isEmpty()) {
        return;
    }

    QString strPath = path();
    int i = 0;
    int len = strPath.length();
    // Add the trailing '/' if it is missing
    if (_txt[0] != QLatin1Char('/') && (len == 0 || strPath[len - 1] != QLatin1Char('/'))) {
        strPath += QLatin1Char('/');
    }

    // No double '/' characters
    i = 0;
    const int _txtlen = _txt.length();
    if (strPath.endsWith(QLatin1Char('/'))) {
        while (i < _txtlen && _txt[i] == QLatin1Char('/')) {
            ++i;
        }
    }

    setPath(strPath + _txt.mid(i));
}

QString KUrl::directory(const DirectoryOptions &options) const
{
    Q_ASSERT(options != 0); // Disallow options == false
    QString result = path();
    if (!(options & ObeyTrailingSlash)) {
        result = trailingSlash(RemoveTrailingSlash, result);
    }

    if (result.isEmpty() || result == QLatin1String("/")) {
        return result;
    }

    int i = result.lastIndexOf(QLatin1Char('/'));
    // If ( i == -1 ) => the first character is not a '/'
    // So it's some URL like file:blah.tgz, with no path
    if (i == -1) {
        return QString();
    }

    if (i == 0) {
        return QString(QLatin1Char('/'));
    }


    if (options & AppendTrailingSlash) {
        result = result.left(i + 1);
    } else {
        result = result.left(i);
    }

    return result;
}

KUrl KUrl::upUrl( ) const
{
    KUrl u(*this);
    u.setPath(QUrl::path() + QLatin1String("/.."));
    return u;
}

void KUrl::setDirectory(const QString &dir)
{
    if (dir.endsWith(QLatin1Char('/'))) {
        setPath(dir);
    } else {
        setPath(dir + QLatin1Char('/'));
    }
}

QString KUrl::relativePath(const QString &base_dir, const QString &path, bool *isParent)
{
    bool parent = false;
    QString result = _relativePath(base_dir, path, parent);
    if (parent) {
        result.prepend(QLatin1String("./"));
    }

    if (isParent) {
      *isParent = parent;
    }

    return result;
}

QString KUrl::relativeUrl(const KUrl &base_url, const KUrl &url)
{
    if ((url.protocol() != base_url.protocol()) ||
        (url.host() != base_url.host()) ||
        (url.port() && url.port() != base_url.port()) ||
        (url.hasUser() && url.user() != base_url.user()) ||
        (url.hasPass() && url.pass() != base_url.pass()))
    {
        return url.url();
    }

   QString relURL;
    if ((base_url.path() != url.path()) || (base_url.query() != url.query())) {
        bool dummy;
        QString basePath = base_url.directory(KUrl::ObeyTrailingSlash);
        static const char s_pathExcludeChars[] = "!$&'()*+,;=:@/";
        relURL = QString::fromLatin1(QUrl::toPercentEncoding(_relativePath(basePath, url.path(), dummy), s_pathExcludeChars));
        relURL += url.query();
    }

    if (url.hasRef()) {
        relURL += QLatin1Char('#');
        relURL += url.ref();
    }

    if (relURL.isEmpty()) {
        return QLatin1String("./");
    }

    return relURL;
}

void KUrl::setPath(const QString &_path)
{
    if (scheme().isEmpty() && !_path.startsWith(QLatin1String("file:/"))) {
        setScheme(QLatin1String("file"));
    }
    QString path = KShell::tildeExpand(_path);
    if (path.isEmpty()) {
        path = _path;
    }
    QUrl::setPath(path);
}

void KUrl::populateMimeData(QMimeData* mimeData, const MetaDataMap& metaData, MimeDataFlags flags) const
{
    KUrl::List lst(*this);
    lst.populateMimeData(mimeData, metaData, flags);
}

bool KUrl::isParentOf(const KUrl &u) const
{
    return QUrl::isParentOf(u) || equals(u, CompareWithoutTrailingSlash);
}

uint qHash(const KUrl& kurl)
{
    // qHash(kurl.url()) was the worse implementation possible, since QUrl::toEncoded()
    // had to concatenate the bits of the url into the full url every time.
    return qHash(kurl.protocol()) ^ qHash(kurl.path()) ^ qHash(kurl.fragment()) ^ qHash(kurl.query());
}
