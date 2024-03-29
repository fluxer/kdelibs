/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009 David Faure <faure@kde.org>

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

/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Generated: Thu Mar  5 16:05:28 EST 1998
 */

#include <config.h>
#include <config-prefix.h>
#include <config-kstandarddirs.h>

#include "kstandarddirs.h"
#include "kdebug.h"
#include "kcomponentdata.h"
#include "kshell.h"
#include "kde_file.h"
#include "klocale.h"

#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <mutex>

#define case_sensitivity Qt::CaseSensitive

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

// same as KPATH_SEPARATOR
static const QString s_pathseparator = QString::fromLatin1(":");
static const QChar s_pathseparatorchar = QChar::fromLatin1(KPATH_SEPARATOR);

static QString readEnvPath(const char *env)
{
    const QByteArray c_path = qgetenv(env);
    if (c_path.isEmpty())
        return QString();
    return QDir::fromNativeSeparators(QFile::decodeName(c_path));
}

// split path using : as delimiters
static inline QStringList splitPath(const QString &path)
{
    const int len = path.length();
    QString token;
    QStringList tokens;

    for(int index = 0; index < len; index++) {
        if (path.at(index) == s_pathseparatorchar) {
            tokens.append(token);
            token.clear();
        } else {
            token += path.at(index);
        }
    }
    if (!token.isEmpty()) {
        tokens.append(token);
    }

    return tokens;
}

static QString checkExecutable( const QString& path, bool ignoreExecBit )
{
    QFileInfo info( path );
    if( info.exists() && info.isSymLink() )
        info = QFileInfo( info.canonicalFilePath() );
    if( info.exists() && ( ignoreExecBit || info.isExecutable() ) && info.isFile() ) {
        // return absolute path, but without symlinks resolved in order to prevent
        // problems with executables that work differently depending on name they are
        // run as (for example gunzip)
        QFileInfo absoluteinfo (path);
        absoluteinfo.makeAbsolute();
        return absoluteinfo.filePath();
    }
    // kDebug() << "checkExecutable(): failed, returning empty string";
    return QString();
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    KDE_struct_stat buff;
    if ((KDE::access(file, R_OK) == 0) && (KDE::stat(file, &buff) == 0) && (S_ISREG(buff.st_mode))) {
        hash = hash + static_cast<quint32>(buff.st_ctime);
    }
    return hash;
}

static void lookupDirectory(const QString& path, const QString &relPart,
                            const QRegExp &regexp,
                            QStringList& list,
                            QStringList& relList,
                            bool recursive, bool unique)
{
    const QString pattern = regexp.pattern();
    if (recursive || pattern.contains(QLatin1Char('?')) || pattern.contains(QLatin1Char('*')))
    {
        if (path.isEmpty()) //for sanity
            return;
        // We look for a set of files.
        DIR *dp = opendir( QFile::encodeName(path));
        if (!dp)
            return;

        assert(path.endsWith(QLatin1Char('/')));

        struct dirent *ep;

        while( ( ep = readdir( dp ) ) != 0L )
        {
            QString fn( QFile::decodeName(ep->d_name));
            if (fn == QString::fromLatin1(".") || fn == QString::fromLatin1("..") || fn.at(fn.length() - 1) == QLatin1Char('~'))
                continue;

            if (!recursive && !regexp.exactMatch(fn))
                continue; // No match

            bool isDir;
            bool isReg;

            QString pathfn = path + fn;
#ifdef HAVE_DIRENT_D_TYPE
            isDir = ep->d_type == DT_DIR;
            isReg = ep->d_type == DT_REG;

            if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
            {
                KDE_struct_stat buff;
                if ( KDE::stat( pathfn, &buff ) != 0 ) {
                    kDebug() << "Error stat'ing " << pathfn << " : " << ::strerror(errno);
                    continue; // Couldn't stat (e.g. no read permissions)
                }
                isReg = S_ISREG (buff.st_mode);
                isDir = S_ISDIR (buff.st_mode);
            }

            if ( recursive ) {
                if ( isDir ) {
                    lookupDirectory(pathfn + QLatin1Char('/'), relPart + fn + QLatin1Char('/'), regexp, list, relList, recursive, unique);
                }
                if (!regexp.exactMatch(fn))
                    continue; // No match
            }
            if ( isReg )
            {
                if (!unique || !relList.contains(relPart + fn, case_sensitivity))
                {
                    list.append( pathfn );
                    relList.append( relPart + fn );
                }
            }
        }
        closedir( dp );
    }
    else
    {
        // We look for a single file.
        QString fn = pattern;
        QString pathfn = path + fn;
        KDE_struct_stat buff;
        if ( KDE::stat( pathfn, &buff ) != 0 )
            return; // File not found
        if ( S_ISREG( buff.st_mode))
        {
            if (!unique || !relList.contains(relPart + fn, case_sensitivity))
            {
                list.append( pathfn );
                relList.append( relPart + fn );
            }
        }
    }
}

static void lookupPrefix(const QString& prefix, const QString& relpath,
                         const QString& relPart,
                         const QRegExp &regexp,
                         QStringList& list,
                         QStringList& relList,
                         bool recursive, bool unique)
{
    if (relpath.isEmpty()) {
        if (recursive)
            Q_ASSERT(prefix != QLatin1String("/")); // we don't want to recursively list the whole disk!
        lookupDirectory(prefix, relPart, regexp, list,
                        relList, recursive, unique);
        return;
    }
    QString path;
    QString rest;

    int slash = relpath.indexOf(QLatin1Char('/'));
    if (slash < 0)
        rest = relpath.left(relpath.length() - 1);
    else {
        path = relpath.left(slash);
        rest = relpath.mid(slash + 1);
    }

    if (prefix.isEmpty()) //for sanity
        return;
    // what does this assert check ?
    assert(prefix.endsWith(QLatin1Char('/')));
    if (path.contains(QLatin1Char('*')) || path.contains(QLatin1Char('?'))) {

        QRegExp pathExp(path, Qt::CaseSensitive, QRegExp::Wildcard);

        DIR *dp = opendir( QFile::encodeName(prefix) );
        if (!dp) {
            return;
        }

        struct dirent *ep;

        while( ( ep = readdir( dp ) ) != 0L )
        {
            QString fn( QFile::decodeName(ep->d_name));
            if (fn == QLatin1String(".") || fn == QLatin1String("..") || fn.at(fn.length() - 1) == QLatin1Char('~'))
                continue;

            if ( !pathExp.exactMatch(fn) )
                continue; // No match
            QString rfn = relPart+fn;
            fn = prefix + fn;

            bool isDir = false;
#ifdef HAVE_DIRENT_D_TYPE
            isDir = ep->d_type == DT_DIR;

            if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
            {
                KDE_struct_stat buff;
                if ( KDE::stat( fn, &buff ) != 0 ) {
                    kDebug() << "Error stat'ing " << fn << " : " << ::strerror(errno);
                    continue; // Couldn't stat (e.g. no read permissions)
                }
                isDir = S_ISDIR (buff.st_mode);
            }
            if ( isDir )
                lookupPrefix(fn + QLatin1Char('/'), rest, rfn + QLatin1Char('/'), regexp, list, relList, recursive, unique);
        }

        closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + QLatin1Char('/'), rest,
                     relPart + path + QLatin1Char('/'), regexp, list,
                     relList, recursive, unique);
    }
}

class KStandardDirs::KStandardDirsPrivate
{
public:
    QStringList xdgdata_prefixes;
    QStringList xdgconf_prefixes;
    QStringList m_prefixes;

    // Directory dictionaries
    QMap<QByteArray, QStringList> m_absolutes; // For each resource type, the list of absolute paths, from most local (most priority) to most global
    QMap<QByteArray, QStringList> m_relatives; // Same with relative paths
    // The search path is "all relative paths" < "all absolute paths", from most priority to least priority.

    // Caches (protected by mutex in const methods, cf ctor docu)
    QMap<QByteArray, QStringList> m_dircache;
    std::recursive_mutex m_cacheMutex; // resourceDirs is recursive
};

/*
    If you add a new resource type here, make sure to
        1) update the KStandardDirs class documentation
        2) update the list in kde-config.cpp
*/
static const struct ResourcesTblData {
    const char* const type;
    const char* const relativename;
} ResourcesTbl[] = {
    { "data\0", "share\0" },
    { "icon\0", "share/icons\0" },
    { "config\0", "share/config\0" },
    { "pixmap\0", "share/pixmaps\0" },
    { "sound\0", "share/sounds\0" },
    { "locale\0", "share/locale\0" },
    { "services\0", "share/kde4/services\0" },
    { "servicetypes\0", "share/kde4/servicetypes\0" },
    { "wallpaper\0", "share/wallpapers\0" },
    { "templates\0", "share/templates\0" },
    { "exe\0", "bin\0" },
    { "module\0", "%lib/kde4\0" },
    { "qtplugins\0", "%lib/kde4/plugins\0" },
    { "xdgdata-apps\0", "applications\0" },
    { "xdgdata-icon\0", "icons\0" },
    { "xdgdata-pixmap\0", "pixmaps\0" },
    { "xdgdata-dirs\0", "desktop-directories\0" },
    { "xdgdata-mime\0", "mime\0" },
    { "xdgconf-menu\0", "menus\0" },
    { "xdgconf-autostart\0", "autostart\0" }
};
static const qint16 ResourcesTblSize = sizeof(ResourcesTbl) / sizeof(ResourcesTblData);

KStandardDirs::KStandardDirs()
    : d(new KStandardDirsPrivate())
{
    QStringList kdedirList;
    // begin KDEDIRS
    QString kdedirs = readEnvPath("KDEDIRS");

    if (!kdedirs.isEmpty()) {
        kdedirList = splitPath(kdedirs);
    }
    kdedirList.append(installPath("kdedir"));

    QString execPrefix(QFile::decodeName(EXEC_INSTALL_PREFIX));
    if (!execPrefix.isEmpty() && !kdedirList.contains(execPrefix, case_sensitivity))
        kdedirList.append(execPrefix);

    // We treat root differently to prevent a "su" shell messing up the
    // file permissions in the user's home directory.
    QString localKdeDir = readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME");
    if (!localKdeDir.isEmpty()) {
        if (!localKdeDir.endsWith(QLatin1Char('/')))
            localKdeDir += QLatin1Char('/');
    } else {
        // TODO KDE5: make localKdeDir equal to localXdgDir (which is determined further below and
        // defaults to ~/.config) + '/' + $KDECONFIG (which would default to e.g. "KDE")
        // This would mean ~/.config/KDE/ by default, more xdg-compliant.
        localKdeDir =  QDir::homePath() + QLatin1Char('/') + QString::fromLatin1(KDE_DEFAULT_HOME) + QLatin1Char('/');
    }

    if (localKdeDir != QLatin1String("-/")) {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

    foreach (const QString &it, kdedirList) {
        addPrefix(KShell::tildeExpand(it));
    }
    // end KDEDIRS

    // begin XDG_CONFIG_XXX
    QStringList xdgdirList;
    QString xdgdirs = readEnvPath("XDG_CONFIG_DIRS");
    if (!xdgdirs.isEmpty()) {
        xdgdirList = splitPath(xdgdirs);
    } else {
        xdgdirList.clear();
        xdgdirList.append(QString::fromLatin1("/etc/xdg"));
        xdgdirList.append(QFile::decodeName(SYSCONF_INSTALL_DIR "/xdg"));
    }

    QString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty()) {
        if (!localXdgDir.endsWith(QLatin1Char('/'))) {
            localXdgDir += QLatin1Char('/');
        }
    } else {
        localXdgDir = QDir::homePath() + QString::fromLatin1("/.config/");
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    foreach (const QString &it, xdgdirList) {
        addXdgConfigPrefix(KShell::tildeExpand(it));
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    QStringList kdedirDataDirs;
    foreach (const QString &it, kdedirList) {
        if (!it.endsWith(QLatin1Char('/'))) {
            kdedirDataDirs.append(it + QString::fromLatin1("/share/"));
        } else {
            kdedirDataDirs.append(it + QString::fromLatin1("share/"));
        }
    }

    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty()) {
        xdgdirList = splitPath(xdgdirs);
        // Ensure the kdedirDataDirs are in there too,
        // otherwise resourceDirs() will add kdedir/share/applications/kde4
        // as returned by installPath(), and that's incorrect.
        Q_FOREACH(const QString& dir, kdedirDataDirs) {
            if (!xdgdirList.contains(dir, case_sensitivity)) {
                xdgdirList.append(dir);
            }
        }
    } else {
        xdgdirList = kdedirDataDirs;
        xdgdirList.append(QString::fromLatin1("/usr/local/share/"));
        xdgdirList.append(QString::fromLatin1("/usr/share/"));
        xdgdirList.append(QString::fromLatin1("/share/"));
    }

    localXdgDir = readEnvPath("XDG_DATA_HOME");
    if (!localXdgDir.isEmpty()) {
        if (localXdgDir[localXdgDir.length()-1] != QLatin1Char('/')) {
            localXdgDir += QLatin1Char('/');
        }
    } else {
        localXdgDir = QDir::homePath() + QString::fromLatin1("/.local/share/");
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    foreach (const QString &it, xdgdirList) {
        addXdgDataPrefix(KShell::tildeExpand(it));
    }
    // end XDG_DATA_XXX

    addResourceDir("lib", QString::fromLatin1(LIB_INSTALL_DIR "/"), true);
    addResourceDir("exe", QString::fromLatin1(LIBEXEC_INSTALL_DIR), true);
    // NOTE: QStandardPaths::writableLocation() should create the base directory
    addResourceDir("cache", QStandardPaths::writableLocation(QStandardPaths::CacheLocation), true);
    addResourceDir("tmp", QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation), true);

    addResourceType("qtplugins", "lib", QString::fromLatin1("plugins"));

    for (int i = 0; i < ResourcesTblSize; i++) {
        addResourceType(ResourcesTbl[i].type, nullptr, QString::fromLatin1(ResourcesTbl[i].relativename), true);
    }

    addResourceType("autostart", "xdgconf-autostart", QString::fromLatin1("/")); // merge them, start with xdg autostart
    addResourceType("autostart", nullptr, QString::fromLatin1("share/autostart")); // KDE ones are higher priority
}

KStandardDirs::~KStandardDirs()
{
    delete d;
}

QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    list.reserve(ResourcesTblSize + 4);
    for (int i = 0; i < ResourcesTblSize; i++) {
        list.append(QString::fromLatin1(ResourcesTbl[i].type));
    }
    // Those are added manually by the constructor
    list.append(QString::fromLatin1("lib"));
    list.append(QString::fromLatin1("tmp"));
    list.append(QString::fromLatin1("cache"));
    // Those are handled by installPath()
    list.append(QString::fromLatin1("include"));

    // If you add anything here, make sure kde-config.cpp has a description for it.

    return list;
}

void KStandardDirs::addPrefix(const QString &_dir)
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->m_prefixes.contains(dir, case_sensitivity)) {
        d->m_prefixes.append(dir);
        std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
        d->m_dircache.clear();
    }
}

void KStandardDirs::addXdgConfigPrefix(const QString &_dir)
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->xdgconf_prefixes.contains(dir, case_sensitivity)) {
        d->xdgconf_prefixes.append(dir);
        std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
        d->m_dircache.clear();
    }
}

void KStandardDirs::addXdgDataPrefix(const QString &_dir)
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->xdgdata_prefixes.contains(dir, case_sensitivity)) {
        d->xdgdata_prefixes.append(dir);
        std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
        d->m_dircache.clear();
    }
}

QString KStandardDirs::kfsstnd_prefixes()
{
    return d->m_prefixes.join(s_pathseparator);
}

QString KStandardDirs::kfsstnd_xdg_conf_prefixes()
{
    return d->xdgconf_prefixes.join(s_pathseparator);
}

QString KStandardDirs::kfsstnd_xdg_data_prefixes()
{
    return d->xdgdata_prefixes.join(s_pathseparator);
}

bool KStandardDirs::addResourceType(const char *type,
                                    const char *basetype,
                                    const QString &relativename,
                                    bool priority)
{
    if (relativename.isEmpty()) {
        return false;
    }

    QString copy = relativename;
    if (basetype) {
        copy = QLatin1Char('%') + QString::fromLatin1(basetype) + QLatin1Char('/') + relativename;
    }
    if (!copy.endsWith(QLatin1Char('/'))) {
        copy += QLatin1Char('/');
    }

    std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
    QByteArray typeBa = type;
    QStringList& rels = d->m_relatives[typeBa]; // find or insert
    if (!rels.contains(copy, case_sensitivity)) {
        if (priority) {
            rels.prepend(copy);
        } else {
            rels.append(copy);
        }
        // clean the caches
        d->m_dircache.remove(typeBa);
        return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir(const char *type,
                                   const QString &absdir,
                                   bool priority)
{
    if (absdir.isEmpty() || !type) {
        return false;
    }

    // find or insert entry in the map
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != QLatin1Char('/')) {
        copy += QLatin1Char('/');
    }

    QByteArray typeBa = type;
    QStringList &paths = d->m_absolutes[typeBa];
    if (!paths.contains(copy, case_sensitivity)) {
        if (priority) {
            paths.prepend(copy);
        } else {
            paths.append(copy);
        }
        // clean the caches
        std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
        d->m_dircache.remove(typeBa);
        return true;
    }
    return false;
}

QString KStandardDirs::findResource(const char *type,
                                    const QString &filename) const
{
    if (!QDir::isRelativePath(filename)) {
        return !KGlobal::hasLocale() ? filename // absolute dirs are absolute dirs, right? :-/
                                     : KGlobal::locale()->localizedFilePath(filename); // -- almost.
    }

#if 0
    kDebug() << "Find resource: " << type;
    foreach (const QString &it, d->m_prefixes) {
        kDebug() << "Prefix: " << it;
    }
#endif

    const QString dir = findResourceDir(type, filename);
    if (dir.isEmpty()) {
        return dir;
    }
    return !KGlobal::hasLocale() ? dir + filename
                                 : KGlobal::locale()->localizedFilePath(dir + filename);
}

quint32 KStandardDirs::calcResourceHash(const char *type,
                                        const QString &filename,
                                        SearchOptions options) const
{
    quint32 hash = 0;

    if (!QDir::isRelativePath(filename)) {
        // absolute dirs are absolute dirs, right? :-/
        return updateHash(filename, hash);
    }

    foreach (const QString &it, resourceDirs(type)) {
        hash = updateHash(it + filename, hash);
        if (!( options & Recursive ) && hash) {
            return hash;
        }
    }
    return hash;
}


QStringList KStandardDirs::findDirs(const char *type,
                                    const QString &reldir) const
{
    QDir testdir;
    QStringList list;
    if (!QDir::isRelativePath(reldir)) {
        testdir.setPath(reldir);
        if (testdir.exists()) {
            if (reldir.endsWith(QLatin1Char('/'))) {
                list.append(reldir);
            } else {
                list.append(reldir + QLatin1Char('/'));
            }
        }
        return list;
    }

    foreach (const QString &it, resourceDirs(type)) {
        testdir.setPath(it + reldir);
        if (testdir.exists()) {
            list.append(testdir.absolutePath() + QLatin1Char('/'));
        }
    }

    return list;
}

QString KStandardDirs::findResourceDir(const char *type,
                                       const QString &filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
        kWarning() << "filename for type " << type << " in KStandardDirs::findResourceDir is not supposed to be empty!!";
        return QString();
    }
#endif

    foreach (const QString &it, resourceDirs(type)) {
        if (KStandardDirs::exists(it + filename)) {
            return it;
        }
    }

#ifndef NDEBUG
    if(false && strcmp(type, "locale"))
        kDebug() << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\".";
#endif

    return QString();
}

bool KStandardDirs::exists(const QString &fullPath)
{
    QFileInfo fileinfo(fullPath);
    if (!fileinfo.isReadable()) {
        return false;
    } else if (!fullPath.endsWith(QLatin1Char('/'))) {
        return !fileinfo.isDir() && fileinfo.exists();
    }
    return fileinfo.isDir() && fileinfo.exists();
}

QStringList
KStandardDirs::findAllResources(const char *type,
                                const QString &filter,
                                SearchOptions options,
                                QStringList &relList) const
{
    QString filterPath;
    QString filterFile;

    if (!filter.isEmpty()) {
        const int slash = filter.lastIndexOf(QLatin1Char('/'));
        if (slash < 0) {
            filterFile = filter;
        } else {
            filterPath = filter.left(slash + 1);
            filterFile = filter.mid(slash + 1);
        }
    }

    QStringList candidates;
    if (!QDir::isRelativePath(filter)) { // absolute path
        candidates << QString::fromLatin1("/");
        filterPath = filterPath.mid(1);
    } else {
        candidates = resourceDirs(type);
    }

    if (filterFile.isEmpty()) {
        filterFile = QString(QLatin1Char('*'));
    }

    QRegExp regExp(filterFile, Qt::CaseSensitive, QRegExp::Wildcard);

    QStringList list;
    foreach (const QString& candidate, candidates) {
        lookupPrefix(candidate, filterPath, QString(), regExp, list,
                     relList, options & Recursive, options & NoDuplicates);
    }

    return list;
}

QStringList
KStandardDirs::findAllResources(const char *type,
                                const QString &filter,
                                SearchOptions options) const
{
    QStringList relList;
    return findAllResources(type, filter, options, relList);
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString KStandardDirs::realPath(const QString &dirname)
{
    if (dirname.isEmpty() || (dirname.size() == 1 && dirname.at(0) == QLatin1Char('/')))
       return dirname;

    if (dirname.at(0) != QLatin1Char('/')) {
        qWarning("KStandardDirs::realPath called with a relative path '%s', please fix", qPrintable(dirname));
        return dirname;
    }

    char realpath_buffer[PATH_MAX + 1];
    ::memset(realpath_buffer, 0, PATH_MAX + 1);

    /* If the path contains symlinks, get the real name */
    if (::realpath( QFile::encodeName(dirname).constData(), realpath_buffer) != 0) {
        // success, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = '/';
        realpath_buffer[len+1] = 0;
        return QFile::decodeName(realpath_buffer);
    }

    // Does not exist yet; resolve symlinks in parent dirs then.
    // This ensures that once the directory exists, it will still be resolved
    // the same way, so that the general rule that KStandardDirs always returns
    // canonical paths stays true, and app code can compare paths more easily.
    QString dir = dirname;
    if (!dir.endsWith(QLatin1Char('/')))
        dir += QLatin1Char('/');
    QString relative;
    while (!KStandardDirs::exists(dir)) {
        //qDebug() << "does not exist:" << dir;
        const int pos = dir.lastIndexOf(QLatin1Char('/'), -2);
        Q_ASSERT(pos >= 0); // what? even "/" doesn't exist?
        relative.prepend(dir.mid(pos+1)); // keep "subdir/"
        dir = dir.left(pos+1);
        Q_ASSERT(dir.endsWith(QLatin1Char('/')));
    }
    Q_ASSERT(!relative.isEmpty()); // infinite recursion ahead
    if (!relative.isEmpty()) {
        //qDebug() << "done, resolving" << dir << "and adding" << relative;
        dir = KStandardDirs::realPath(dir) + relative;
    }
    return dir;
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::realFilePath(const QString &filename)
{
    char realpath_buffer[PATH_MAX + 1];
    ::memset(realpath_buffer, 0, PATH_MAX + 1);

    /* If the path contains symlinks, get the real name */
    if (::realpath( QFile::encodeName(filename).constData(), realpath_buffer) != 0) {
        // success, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }

    return filename;
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);

    QMap<QByteArray, QStringList>::const_iterator dirCacheIt = d->m_dircache.constFind(type);

    QStringList candidates;

    if (dirCacheIt != d->m_dircache.constEnd()) {
        //qDebug() << this << "resourceDirs(" << type << "), in cache already";
        candidates = *dirCacheIt;
    } else { // filling cache
        //qDebug() << this << "resourceDirs(" << type << "), not in cache";
        QDir testdir;

        const QStringList dirs = d->m_relatives.value(type);
        const QString typeInstallPath = installPath(type); // could be empty
        const QString installdir = typeInstallPath.isEmpty() ? QString() : KStandardDirs::realPath(typeInstallPath);
        const QString installprefix = installPath("kdedir");
        if (!dirs.isEmpty()) {
            bool local = true;

            foreach (const QString &it, dirs) {
                if (it.startsWith(QLatin1Char('%'))) {
                    // grab the "data" from "%data/apps"
                    const int pos = it.indexOf(QLatin1Char('/'));
                    QByteArray rel = it.mid(1, pos - 1).toUtf8();
                    QString rest = it.mid(pos + 1);
                    foreach (const QString &it2, resourceDirs(rel.constData())) {
                        const QString path = KStandardDirs::realPath(it2 + rest);
                        testdir.setPath(path);
                        if ((local || testdir.exists()) && !candidates.contains(path, case_sensitivity)) {
                            candidates.append(path);
                        }
                        local = false;
                    }
                }
            }

            const QStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0) {
                prefixList = &(d->xdgdata_prefixes);
            } else if (strncmp(type, "xdgconf-", 8) == 0) {
                prefixList = &(d->xdgconf_prefixes);
            } else {
                prefixList = &d->m_prefixes;
            }

            for (QStringList::ConstIterator pit = prefixList->begin(); pit != prefixList->end(); ++pit) {
                if((*pit).compare(installprefix, case_sensitivity) != 0 || installdir.isEmpty()) {
                    foreach (const QString &it, dirs) {
                        if (it.startsWith(QLatin1Char('%')))
                            continue;
                        const QString path = KStandardDirs::realPath(*pit + it);
                        testdir.setPath(path);
                        if ((local || testdir.exists()) && !candidates.contains(path, case_sensitivity))
                            candidates.append(path);
                    }
                    local = false;
                } else {
                    // we have a custom install path, so use this instead of <installprefix>/<relative dir>
                    testdir.setPath(installdir);
                    if (testdir.exists() && ! candidates.contains(installdir, case_sensitivity))
                        candidates.append(installdir);
                }
            }
        }

        // make sure we find the path where it's installed
        if (!installdir.isEmpty()) {
            bool ok = true;
            foreach (const QString &s, candidates) {
                if (installdir.startsWith(s, case_sensitivity)) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                candidates.append(installdir);
        }

        foreach (const QString &it, d->m_absolutes.value(type)) {
            testdir.setPath(it);
            if (testdir.exists()) {
                const QString filename = KStandardDirs::realPath(it);
                if (!candidates.contains(filename, case_sensitivity)) {
                    candidates.append(filename);
                }
            }
        }

        // Insert result into the cache for next time.
        //kDebug() << this << "Inserting" << type << candidates << "into dircache";
        d->m_dircache.insert(type, candidates);
    }

#if 0
    kDebug() << "found dirs for resource" << type << ":" << candidates;
#endif

    return candidates;
}

QStringList KStandardDirs::systemPaths(const QString &pstr)
{
    QStringList tokens;

    if (pstr.isEmpty()) {
        tokens = splitPath(QString::fromLocal8Bit(qgetenv("PATH")));
    } else {
        tokens = splitPath(pstr);
    }

    QStringList exePaths;
    for (int i = 0; i < tokens.count(); i++) {
        exePaths << KShell::tildeExpand( tokens.at(i) );
    }

    return exePaths;
}

QString KStandardDirs::findExe(const QString &appname,
                               const QString &pstr,
                               SearchOptions options)
{
    // kDebug() << "findExe(" << appname << ", pstr, " << ignoreExecBit << ") called";

    // absolute or relative path?
    if (appname.contains(QDir::separator())) {
        // kDebug() << "findExe(): absolute path given";
        return checkExecutable(appname, options & IgnoreExecBit);
    }

    // kDebug) << "findExe(): relative path given";

    QString p = installPath("libexec") + appname;
    QString result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        // kDebug() << "findExe(): returning " << result;
        return result;
    }

    //kDebug() << "findExe(): checking system paths";
    foreach (const QString &it, systemPaths(pstr)) {
        p = it + QLatin1Char('/') + appname;

        // Check for executable in this tokenized path
        result = checkExecutable(p, options & IgnoreExecBit);
        if (!result.isEmpty()) {
            //kDebug() << "findExe(): returning " << result;
            return result;
        }
    }

    // Not found in PATH, look into the KDE-specific bin dir ("exe" resource)
    p = installPath("exe") + appname;
    result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        // kDebug() << "findExe(): returning " << result;
        return result;
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    // kDebug() << "findExe(): failed, nothing matched";
    return QString();
}

QString KStandardDirs::findRootExe( const QString& appname,
                                    const QString& pstr,
                                    SearchOptions options )
{
    QStringList exePaths = systemPaths( pstr );
    static const QStringList rootPaths = QStringList()
        << QString::fromLatin1("/sbin")
        << QString::fromLatin1("/usr/sbin")
        << QString::fromLatin1("/usr/local/sbin")
        << QString::fromLatin1(KDEDIR "/sbin");

    foreach (const QString &rootPath, rootPaths) {
        if (exePaths.contains(rootPath) || !QDir(rootPath).exists()) {
            continue;
        }
        exePaths << rootPath;
    }

    return findExe(appname, exePaths.join(s_pathseparator), options);
}

int KStandardDirs::findAllExe(QStringList &list, const QString &appname,
                              const QString &pstr, SearchOptions options)
{
    list.clear();

    foreach (const QString &it, systemPaths(pstr)) {
        QString p = it + QLatin1Char('/') + appname;
        QFileInfo info(p);
        if (info.exists() && ((options & IgnoreExecBit) || info.isExecutable()) && info.isFile()) {
            list.append(p);
        }
    }

    return list.count();
}

QString KStandardDirs::saveLocation(const char *type,
                                    const QString &suffix,
                                    bool create) const
{
    const QStringList dirs = resourceDirs(type); // Generate tmp|cache resource.
    if (dirs.isEmpty()) {
        qFatal("KStandardDirs: The resource type %s is not registered", type);
        return QString();
    }

    QString fullPath = KStandardDirs::realPath(dirs.first()) + suffix;
    if (!fullPath.endsWith(QLatin1Char('/')))
        fullPath += QLatin1Char('/');

    if (create && KStandardDirs::makeDir(fullPath, 0700)) {
        std::lock_guard<std::recursive_mutex> lock(d->m_cacheMutex);
        d->m_dircache.remove(type);
    }

    return fullPath;
}

QString KStandardDirs::relativeLocation(const char *type, const QString &absPath) const
{
    QString fullPath = absPath;
    int i = absPath.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        fullPath = realFilePath(absPath); // Normalize
    }

    foreach (const QString &it, resourceDirs(type)) {
        if (fullPath.startsWith(it, case_sensitivity)) {
            return fullPath.mid(it.length());
        }
    }
    return absPath;
}


bool KStandardDirs::makeDir(const QString &dir, int mode)
{
    // we want an absolute path
    if (QDir::isRelativePath(dir))
        return false;

    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != QLatin1Char('/'))
        target += QLatin1Char('/');

    QString base;
    uint i = 1;

    while (i < len) {
        KDE_struct_stat st;
        int pos = target.indexOf(QLatin1Char('/'), i);
        base += target.mid(i - 1, pos - i + 1);
        QByteArray baseEncoded = QFile::encodeName(base);
        // bail out if we encountered a problem
        if (KDE_stat(baseEncoded, &st) != 0)
        {
            // Directory does not exist....
            // Or maybe a dangling symlink ?
            if (KDE_lstat(baseEncoded, &st) == 0)
                (void)unlink(baseEncoded); // try removing

            if (KDE_mkdir(baseEncoded, static_cast<mode_t>(mode)) != 0) {
                baseEncoded.prepend( "trying to create local folder " );
                perror(baseEncoded.constData());
                return false; // Couldn't create it :-(
            }
        }
        i = pos + 1;
    }
    return true;
}

QString KStandardDirs::localkdedir() const
{
    // Return the prefix to use for saving
    return d->m_prefixes.first();
}

QString KStandardDirs::localxdgdatadir() const
{
    // Return the prefix to use for saving
    return d->xdgdata_prefixes.first();
}

QString KStandardDirs::localxdgconfdir() const
{
    // Return the prefix to use for saving
    return d->xdgconf_prefixes.first();
}


// just to make code more readable without macros
QString KStandardDirs::locate(const char *type,
                              const QString &filename, const KComponentData &cData)
{
    return cData.dirs()->findResource(type, filename);
}

QString KStandardDirs::locateLocal(const char *type,
                                   const QString &filename, const KComponentData &cData)
{
    return locateLocal(type, filename, true, cData);
}

QString KStandardDirs::locateLocal(const char *type,
                                   const QString &filename, bool createDir,
                                   const KComponentData &cData)
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    const int slash = filename.lastIndexOf(QLatin1Char('/')) + 1;
    if (!slash) { // only one filename
        return cData.dirs()->saveLocation(type, QString(), createDir) + filename;
    }

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return cData.dirs()->saveLocation(type, dir, createDir) + file;
}

bool KStandardDirs::checkAccess(const QString &pathname, int mode)
{
    int accessOK = KDE::access(pathname, mode);
    if (accessOK == 0) {
        return true;  // OK, I can really access the file
    }

    // if we want to write the file would be created. Check, if the
    // user may write to the directory to create the file.
    if ((mode & W_OK) == 0) {
        return false; // Check for write access is not part of mode => bail out
    }

    if (!KDE::access(pathname, F_OK)) { // if it already exists
        return false;
    }

    //strip the filename (everything until '/' from the end
    QString dirName(pathname);
    int pos = dirName.lastIndexOf(QLatin1Char('/'));
    if (pos == -1) {
        return false;   // No path in argument. This is evil, we won't allow this
    } else if (pos == 0) { // don't turn e.g. /root into an empty string
        pos = 1;
    }

    dirName.truncate(pos); // strip everything starting from the last '/'

    accessOK = KDE::access(dirName, W_OK);
    // -?- Can I write to the accessed diretory
    if (accessOK == 0) {
        return true; // Yes
    }
    return false; // No
}

