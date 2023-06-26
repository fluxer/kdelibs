// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
 *  Copyright (C) 2005-2006 David Faure <faure@kde.org>
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

#ifndef KURL_H
#define KURL_H

#include <kdecore_export.h>

#include <QtCore/qvariant.h>
#include <QtCore/qurl.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmimedata.h>

class KUrlPrivate;

/**
 * \class KUrl kurl.h <KUrl>
 *
 * Represents and parses a URL.
 *
 * A prototypical URL looks like:
 * \code
 *   protocol://user:password\@hostname:port/path/to/file.ext#reference
 * \endcode
 *
 * KUrl handles escaping of URLs. This means that the specification
 * of a full URL will differ from the corresponding string that would specify a
 * local file or directory in file-operations like fopen. This is because an URL
 * doesn't allow certain characters and escapes them. (e.g. '#'->"%23", space->"%20")
 * (In a URL the hash-character '#' is used to specify a "reference", i.e. the position
 * within a document).
 *
 * The constructor KUrl(const QString&) expects a string properly escaped,
 * or at least non-ambiguous.
 * If you have the absolute path you should use KUrl::fromPath(const QString&).
 * \code
 *     KUrl kurl = KUrl::fromPath("/bar/#foo#");
 *     QString url = kurl.url();    // -> "file:///bar/%23foo%23"
 * \endcode
 *
 * If you have the URL of a local file or directory and need the absolute path,
 * you would use toLocalFile().
 * \code
 *    KUrl url( "file:///bar/%23foo%23" );
 *    ...
 *    if ( url.isLocalFile() )
 *       QString path = url.toLocalFile();       // -> "/bar/#foo#"
 * \endcode
 *
 * This must also be considered when you have separated directory and file
 * strings and need to put them together.
 * While you can simply concatenate normal path strings, you must take care if
 * the directory-part is already an escaped URL.
 * (This might be needed if the user specifies a relative path, and your
 * program supplies the rest from elsewhere.)
 *
 * Wrong:
 * \code
 *    QString dirUrl = "file:///bar/";
 *    QString fileName = "#foo#";
 *    QString invalidURL = dirUrl + fileName;   // -> "file:///bar/#foo#" won't behave like you would expect.
 * \endcode
 * Instead you should use addPath():
 * Right:
 * \code
 *    KUrl url( "file:///bar/" );
 *    QString fileName = "#foo#";
 *    url.addPath( fileName );
 *    QString validURL = url.url();    // -> "file:///bar/%23foo%23"
 * \endcode
 *
 * Also consider that some URLs contain the password, but this shouldn't be
 * visible. Your program should use prettyUrl() every time it displays a
 * URL, whether in the GUI or in debug output or...
 *
 * \code
 *    KUrl url( "ftp://name:password@ftp.faraway.org/bar/%23foo%23");
 *    QString visibleURL = url.prettyUrl(); // -> "ftp://name@ftp.faraway.org/bar/%23foo%23"
 * \endcode
 * Note that prettyUrl() doesn't change the character escapes (like "%23").
 * Otherwise the URL would be invalid and the user wouldn't be able to use it in another
 * context.
 *
 */
class KDECORE_EXPORT KUrl : public QUrl
{
public:
    enum MimeDataFlags {
        DefaultMimeDataFlags = 0,
        NoTextExport = 1
    };

    /**
    * Options to be used in adjustPath
    */
    enum AdjustPathOption {
        /**
         * strips a trailing '/', except when the path is already just "/".
         */
        RemoveTrailingSlash,

        /**
         * Do not change the path.
         */
        LeaveTrailingSlash,

        /**
         * adds a trailing '/' if there is none yet
         */
        AddTrailingSlash
    };

    /**
     * \class List kurl.h <KUrl>
     *
     * KUrl::List is a QList that contains KUrls with a few
     * convenience methods.
     * @see KUrl
     * @see QList
     */
    class KDECORE_EXPORT List : public QList<KUrl>
    {
    public:
        /**
         * Creates an empty List.
         */
        List() { }

        /**
         * Creates a list that contains the given URL as only
         * item.
         * @param url the url to add.
         */
        List(const KUrl &url);
    
        /**
         * Creates a list that contains the URLs from the given
         * list of strings.
         * @param list the list containing the URLs as strings
         */
        List(const QStringList &list);
    
        /**
         * Creates a list that contains the URLs from the given QList<KUrl>.
         * @param list the list containing the URLs
         */
        List(const QList<KUrl> &list);
    
        /**
         * Creates a list that contains the URLs from the given QList<KUrl>.
         * @param list the list containing the URLs
         * @since 4.7
         */
        List(const QList<QUrl> &list);
    
        /**
         * Converts the URLs of this list to a list of strings.
         * @return the list of strings
         */
        QStringList toStringList() const;

        /**
         * Converts the URLs of this list to a list of strings.
         *
         * @param trailing use to add or remove a trailing slash to/from the path.
         *
         * @return the list of strings
         *
         * @since 4.6
         */
        QStringList toStringList(KUrl::AdjustPathOption trailing) const;

        /**
         * Converts this KUrl::List to a QVariant, this allows to use KUrl::List
         * in QVariant() constructor
         */
        operator QVariant() const;

        /**
         * Converts this KUrl::List into a list of QUrl instances.
         * @since 4.7
         */
        operator QList<QUrl>() const;

        /**
         * Adds URLs data into the given QMimeData.
         *
         * By default, populateMimeData also exports the URLs as plain text, for e.g. dropping
         * onto a text editor.
         * But in some cases this might not be wanted, e.g. if adding other mime data
         * which provides better plain text data.
         *
         * WARNING: do not call this method multiple times on the same mimedata object,
         * you can add urls only once. But you can add other things, e.g. images, XML...
         *
         * @param mimeData the QMimeData instance used to drag or copy this URL
         * @param flags set NoTextExport to prevent setting plain/text data into @p mimeData
         */
        void populateMimeData(QMimeData *mimeData,
                              MimeDataFlags flags = DefaultMimeDataFlags) const;

        /**
         * Adds URLs into the given QMimeData.
         *
         * This should add both the KDE-style URLs (eg: desktop:/foo) and
         * the "most local" version of the URLs (eg:
         * file:///home/jbloggs/Desktop/foo) to the mimedata.
         *
         * This method should be called on the KDE-style URLs.
         *
         * @code
         * QMimeData* mimeData = new QMimeData();
         *
         * KUrl::List kdeUrls;
         * kdeUrls << "desktop:/foo";
         * kdeUrls << "desktop:/bar";
         *
         * KUrl::List normalUrls;
         * normalUrls << "file:///home/jbloggs/Desktop/foo";
         * normalUrls << "file:///home/jbloggs/Desktop/bar";
         *
         * kdeUrls.populateMimeData(normalUrls, mimeData);
         * @endcode
         *
         * @param mostLocalUrls the "most local" urls
         * @param mimeData      the mime data object to populate
         * @param flags         set NoTextExport to prevent setting plain/text
         *                      data into @p mimeData.
         * @since 4.2
         */
        void populateMimeData(const KUrl::List &mostLocalUrls,
                              QMimeData *mimeData,
                              MimeDataFlags flags = DefaultMimeDataFlags) const;

        /**
         * Return true if @p mimeData contains URI data
         */
        static bool canDecode(const QMimeData *mimeData);

        /**
         * Return the list of mimeTypes that can be decoded by fromMimeData
         */
        static QStringList mimeDataTypes();

        /**
         * Flags to be used in fromMimeData.
         * @since 4.2.3
         */
        enum DecodeOptions {
            /**
             * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
             * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
             * decode it as local urls. Useful in paste/drop operations that end up calling KIO,
             * so that urls from other users work as well.
             */
            PreferLocalUrls,
            /**
             * When the mimedata contains both KDE-style URLs (eg: desktop:/foo) and
             * the "most local" version of the URLs (eg: file:///home/dfaure/Desktop/foo),
             * decode it as the KDE-style URL. Useful in DnD code e.g. when moving icons,
             * and the kde-style url is used as identifier for the icons.
             */
            PreferKdeUrls
        };

        /**
         * Extract a list of KUrls from the contents of @p mimeData.
         * Decoding will fail if @p mimeData does not contain any URLs, or if at
         * least one extracted URL is not valid.
         * @param mimeData the mime data to extract from; cannot be 0
         * @param decodeOptions options for decoding
         * @return the list of urls
         * @since 4.2.3
         */
        static KUrl::List fromMimeData(const QMimeData *mimeData,
                                       DecodeOptions decodeOptions = PreferKdeUrls);
    };

    /**
     * Constructs an empty URL.
     */
    KUrl();

    /**
     * Usual constructor, to construct from a string.
     * @param urlOrPath An encoded URL or a path.
     */
    KUrl(const QString &urlOrPath);

    /**
     * Constructor taking a char * @p urlOrPath, which is an _encoded_ representation
     * of the URL, exactly like the usual constructor. This is useful when
     * the URL, in its encoded form, is strictly ascii.
     * @param urlOrPath An encoded URL, or a path.
     */
    explicit KUrl(const char *urlOrPath);

    /**
     * Constructor taking a QByteArray @p urlOrPath, which is an _encoded_ representation
     * of the URL, exactly like the usual constructor. This is useful when
     * the URL, in its encoded form, is strictly ascii.
     * @param urlOrPath An encoded URL, or a path.
     */
    explicit KUrl(const QByteArray &urlOrPath);

    /**
     * Copy constructor.
     * @param u the KUrl to copy
     */
    KUrl(const KUrl &u);

    /**
     * Converts from a QUrl.
     * @param u the QUrl
     */
    KUrl(const QUrl &u);

    /**
     * Constructor allowing relative URLs.
     *
     * @param _baseurl The base url.
     * @param _rel_url A relative or absolute URL.
     * If this is an absolute URL then @p _baseurl will be ignored.
     * If this is a relative URL it will be combined with @p _baseurl.
     * Note that _rel_url should be encoded too, in any case.
     * So do NOT pass a path here (use setPath or addPath instead).
     */
    KUrl(const KUrl &_baseurl, const QString &_rel_url);

    /**
     * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath
     *
     * @return The current decoded path. This does not include the query. Can
     *         be QString() if no path is set.
     */
    QString path(AdjustPathOption trailing = LeaveTrailingSlash) const;

    /**
     * @param trailing use to add or remove a trailing slash to/from the local path. see adjustPath
     *
     * @return The current local path. Can
     *   be QString() if no path is set.
     */
    QString toLocalFile(AdjustPathOption trailing = LeaveTrailingSlash) const;

    /// \reimp so that KUrl u; u.setPath(path); implies "file" protocol.
    void setPath(const QString &path);

    /**
     * Resolves "." and ".." components in path.
     * Some servers seem not to like the removal of extra '/'
     * even though it is against the specification in RFC 2396.
     */
    void cleanPath();

    /**
     * Add or remove a trailing slash to/from the path.
     *
     * If the URL has no path, then no '/' is added
     * anyway. And on the other side: If the path is "/", then this
     * character won't be stripped. Reason: "ftp://weis\@host" means something
     * completely different than "ftp://weis\@host/". So adding or stripping
     * the '/' would really alter the URL, while "ftp://host/path" and
     * "ftp://host/path/" mean the same directory.
     *
     * @param trailing  RemoveTrailingSlash strips any trailing '/' and
     *                  AddTrailingSlash adds  a trailing '/' if there is none yet
     */
    void adjustPath(AdjustPathOption trailing);

    /**
     * Checks whether the file is local.
     * @return true if the file is a plain local file (i.e. uses the file protocol
     *   and no hostname, or the local hostname).
     *  When isLocalFile returns true, you can use toLocalFile to read the file contents.
     *  Otherwise you need to use KIO (e.g. KIO::get).
     */
    bool isLocalFile() const;

    /**
     * Adds to the current path.
     * Assumes that the current path is a directory. @p txt is appended to the
     * current path. The function adds '/' if needed while concatenating.
     * This means it does not matter whether the current path has a trailing
     * '/' or not. If there is none, it becomes appended. If @p txt
     * has a leading '/' then this one is stripped.
     *
     * @param txt The text to add. It is considered to be decoded.
     */
    void addPath(const QString &txt);


    /**
     * Sets the filename of the path.
     * In comparison to addPath() this function does not assume that the current
     * path is a directory. This is only assumed if the current path ends with '/'.
     *
     * Any reference is reset.
     *
     * @param txt The filename to be set. It is considered to be decoded. If the
     *             current path ends with '/' then @p txt is just appended, otherwise
     *             all text behind the last '/' in the current path is erased and
     *             @p txt is appended then. It does not matter whether @p txt starts
     *             with '/' or not.
     */
    void setFileName(const QString &txt);

    /**
     * Returns the filename of the path.
     * @param trailing use to add or remove a trailing slash to/from the path. See adjustPath
     * @return The filename of the current path. The returned string is decoded. Null
     *         if there is no file (and thus no path).
     */
    QString fileName(AdjustPathOption trailing = RemoveTrailingSlash) const;

    /**
     * Returns the directory of the path.
     * @param trailing use to add or remove a trailing slash to/from the path. See adjustPath
     * @return The directory part of the current path. Everything between the last and the second last '/'
     *         is returned. For example <tt>file:///hallo/torben/</tt> would return "/hallo/torben/" while
     *         <tt>file:///hallo/torben</tt> would return "hallo/". The returned string is decoded.
     *         QString() is returned when there is no path.
     */
    QString directory(AdjustPathOption trailing = RemoveTrailingSlash) const;

    /**
     * Set the directory to @p dir, leaving the filename empty.
     */
    void setDirectory(const QString &dir);

    /**
     * Returns the URL as string, with all escape sequences intact,
     * encoded in a given charset.
     * This is used in particular for encoding URLs in UTF-8 before using them
     * in a drag and drop operation.
     * Please note that the string returned by url() will include
     * the password of the URL. If you want to show the URL to the
     * user, use prettyUrl().
     *
     * @param trailing use to add or remove a trailing slash to/from the path. See adjustPath
     * @return The complete URL, with all escape sequences intact, encoded
     * in a given charset.
     * @see prettyUrl()
     */
    QString url(AdjustPathOption trailing = LeaveTrailingSlash) const;

    /**
     * Returns the URL as string in human-friendly format.
     * Example:
     * \code
     * http://localhost:8080/test.cgi?test=hello world&name=fred
     * \endcode
     * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath.
     *
     * @return A human readable URL, with no non-necessary encodings/escaped
     * characters. Password will not be shown.
     * @see url()
     */
    QString prettyUrl(AdjustPathOption trailing = LeaveTrailingSlash) const;

    /**
     * Return the URL as a string, which will be either the URL (as prettyUrl
     * would return) or, when the URL is a local file without query or ref,
     * the path.
     * Use this method, to display URLs to the user.
     * You can give the result of pathOrUrl back to the KUrl constructor, it accepts
     * both paths and urls.
     *
     * @param trailing use to add or remove a trailing slash to/from the path. see adjustPath.
     * @return the new KUrl
     * @since 4.2
     */
    QString pathOrUrl(AdjustPathOption trailing = LeaveTrailingSlash) const;

    /**
     * Returns the URL as a string, using the standard conventions for mime data
     * (drag-n-drop or copy-n-paste).
     * Internally used by KUrl::List::fromMimeData, which is probably what you want to use instead.
     */
    QString toMimeDataString() const;

    /**
     * This function is useful to implement the "Up" button in a file manager for example.
     * That means that if you are in file:///home/joe/ and hit the up button you expect to see
     * file:///home. The algorithm tries to go up on the right-most URL.
     * @return a URL that is a level higher
     */
    KUrl upUrl() const;

    KUrl& operator=(const KUrl &_u);

    // Define those, since the constructors are explicit
    KUrl& operator=(const char *_url) { *this = KUrl(_url); return *this; }
    KUrl& operator=(const QByteArray &_url) { *this = KUrl(_url); return *this; }
    KUrl& operator=(const QString &_url) { *this = KUrl(_url); return *this; }

    bool operator==(const KUrl &_u) const;
    bool operator==(const QString &_u) const;
    bool operator!=(const KUrl &_u) const { return !(*this == _u); }
    bool operator!=(const QString &_u) const { return !(*this == _u); }

    /**
     * Converts this KUrl to a QVariant, this allows to use KUrl
     * in QVariant() constructor
     */
    operator QVariant() const;

    /**
     * Compares this url with @p u.
     * @param u the URL to compare this one with.
     * @param trailing use to add or remove a trailing slash to/from the path. See adjustPath
     * @return True if both urls are the same. If at least one of the urls is invalid,
     * false is returned.
     * @see operator==. This function should be used if you want to
     * set additional options, like ignoring trailing '/' characters.
     */
    bool equals(const KUrl &u, AdjustPathOption trailing = LeaveTrailingSlash) const;

    /**
     * Checks whether the given URL is parent of this URL.
     * For instance, ftp://host/dir/ is a parent of ftp://host/dir/subdir/subsubdir/.
     * @return true if this url is a parent of @p u (or the same URL as @p u)
     *
     */
    bool isParentOf(const KUrl &u) const;

    /**
     * Creates a KUrl object from a QString representing an absolute path.
     * KUrl url( somePath ) is almost the same, but this method is more explicit,
     * avoids the path-or-url detection in the KUrl constructor, and parses
     * "abc:def" as a filename, not as URL.
     *
     * @param text the path
     * @return the new KUrl
     */
    static KUrl fromPath(const QString &text);

    /**
     * Adds URL data into the given QMimeData.
     *
     * By default, populateMimeData also exports the URL as plain text, for e.g. dropping
     * onto a text editor.
     * But in some cases this might not be wanted, e.g. if adding other mime data
     * which provides better plain text data.
     *
     * WARNING: do not call this method multiple times, use KUrl::List::populateMimeData instead.
     *
     * @param mimeData the QMimeData instance used to drag or copy this URL
     * @param flags set NoTextExport to prevent setting plain/text data into @p mimeData
     */
    void populateMimeData(QMimeData* mimeData,
                          MimeDataFlags flags = DefaultMimeDataFlags) const;


    /**
     * Convenience function.
     *
     * Returns whether '_url' is likely to be a "relative" URL instead of
     * an "absolute" URL.
     *
     * This is mostly meant for KUrl(url, relativeUrl).
     *
     * If you are looking for the notion of "relative path" (foo) vs "absolute path" (/foo),
     * use QUrl::isRelative() instead.
     * Indeed, isRelativeUrl() returns true for the string "/foo" since it doesn't contain a protocol,
     * while KUrl("/foo").isRelative() is false since the KUrl constructor turns it into file:///foo.
     * The two methods basically test the same thing, but this one takes a string (which is faster)
     * while the class method requires a QUrl/KUrl which gives a more expected result, given
     * the "magic" in the KUrl constructor.
     *
     * @param _url URL to examine
     * @return true when the URL is likely to be "relative", false otherwise.
     */
    static bool isRelativeUrl(const QString &_url);

    /**
     * Convenience function
     *
     * Returns a "relative URL" based on @p base_url that points to @p url.
     *
     * If no "relative URL" can be created, e.g. because the protocol
     * and/or hostname differ between @p base_url and @p url an absolute
     * URL is returned.
     * Note that if @p base_url represents a directory, it should contain
     * a trailing slash.
     * @param base_url the URL to derive from
     * @param url new URL
     * @see adjustPath()
     */
    static QString relativeUrl(const KUrl &base_url, const KUrl &url);

    /**
     * Convenience function
     *
     * Returns a relative path based on @p base_dir that points to @p path.
     * @param base_dir the base directory to derive from
     * @param path the new target directory
     * @param isParent A pointer to a boolean which, if provided, will be set to reflect
     * whether @p path has @p base_dir is a parent dir.
     */
    static QString relativePath(const QString &base_dir, const QString &path, bool *isParent = nullptr);

    /**
     * Compatibility methods, do not use
     */
    inline QString protocol() const { return scheme().toLower(); };
    inline bool hasUser() const { return !userName().isEmpty(); };
    inline bool hasPass() const { return !password().isEmpty(); };
    inline bool hasHost() const { return !host().isEmpty(); };
    inline bool hasPath() const { return !path().isEmpty(); };

private:
    QString toString() const; // forbidden, use url(), prettyUrl(), or pathOrUrl() instead.
    operator QString() const; // forbidden, use url(), prettyUrl(), or pathOrUrl() instead.
};

Q_DECLARE_METATYPE(KUrl)
Q_DECLARE_METATYPE(KUrl::List)

KDECORE_EXPORT uint qHash(const KUrl& kurl);

#endif
