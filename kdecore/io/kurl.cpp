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

// for reference:
// https://www.rfc-editor.org/rfc/rfc3986

#include "kurl.h"

#include <kdebug.h>
#include <kshell.h>
#include <kde_file.h>

#include <QDir>
#include <QHostInfo>

static const char s_kdeUriListMime[] = "application/x-kde4-urilist";

// FIXME: using local files to pass around queries and fragments is totally bonkers, this will make
// sure they are not a thing
// #define KURL_COMPAT_CHECK
#ifdef KURL_COMPAT_CHECK
// see kdebug.areas
static const int s_kurlarea = 181;

void kCheckLocalFile(const KUrl *kurl)
{
    if (kurl->isLocalFile()) {
        if (kurl->hasQuery() || kurl->hasFragment()) {
            kFatal(s_kurlarea) << "Query or fragment detected in" << kurl->prettyUrl();
        }
    }
}
#endif

static QString kPathFileName(const QString &path)
{
    const int lastslash = path.lastIndexOf(QLatin1Char('/'));
    return path.mid(lastslash + 1);
}

static QString kPathDirectory(const QString &path)
{
    const int lastslash = path.lastIndexOf(QLatin1Char('/'));
    if (lastslash == -1) {
        return QString();
    }
    if (lastslash == 0) {
        return QString(QLatin1Char('/'));
    }
    Q_ASSERT(!path.endsWith(QLatin1Char('/')));
    QString result = path.left(lastslash);
    result.append(QLatin1Char('/'));
    return result;
}

static QByteArray kUriListData(const KUrl::List &urls)
{
    QByteArray result;
    foreach (const KUrl &url, urls) {
        // Get each URL encoded in utf8
        result += url.toMimeDataString().toUtf8();
        result += "\r\n";
    }
    return result;
}

static QString kTrailingSlash(KUrl::AdjustPathOption trailing, const QString &path)
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

static QString kRelativePath(const QString &base_dir, const QString &path, bool &isParent)
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
    reserve(list.size());
    foreach (const QUrl &url, list) {
        append(KUrl(url));
    }
}

KUrl::List::List(const QStringList &list)
{
    reserve(list.size());
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
    QStringList result;
    result.reserve(size());
    for (KUrl::List::ConstIterator it = constBegin(); it != constEnd(); ++it) {
        result.append(it->url(trailing));
    }
    return result;
}

void KUrl::List::populateMimeData(QMimeData* mimeData,
                                  MimeDataFlags flags) const
{
    mimeData->setData(QString::fromLatin1("text/uri-list"), kUriListData(*this));

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
        mimeData->setData(QString::fromLatin1("text/plain"), plainTextData);
    }
}

void KUrl::List::populateMimeData(const KUrl::List& mostLocalUrls,
                                  QMimeData* mimeData,
                                  MimeDataFlags flags) const
{
    // Export the most local urls as text/uri-list and plain text.
    mostLocalUrls.populateMimeData(mimeData, flags);
    mimeData->setData(QString::fromLatin1(s_kdeUriListMime), kUriListData(*this));
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
                uris.append(KUrl(s));
            }
            // Skip junk
            while (c < payload.size() && d[c] && (d[c] == '\n' || d[c] == '\r')) {
                ++c;
            }
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
    QList<QUrl> result;
    result.reserve(size());
    foreach(const KUrl &url, *this) {
        result << url;
    }
    return result;
}

///
KUrl::KUrl()
    : QUrl()
{
}

KUrl::KUrl(const QString &str)
  : QUrl()
{
    if (!str.isEmpty()) {
        if (str[0] == QLatin1Char('/') || str[0] == QLatin1Char('~')) {
            setPath(str);
        } else {
            setUrl(str, QUrl::TolerantMode);
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
            setUrl(QUrl::fromPercentEncoding(str), QUrl::TolerantMode);
        }
    }
}

KUrl::KUrl(const QByteArray &str)
   : QUrl()
{
    if (!str.isEmpty()) {
        if (str[0] == '/' || str[0] == '~') {
            setPath(QString::fromUtf8(str.constData(), str.size()));
        } else {
            setUrl(QUrl::fromPercentEncoding(str), QUrl::TolerantMode);
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
    return equals(u, KUrl::RemoveTrailingSlash);
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

bool KUrl::equals(const KUrl &u, AdjustPathOption trailing) const
{
    if (isValid() != u.isValid()) {
        return false;
    }
    if (isLocalFile()) {
        // that is how QFileInfo does it essentially
        return (QDir::cleanPath(url(trailing)) == QDir::cleanPath(u.url(trailing)));
    }
    return (url(trailing) == u.url(trailing));
}

KUrl KUrl::fromPath(const QString &text)
{
    KUrl u;
    u.setPath(text);
    return u;
}

void KUrl::setFileName(const QString &txt)
{
    setFragment(QString());
    int i = 0;
    while (i < txt.length() && txt[i] == QLatin1Char('/')) {
        ++i;
    }

    QString tmp = (i ? txt.mid(i) : txt);
    QString path = this->path();
    if (path.isEmpty()) {
        path = QDir::rootPath();
    } else {
        int lastSlash = path.lastIndexOf(QLatin1Char('/'));
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
    const QString newpath = QDir::cleanPath(path());
    if (path() != newpath) {
        setPath(newpath);
    }
}

void KUrl::adjustPath(AdjustPathOption trailing)
{
    const QString newpath = kTrailingSlash(trailing, path());
    if (path() != newpath) {
        setPath(newpath);
    }
}

QString KUrl::path(AdjustPathOption trailing) const
{
    return kTrailingSlash(trailing, QUrl::path());
}

QString KUrl::toLocalFile(AdjustPathOption trailing) const
{
    if (hasHost() && isLocalFile()) {
        KUrl urlWithoutHost(*this);
        urlWithoutHost.setHost(QString());
        return kTrailingSlash(trailing, urlWithoutHost.toLocalFile());
    }
    if (isLocalFile()) {
        return kTrailingSlash(trailing, QUrl::path());
    }
    return kTrailingSlash(trailing, QUrl::toLocalFile());
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

QString KUrl::url(AdjustPathOption trailing) const
{
    if (QString::compare(scheme(), QLatin1String("mailto"), Qt::CaseInsensitive) == 0) {
        // mailto urls should be prettified, see the url183433 testcase.
        return prettyUrl(trailing);
    }
    if (isLocalFile()) {
#ifdef KURL_COMPAT_CHECK
        kCheckLocalFile(this);
#endif
        QString result = path(trailing);
        if (hasQuery()) {
            result.append(QLatin1Char('?'));
            result.append(query());
        }
        if (hasFragment()) {
            result.append(QLatin1Char('#'));
            result.append(fragment());
        }
        return result;
    }
    if (trailing == AddTrailingSlash) {
        return QUrl::toString(QUrl::AddTrailingSlash);
    } else if (trailing == RemoveTrailingSlash) {
        return QUrl::toString(QUrl::StripTrailingSlash);
    }
    return QUrl::toString(QUrl::None);
}

// A pretty URL is the same as a normal URL, except that:
// - the password is removed
// - the hostname is shown in Unicode (as opposed to ACE/Punycode)
// - the pathname and fragment parts are shown in Unicode (as opposed to %-encoding)
QString KUrl::prettyUrl(AdjustPathOption trailing) const
{
    if (isLocalFile()) {
        QString result = path(trailing);
        if (hasQuery()) {
            result.append(QLatin1Char('?'));
            result.append(query());
        }
        if (hasFragment()) {
            result.append(QLatin1Char('#'));
            result.append(fragment());
        }
        return result;
    }
    if (trailing == AddTrailingSlash) {
        return QUrl::toString(QUrl::AddTrailingSlash | QUrl::RemovePassword);
    } else if (trailing == RemoveTrailingSlash) {
        return QUrl::toString(QUrl::StripTrailingSlash | QUrl::RemovePassword);
    }
    return QUrl::toString(QUrl::RemovePassword);
}

QString KUrl::pathOrUrl(AdjustPathOption trailing) const
{
    if (isLocalFile() && !hasFragment() && !hasQuery()) {
        return toLocalFile(trailing);
    }
    return url(trailing);
}

// used for text/uri-list in the mime data. don't fold this into populateMimeData, it's also needed
// by other code like konqdrag
QString KUrl::toMimeDataString() const
{
    if (hasPass() && !isLocalFile()) {
        KUrl safeUrl(*this);
        safeUrl.setPassword(QString());
        return safeUrl.url();
    }
    return url();
}

QString KUrl::fileName(AdjustPathOption trailing) const
{
    const QString urlpath = path();
    if (urlpath.isEmpty()) {
        return urlpath;
    }
    if (!urlpath.contains(QLatin1Char('/'))) {
        return urlpath;
    }
    return kTrailingSlash(trailing, kPathFileName(urlpath));
}

void KUrl::addPath(const QString &txt)
{
    if (txt.isEmpty()) {
        return;
    }

    QString strPath = path();
    int i = 0;
    int len = strPath.length();
    // Add the trailing '/' if it is missing
    if (txt[0] != QLatin1Char('/') && (len == 0 || strPath[len - 1] != QLatin1Char('/'))) {
        strPath += QLatin1Char('/');
    }

    // No double '/' characters
    i = 0;
    const int txtlen = txt.length();
    if (strPath.endsWith(QLatin1Char('/'))) {
        while (i < txtlen && txt[i] == QLatin1Char('/')) {
            ++i;
        }
    }

    setPath(strPath + txt.mid(i));
}

QString KUrl::directory(AdjustPathOption trailing) const
{
    QString urlpath = path();
    if (urlpath.isEmpty() || urlpath == QLatin1String("/")) {
        return urlpath;
    }
    return kTrailingSlash(trailing, kPathDirectory(urlpath));
}

KUrl KUrl::upUrl() const
{
    const QString urlpath = QUrl::path();
    if (urlpath.isEmpty() || urlpath == QLatin1String("/")) {
        return *this;
    }

    static const QString s_dotdotslash = QString::fromLatin1("../");
    if (urlpath.count(s_dotdotslash) >= 10) {
        // way too long, going to reach the path limit with that
        KUrl result(*this);
        result.setPath(QLatin1String("/"));
        result.setQuery(QString());
        result.setFragment(QString());
        return result;
    }

    if (QDir::isRelativePath(urlpath)) {
        KUrl result(*this);
        QString newpath = s_dotdotslash;
        newpath.append(kPathDirectory(urlpath));
        result.setPath(newpath);
        result.setQuery(QString());
        result.setFragment(QString());
        return result;
    }

    const QString cleanurlpath = QDir::cleanPath(urlpath);
    if (cleanurlpath.count(QLatin1Char('/')) <= 1) {
        // something like /home
        KUrl result(*this);
        result.setPath(QLatin1String("/"));
        result.setQuery(QString());
        result.setFragment(QString());
        return result;
    }

    if (isLocalFile()) {
        QString newpath;
        if (urlpath.endsWith(QLatin1Char('/'))) {
            // assuming it is directory
            newpath = kPathDirectory(cleanurlpath);
        } else {
            // the only way to be sure is to stat() then
            KDE_struct_stat statbuff;
            const QByteArray urlpathencoded = QFile::encodeName(urlpath);
            if (KDE_stat(urlpathencoded, &statbuff) == 0 && S_ISDIR(statbuff.st_mode)) {
                newpath = kPathDirectory(urlpath);
            } else {
                newpath = kPathDirectory(urlpath);
                newpath = kPathDirectory(newpath.mid(0, newpath.size() - 1));
            }
        }
        KUrl result(*this);
        result.setPath(newpath);
        result.setQuery(QString());
        result.setFragment(QString());
        return result;
    }

    return resolved(QUrl(QString::fromLatin1("..")));
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
    QString result = kRelativePath(base_dir, path, parent);
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
        (url.hasUser() && url.userName() != base_url.userName()) ||
        (url.hasPass() && url.password() != base_url.password()))
    {
        return url.url();
    }

    QString relURL;
    if ((base_url.path() != url.path()) || (base_url.query() != url.query())) {
        bool dummy = false;
        QString basePath = base_url.directory(KUrl::LeaveTrailingSlash);
        relURL = kRelativePath(basePath, url.path(), dummy);
        relURL += url.query();
    }

    if (url.hasFragment()) {
        relURL += QLatin1Char('#');
        relURL += url.fragment();
    }

    if (relURL.isEmpty()) {
        return QLatin1String("./");
    }

    return relURL;
}

bool KUrl::isRelativeUrl(const QString &_url)
{
    int len = _url.length();
    if (!len) {
        // Very short relative URL.
        return true;
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

void KUrl::setPath(const QString &_path)
{
    QString newpath = KShell::tildeExpand(_path);
    if (newpath.isEmpty()) {
        newpath = _path;
    }
    if (newpath.startsWith(QLatin1String("file://"))) {
        newpath.chop(7);
    }
    if (scheme().isEmpty()) {
        if (newpath.isEmpty()) {
            // Empty scheme and path - that's null/empty local file URL regardless of query and fragment
            QUrl::clear();
            return;
        }
        setScheme(QLatin1String("file"));
    }
    QUrl::setPath(newpath);
}

void KUrl::populateMimeData(QMimeData *mimeData, const  MimeDataFlags flags) const
{
    KUrl::List lst(*this);
    lst.populateMimeData(mimeData, flags);
}

bool KUrl::isParentOf(const KUrl &u) const
{
    return QUrl::isParentOf(u) || equals(u, KUrl::RemoveTrailingSlash);
}

uint qHash(const KUrl &kurl)
{
    // qHash(kurl.url()) was the worse implementation possible, since QUrl::toEncoded()
    // had to concatenate the bits of the url into the full url every time.
    return qHash(kurl.url());
}
