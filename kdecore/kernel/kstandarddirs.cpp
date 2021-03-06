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

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kdebug.h"
#include "kcomponentdata.h"
#include "kshell.h"
#include "kuser.h"
#include "kde_file.h"
#include "klocale.h"

#include <config.h>
#include <config-prefix.h>
#include <config-kstandarddirs.h>

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <QtCore/QMutex>
#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QCache>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtNetwork/QHostInfo>

#define case_sensitivity Qt::CaseSensitive

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

class KStandardDirs::KStandardDirsPrivate
{
public:
    KStandardDirsPrivate(KStandardDirs* qq)
        : m_restrictionsActive(false),
          m_checkRestrictions(true),
          m_cacheMutex(QMutex::Recursive), // resourceDirs is recursive
          q(qq)
    { }

    bool hasDataRestrictions(const QString &relPath) const;
    QStringList resourceDirs(const char* type, const QString& subdirForRestrictions);
    void createSpecialResource(const char*);
    bool exists(const QString &fullPath);
    QString realPath(const QString &dirname);

    bool m_restrictionsActive : 1;
    bool m_checkRestrictions : 1;
    QMap<QByteArray, bool> m_restrictions;

    QStringList xdgdata_prefixes;
    QStringList xdgconf_prefixes;
    QStringList m_prefixes;

    // Directory dictionaries
    QMap<QByteArray, QStringList> m_absolutes; // For each resource type, the list of absolute paths, from most local (most priority) to most global
    QMap<QByteArray, QStringList> m_relatives; // Same with relative paths
    // The search path is "all relative paths" < "all absolute paths", from most priority to least priority.

    // Caches (protected by mutex in const methods, cf ctor docu)
    QMap<QByteArray, QStringList> m_dircache;
    QMap<QByteArray, QString> m_savelocations;
    QMutex m_cacheMutex;

    KStandardDirs* q;
};

/* If you add a new resource type here, make sure to
 * 1) regenerate using "generate_string_table.pl types < tmpfile" with the data below in tmpfile.
 * 2) update the KStandardDirs class documentation
 * 3) update the list in kde-config.cpp

data
share/apps
html
share/doc/HTML
icon
share/icons
config
share/config
pixmap
share/pixmaps
sound
share/sounds
locale
share/locale
services
share/kde4/services
servicetypes
share/kde4/servicetypes
wallpaper
share/wallpapers
templates
share/templates
exe
bin
module
%lib/kde4
qtplugins
%lib/kde4/plugins
kcfg
share/config.kcfg
emoticons
share/emoticons
xdgdata-apps
applications
xdgdata-icon
icons
xdgdata-pixmap
pixmaps
xdgdata-dirs
desktop-directories
xdgdata-mime
mime
xdgconf-menu
menus
xdgconf-autostart
autostart
*/

static const char types_string[] =
    "data\0"
    "share/apps\0"
    "html\0"
    "share/doc/HTML\0"
    "icon\0"
    "share/icons\0"
    "config\0"
    "share/config\0"
    "pixmap\0"
    "share/pixmaps\0"
    "sound\0"
    "share/sounds\0"
    "locale\0"
    "share/locale\0"
    "services\0"
    "share/kde4/services\0"
    "servicetypes\0"
    "share/kde4/servicetypes\0"
    "wallpaper\0"
    "share/wallpapers\0"
    "templates\0"
    "share/templates\0"
    "exe\0"
    "bin\0"
    "module\0"
    "%lib/kde4\0"
    "qtplugins\0"
    "%lib/kde4/plugins\0"
    "kcfg\0"
    "share/config.kcfg\0"
    "emoticons\0"
    "share/emoticons\0"
    "xdgdata-apps\0"
    "applications\0"
    "xdgdata-icon\0"
    "icons\0"
    "xdgdata-pixmap\0"
    "pixmaps\0"
    "xdgdata-dirs\0"
    "desktop-directories\0"
    "xdgdata-mime\0"
    "mime\0"
    "xdgconf-menu\0"
    "menus\0"
    "xdgconf-autostart\0"
    "autostart\0"
    "\0";

static const int types_indices[] = {
       0,    5,   16,   21,   36,   41,   53,   60,
      73,   80,   94,  100,  113,  120,  133,  142,
     162,  175,  199,  209,  226,  236,  252,  256,
     260,  267,  277,  287,  305,  310,  328,  338,
     354,  367,  380,  393,  399,  414,  422,  435,
     455,  468,  473,  486,  492,  510,   -1
};

static void tokenize(QStringList& token, const QString& str,
                     const QString& delim);

KStandardDirs::KStandardDirs()
    : d(new KStandardDirsPrivate(this))
{
    addKDEDefaults();
}

KStandardDirs::~KStandardDirs()
{
    delete d;
}

bool KStandardDirs::isRestrictedResource(const char *type, const QString& relPath) const
{
    if (!d->m_restrictionsActive)
        return false;

    if (d->m_restrictions.value(type, false))
        return true;

    if (strcmp(type, "data")==0 && d->hasDataRestrictions(relPath))
        return true;

    return false;
}

bool KStandardDirs::KStandardDirsPrivate::hasDataRestrictions(const QString &relPath) const
{
    QString key;
    const int i = relPath.indexOf(QLatin1Char('/'));
    if (i != -1)
        key = QString::fromLatin1("data_") + relPath.left(i);
    else
        key = QString::fromLatin1("data_") + relPath;

    return m_restrictions.value(key.toLatin1(), false);
}


QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types_indices[i] != -1; i += 2)
        list.append(QLatin1String(types_string + types_indices[i]));
    // Those are added manually by addKDEDefaults
    list.append(QString::fromLatin1("lib"));
    //list.append(QString::fromLatin1("home")); // undocumented on purpose, said Waldo in r113855.

    // Those are handled by resourceDirs() itself
    list.append(QString::fromLatin1("socket"));
    list.append(QString::fromLatin1("tmp"));
    list.append(QString::fromLatin1("cache"));
    // Those are handled by installPath()
    list.append(QString::fromLatin1("include"));

    // If you add anything here, make sure kde-config.cpp has a description for it.

    return list;
}

static void priorityAdd(QStringList &prefixes, const QString& dir, bool priority)
{
    if (priority && !prefixes.isEmpty())
    {
        // Add in front but behind $KDEHOME
        QStringList::iterator it = prefixes.begin();
        ++it;
        prefixes.insert(it, dir);
    }
    else
    {
        prefixes.append(dir);
    }
}

void KStandardDirs::addPrefix( const QString& _dir )
{
    addPrefix(_dir, false);
}

void KStandardDirs::addPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->m_prefixes.contains(dir, case_sensitivity)) {
        priorityAdd(d->m_prefixes, dir, priority);
        d->m_dircache.clear();
    }
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir )
{
    addXdgConfigPrefix(_dir, false);
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->xdgconf_prefixes.contains(dir, case_sensitivity)) {
        priorityAdd(d->xdgconf_prefixes, dir, priority);
        d->m_dircache.clear();
    }
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir )
{
    addXdgDataPrefix(_dir, false);
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
        return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != QLatin1Char('/'))
        dir += QLatin1Char('/');

    if (!d->xdgdata_prefixes.contains(dir, case_sensitivity)) {
        priorityAdd(d->xdgdata_prefixes, dir, priority);
        d->m_dircache.clear();
    }
}

QString KStandardDirs::kfsstnd_prefixes()
{
    return d->m_prefixes.join(QString(QLatin1Char(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_conf_prefixes()
{
    return d->xdgconf_prefixes.join(QString(QLatin1Char(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_data_prefixes()
{
    return d->xdgdata_prefixes.join(QString(QLatin1Char(KPATH_SEPARATOR)));
}


bool KStandardDirs::addResourceType( const char *type,
                                     const char *basetype,
                                     const QString& relativename,
                                     bool priority )
{
    if (relativename.isEmpty())
        return false;

    QString copy = relativename;
    if (basetype)
        copy = QLatin1Char('%') + QString::fromLatin1(basetype) + QLatin1Char('/') + relativename;

    if (!copy.endsWith(QLatin1Char('/')))
        copy += QLatin1Char('/');

    QByteArray typeBa = type;
    QStringList& rels = d->m_relatives[typeBa]; // find or insert

    if (!rels.contains(copy, case_sensitivity)) {
        if (priority)
            rels.prepend(copy);
        else
            rels.append(copy);
        // clean the caches
        d->m_dircache.remove(typeBa);
        d->m_savelocations.remove(typeBa);
        return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const char *type,
                                    const QString& absdir,
                                    bool priority)
{
    if (absdir.isEmpty() || !type)
      return false;
    // find or insert entry in the map
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != QLatin1Char('/'))
        copy += QLatin1Char('/');

    QByteArray typeBa = type;
    QStringList &paths = d->m_absolutes[typeBa];
    if (!paths.contains(copy, case_sensitivity)) {
        if (priority)
            paths.prepend(copy);
        else
            paths.append(copy);
        // clean the caches
        d->m_dircache.remove(typeBa);
        d->m_savelocations.remove(typeBa);
        return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
                                     const QString& filename ) const
{
    if (!QDir::isRelativePath(filename))
        return !KGlobal::hasLocale() ? filename // absolute dirs are absolute dirs, right? :-/
                                     : KGlobal::locale()->localizedFilePath(filename); // -- almost.

#if 0
    kDebug(180) << "Find resource: " << type;
    foreach (const QString &it, d->m_prefixes) {
        kDebug(180) << "Prefix: " << it;
    }
#endif

    const QString dir = findResourceDir(type, filename);
    if (dir.isEmpty())
        return dir;
    return !KGlobal::hasLocale() ? dir + filename
                                 : KGlobal::locale()->localizedFilePath(dir + filename);
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    KDE_struct_stat buff;
    if ((KDE::access(file, R_OK) == 0) && (KDE::stat(file, &buff) == 0) && (S_ISREG(buff.st_mode))) {
        hash = hash + static_cast<quint32>(buff.st_ctime);
    }
    return hash;
}

quint32 KStandardDirs::calcResourceHash( const char *type,
                                         const QString& filename,
                                         SearchOptions options ) const
{
    quint32 hash = 0;

    if (!QDir::isRelativePath(filename))
    {
        // absolute dirs are absolute dirs, right? :-/
        return updateHash(filename, hash);
    }
    QStringList candidates = d->resourceDirs(type, filename);

    foreach ( const QString& candidate, candidates )
    {
        hash = updateHash(candidate + filename, hash);
        if (  !( options & Recursive ) && hash ) {
            return hash;
        }
    }
    return hash;
}


QStringList KStandardDirs::findDirs( const char *type,
                                     const QString& reldir ) const
{
    QDir testdir;
    QStringList list;
    if (!QDir::isRelativePath(reldir))
    {
        testdir.setPath(reldir);
        if (testdir.exists())
        {
            if (reldir.endsWith(QLatin1Char('/')))
                list.append(reldir);
            else
                list.append(reldir+QLatin1Char('/'));
        }
        return list;
    }

    const QStringList candidates = d->resourceDirs(type, reldir);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absolutePath() + QLatin1Char('/'));
    }

    return list;
}

QString KStandardDirs::findResourceDir( const char *type,
                                        const QString& filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
        kWarning() << "filename for type " << type << " in KStandardDirs::findResourceDir is not supposed to be empty!!";
        return QString();
    }
#endif

    const QStringList candidates = d->resourceDirs(type, filename);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (exists(*it + filename)) {
            return *it;
        }
    }

#ifndef NDEBUG
    if(false && strcmp(type, "locale"))
        kDebug(180) << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\".";
#endif

    return QString();
}

bool KStandardDirs::exists(const QString &fullPath) const
{
    return d->exists(fullPath);
}

bool KStandardDirs::KStandardDirsPrivate::exists(const QString &fullPath)
{
    QFileInfo fileinfo(fullPath);
    if (!fileinfo.isReadable()) {
        return false;
    } else if (!fullPath.endsWith(QLatin1Char('/'))) {
        return !fileinfo.isDir() && fileinfo.exists();
    } else {
        return fileinfo.isDir() && fileinfo.exists();
    }
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
                    kDebug(180) << "Error stat'ing " << pathfn << " : " << perror;
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

            bool isDir;

#ifdef HAVE_DIRENT_D_TYPE
            isDir = ep->d_type == DT_DIR;

            if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
            {
                QString pathfn = path + fn;
                KDE_struct_stat buff;
                if ( KDE::stat( fn, &buff ) != 0 ) {
                    kDebug(180) << "Error stat'ing " << fn << " : " << perror;
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

QStringList
KStandardDirs::findAllResources( const char *type,
                                 const QString& filter,
                                 SearchOptions options,
                                 QStringList &relList) const
{
    QString filterPath;
    QString filterFile;

    if ( !filter.isEmpty() )
    {
        int slash = filter.lastIndexOf(QLatin1Char('/'));
        if (slash < 0) {
            filterFile = filter;
        } else {
            filterPath = filter.left(slash + 1);
            filterFile = filter.mid(slash + 1);
        }
    }

    QStringList candidates;
    if ( !QDir::isRelativePath(filter) ) // absolute path
    {
        candidates << QString::fromLatin1("/");
        filterPath = filterPath.mid(1);
    }
    else
    {
        candidates = d->resourceDirs(type, filter);
    }

    if (filterFile.isEmpty()) {
        filterFile = QString(QLatin1Char('*'));
    }

    QRegExp regExp(filterFile, Qt::CaseSensitive, QRegExp::Wildcard);

    QStringList list;
    foreach ( const QString& candidate, candidates )
    {
        lookupPrefix(candidate, filterPath, QString(), regExp, list,
                     relList, options & Recursive, options & NoDuplicates);
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
                                 const QString& filter,
                                 SearchOptions options ) const
{
    QStringList relList;
    return findAllResources(type, filter, options, relList);
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::KStandardDirsPrivate::realPath(const QString &dirname)
{
    if (dirname.isEmpty() || (dirname.size() == 1 && dirname.at(0) == QLatin1Char('/')))
       return dirname;

    if (dirname.at(0) != QLatin1Char('/')) {
        qWarning("realPath called with a relative path '%s', please fix", qPrintable(dirname));
        return dirname;
    }

    char realpath_buffer[PATH_MAX + 1];
    memset(realpath_buffer, 0, PATH_MAX + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(dirname).constData(), realpath_buffer) != 0) {
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
    while (!exists(dir)) {
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
        dir = realPath(dir) + relative;
    }
    return dir;
}

QString
KStandardDirs::realPath(const QString &dirname) const
{
    return d->realPath(dirname);
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
    memset(realpath_buffer, 0, PATH_MAX + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(filename).constData(), realpath_buffer) != 0) {
        // success, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }

    return filename;
}


void KStandardDirs::KStandardDirsPrivate::createSpecialResource(const char *type)
{
    const QString localkdedir = m_prefixes.first();
    QString dir = localkdedir + QString::fromLatin1(type) + QLatin1Char('-') + QHostInfo::localHostName();
    char link[1024];
    link[1023] = 0;
    int result = readlink(QFile::encodeName(dir).constData(), link, 1023);
    bool relink = (result == -1) && (errno == ENOENT);
    if (result > 0)
    {
        link[result] = 0;
        if (!QDir::isRelativePath(QFile::decodeName(link)))
        {
            KDE_struct_stat stat_buf;
            int res = KDE::lstat(QFile::decodeName(link), &stat_buf);
            if ((res == -1) && (errno == ENOENT))
            {
                relink = true;
            }
            else if ((res == -1) || (!S_ISDIR(stat_buf.st_mode)))
            {
                fprintf(stderr, "Error: \"%s\" is not a directory.\n", link);
                relink = true;
            }
            else if (stat_buf.st_uid != getuid())
            {
                fprintf(stderr, "Error: \"%s\" is owned by uid %d instead of uid %d.\n", link, stat_buf.st_uid, getuid());
                relink = true;
            }
        }
    }
    if (relink)
    {
        QString srv = findExe(QLatin1String("lnusertemp"), installPath("libexec"));
        if (srv.isEmpty())
            srv = findExe(QLatin1String("lnusertemp"));
        if (!srv.isEmpty())
        {
            if (system(QByteArray(QFile::encodeName(srv) + ' ' + type)) == -1) {
                fprintf(stderr, "Error: unable to launch lnusertemp command" );
            }
            result = readlink(QFile::encodeName(dir).constData(), link, 1023);
        }
    }
    if (result > 0)
    {
        link[result] = 0;
        if (link[0] == '/')
            dir = QFile::decodeName(link);
        else
            dir = QDir::cleanPath(dir + QFile::decodeName(link));
    }
    q->addResourceDir(type, dir + QLatin1Char('/'), false);
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    return d->resourceDirs(type, QString());
}

QStringList KStandardDirs::KStandardDirsPrivate::resourceDirs(const char* type, const QString& subdirForRestrictions)
{
    QMutexLocker lock(&m_cacheMutex);
    const bool dataRestrictionActive = m_restrictionsActive
                                       && (strcmp(type, "data") == 0)
                                       && hasDataRestrictions(subdirForRestrictions);

    QMap<QByteArray, QStringList>::const_iterator dirCacheIt = m_dircache.constFind(type);

    QStringList candidates;

    if (dirCacheIt != m_dircache.constEnd() && !dataRestrictionActive) {
        //qDebug() << this << "resourceDirs(" << type << "), in cache already";
        candidates = *dirCacheIt;
    }
    else // filling cache
    {
        //qDebug() << this << "resourceDirs(" << type << "), not in cache";
        if (strcmp(type, "socket") == 0)
            createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
            createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
            createSpecialResource(type);

        QDir testdir;

        bool restrictionActive = false;
        if (m_restrictionsActive) {
            if (dataRestrictionActive)
                restrictionActive = true;
            if (m_restrictions.value("all", false))
                restrictionActive = true;
            else if (m_restrictions.value(type, false))
                restrictionActive = true;
        }

        const QStringList dirs = m_relatives.value(type);
        const QString typeInstallPath = installPath(type); // could be empty
        const QString installdir = typeInstallPath.isEmpty() ? QString() : realPath(typeInstallPath);
        const QString installprefix = installPath("kdedir");
        if (!dirs.isEmpty())
        {
            bool local = true;

            for (QStringList::ConstIterator it = dirs.constBegin();
                 it != dirs.constEnd(); ++it)
            {
                if ((*it).startsWith(QLatin1Char('%'))) {
                    // grab the "data" from "%data/apps"
                    const int pos = (*it).indexOf(QLatin1Char('/'));
                    QString rel = (*it).mid(1, pos - 1);
                    QString rest = (*it).mid(pos + 1);
                    const QStringList basedirs = resourceDirs(rel.toUtf8().constData(), subdirForRestrictions);
                    for (QStringList::ConstIterator it2 = basedirs.begin();
                         it2 != basedirs.end(); ++it2)
                    {
                        const QString path = realPath( *it2 + rest );
                        testdir.setPath(path);
                        if ((local || testdir.exists()) && !candidates.contains(path, case_sensitivity))
                            candidates.append(path);
                        local = false;
                    }
                }
            }

            const QStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0)
                prefixList = &(xdgdata_prefixes);
            else if (strncmp(type, "xdgconf-", 8) == 0)
                prefixList = &(xdgconf_prefixes);
            else
                prefixList = &m_prefixes;

            for (QStringList::ConstIterator pit = prefixList->begin();
                 pit != prefixList->end();
                 ++pit)
            {
            if((*pit).compare(installprefix, case_sensitivity) != 0 || installdir.isEmpty())
            {
                    for (QStringList::ConstIterator it = dirs.constBegin();
                         it != dirs.constEnd(); ++it)
                    {
                        if ((*it).startsWith(QLatin1Char('%')))
                            continue;
                        const QString path = realPath( *pit + *it );
                        testdir.setPath(path);
                        if (local && restrictionActive)
                            continue;
                        if ((local || testdir.exists()) && !candidates.contains(path, case_sensitivity))
                            candidates.append(path);
                    }
                    local = false;
                }
            else
            {
                    // we have a custom install path, so use this instead of <installprefix>/<relative dir>
                testdir.setPath(installdir);
                    if(testdir.exists() && ! candidates.contains(installdir, case_sensitivity))
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

        const QStringList absDirs = m_absolutes.value(type);
        for (QStringList::ConstIterator it = absDirs.constBegin();
             it != absDirs.constEnd(); ++it)
        {
            testdir.setPath(*it);
            if (testdir.exists()) {
                const QString filename = realPath( *it );
                if (!candidates.contains(filename, case_sensitivity)) {
                    candidates.append(filename);
                }
            }
        }

        // Insert result into the cache for next time.
        // Exception: data_subdir restrictions are per-subdir, so we can't store such results
        if (!dataRestrictionActive) {
            //kDebug() << this << "Inserting" << type << candidates << "into dircache";
            m_dircache.insert(type, candidates);
        }
    }

#if 0
    kDebug(180) << "found dirs for resource" << type << ":" << candidates;
#endif

    return candidates;
}


QStringList KStandardDirs::systemPaths( const QString& pstr )
{
    QStringList tokens;
    QString p = pstr;

    if( p.isEmpty() )
    {
        p = QString::fromLocal8Bit( qgetenv( "PATH" ) );
    }

    QString delimiters(QLatin1Char(KPATH_SEPARATOR));
    delimiters += QLatin1Char('\b');
    tokenize( tokens, p, delimiters );

    QStringList exePaths;

    // split path using : or \b as delimiters
    for( int i = 0; i < tokens.count(); i++ )
    {
        exePaths << KShell::tildeExpand( tokens[ i ] );
    }

    return exePaths;
}


static QString checkExecutable( const QString& path, bool ignoreExecBit )
{
    QFileInfo info( path );
    QFileInfo orig = info;
    if( info.exists() && info.isSymLink() )
        info = QFileInfo( info.canonicalFilePath() );
    if( info.exists() && ( ignoreExecBit || info.isExecutable() ) && info.isFile() ) {
        // return absolute path, but without symlinks resolved in order to prevent
        // problems with executables that work differently depending on name they are
        // run as (for example gunzip)
        orig.makeAbsolute();
        return orig.filePath();
    }
    //kDebug(180) << "checkExecutable(): failed, returning empty string";
    return QString();
}

QString KStandardDirs::findExe( const QString& appname,
                                const QString& pstr,
                                SearchOptions options )
{
    //kDebug(180) << "findExe(" << appname << ", pstr, " << ignoreExecBit << ") called";

    QFileInfo info;

    // absolute or relative path?
    if (appname.contains(QDir::separator()))
    {
        //kDebug(180) << "findExe(): absolute path given";
        QString path = checkExecutable(appname, options & IgnoreExecBit);
        return path;
    }

    //kDebug(180) << "findExe(): relative path given";

    QString p = installPath("libexec") + appname;
    QString result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result;
        return result;
    }

    //kDebug(180) << "findExe(): checking system paths";
    const QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + QLatin1Char('/');
        p += appname;

        // Check for executable in this tokenized path
        result = checkExecutable(p, options & IgnoreExecBit);
        if (!result.isEmpty()) {
            //kDebug(180) << "findExe(): returning " << result;
            return result;
        }
    }

    // Not found in PATH, look into the KDE-specific bin dir ("exe" resource)
    p = installPath("exe");
    p += appname;
    result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result;
        return result;
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    //kDebug(180) << "findExe(): failed, nothing matched";
    return QString();
}

QString KStandardDirs::findRootExe( const QString& appname,
                                    const QString& pstr,
                                    SearchOptions options )
{
    QStringList exePaths = systemPaths( pstr );
    static const QStringList rootPaths = QStringList()
        << QLatin1String("/sbin")
        << QLatin1String("/usr/sbin")
        << QLatin1String("/usr/local/sbin")
        << QLatin1String(KDEDIR "/sbin");

    foreach (const QString &rootPath, rootPaths) {
        if (exePaths.contains(rootPath) || !QDir(rootPath).exists()) {
            continue;
        }
        exePaths << rootPath;
    }

    return findExe(appname, exePaths.join(QString(QLatin1Char(KPATH_SEPARATOR))), options);
}

int KStandardDirs::findAllExe( QStringList& list, const QString& appname,
                               const QString& pstr, SearchOptions options )
{
    QFileInfo info;
    QString p;
    list.clear();

    const QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + QLatin1Char('/');
        p += appname;


        info.setFile( p );

        if( info.exists() && ( ( options & IgnoreExecBit ) || info.isExecutable())
            && info.isFile() ) {
            list.append( p );
        }
    }

    return list.count();
}

static void tokenize(QStringList& tokens, const QString& str,
                    const QString& delim)
{
    const int len = str.length();
    QString token;

    for(int index = 0; index < len; index++) {
        if (delim.contains(str[index])) {
            tokens.append(token);
            token.clear();
        } else {
            token += str[index];
        }
    }
    if (!token.isEmpty()) {
        tokens.append(token);
    }
}


QString KStandardDirs::saveLocation(const char *type,
                                    const QString& suffix,
                                    bool create) const
{
    QMutexLocker lock(&d->m_cacheMutex);
    QString path = d->m_savelocations.value(type);
    if (path.isEmpty())
    {
        QStringList dirs = d->m_relatives.value(type);
        if (dirs.isEmpty() && (
                (strcmp(type, "socket") == 0) ||
                (strcmp(type, "tmp") == 0) ||
                (strcmp(type, "cache") == 0) ))
        {
            (void) resourceDirs(type); // Generate socket|tmp|cache resource.
            dirs = d->m_relatives.value(type); // Search again.
        }
        if (!dirs.isEmpty())
        {
            path = dirs.first();

            if (path.startsWith(QLatin1Char('%'))) {
                // grab the "data" from "%data/apps"
                const int pos = path.indexOf(QLatin1Char('/'));
                QString rel = path.mid(1, pos - 1);
                QString rest = path.mid(pos + 1);
                QString basepath = saveLocation(rel.toUtf8().constData());
                path = basepath + rest;
            } else

                // Check for existence of typed directory + suffix
                if (strncmp(type, "xdgdata-", 8) == 0) {
                    path = realPath( localxdgdatadir() + path ) ;
                } else if (strncmp(type, "xdgconf-", 8) == 0) {
                    path = realPath( localxdgconfdir() + path );
                } else {
                    path = realPath( localkdedir() + path );
                }
        }
        else {
            dirs = d->m_absolutes.value(type);
            if (dirs.isEmpty()) {
                qFatal("KStandardDirs: The resource type %s is not registered", type);
            } else {
                path = realPath(dirs.first());
            }
        }

        d->m_savelocations.insert(type, path.endsWith(QLatin1Char('/')) ? path : path + QLatin1Char('/'));
    }
    QString fullPath = path + suffix;

    KDE_struct_stat st;
    if (KDE::stat(fullPath, &st) != 0 || !(S_ISDIR(st.st_mode))) {
        if(!create) {
#ifndef NDEBUG
            // Too much noise from kbuildsycoca4 -- it's fine if this happens from KConfig
            // when parsing global files without a local equivalent.
            //kDebug(180) << QString("save location %1 doesn't exist").arg(fullPath);
#endif
            return fullPath;
        }
        if(!makeDir(fullPath, 0700)) {
            return fullPath;
        }
        d->m_dircache.remove(type);
    }
    if (!fullPath.endsWith(QLatin1Char('/')))
        fullPath += QLatin1Char('/');
    return fullPath;
}

// KDE5: make the method const
QString KStandardDirs::relativeLocation(const char *type, const QString &absPath)
{
    QString fullPath = absPath;
    int i = absPath.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        fullPath = realFilePath(absPath); // Normalize
    }

    const QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (fullPath.startsWith(*it, case_sensitivity)) {
            return fullPath.mid((*it).length());
        }
    }
    return absPath;
}


bool KStandardDirs::makeDir(const QString& dir, int mode)
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

    while( i < len )
    {
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

static QString readEnvPath(const char *env)
{
    QByteArray c_path;
    c_path = qgetenv(env);
    if (c_path.isEmpty())
        return QString();
    return QDir::fromNativeSeparators(QFile::decodeName(c_path));
}

#ifdef Q_OS_LINUX
static QString executablePrefix()
{
    char path_buffer[PATH_MAX + 1];
    path_buffer[PATH_MAX] = 0;
    int length = readlink ("/proc/self/exe", path_buffer, PATH_MAX);
    if (length == -1)
        return QString();

    path_buffer[length] = '\0';

    QString path = QFile::decodeName(path_buffer);

    if(path.isEmpty())
        return QString();

    int pos = path.lastIndexOf(QLatin1Char('/')); // Skip filename
    if(pos <= 0)
        return QString();
    pos = path.lastIndexOf(QLatin1Char('/'), pos - 1); // Skip last directory
    if(pos <= 0)
        return QString();

    return path.left(pos);
}
#endif

void KStandardDirs::addResourcesFrom_krcdirs()
{
    QString localFile = QDir::currentPath() + QLatin1String("/.krcdirs");
    if (!QFile::exists(localFile))
        return;

    QSettings iniFile(localFile, QSettings::IniFormat);
#ifndef QT_KATIE
    const QStringList resources = iniFile.allKeys();
#else
    const QStringList resources = iniFile.keys();
#endif
    foreach(QString key, resources)
    {
        QDir path(iniFile.value(key).toString());
        if (!path.exists())
            continue;

        key.replace(QLatin1String("KStandardDirs/"), QLatin1String(""));
        if(path.makeAbsolute())
            addResourceDir(key.toLatin1(), path.path(), false);
    }
}

void KStandardDirs::addKDEDefaults()
{
    addResourcesFrom_krcdirs();

    QStringList kdedirList;
    // begin KDEDIRS
    QString kdedirs = readEnvPath("KDEDIRS");

    if (!kdedirs.isEmpty())
    {
        tokenize(kdedirList, kdedirs, QString(QLatin1Char(KPATH_SEPARATOR)));
    }
    kdedirList.append(installPath("kdedir"));

    QString execPrefix(QFile::decodeName(EXEC_INSTALL_PREFIX));
    if (!execPrefix.isEmpty() && !kdedirList.contains(execPrefix, case_sensitivity))
        kdedirList.append(execPrefix);
#ifdef Q_OS_LINUX
    const QString linuxExecPrefix = executablePrefix();
    if ( !linuxExecPrefix.isEmpty() )
        kdedirList.append( linuxExecPrefix );
#endif

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

    foreach (const QString it, kdedirList) {
        const QString dir = KShell::tildeExpand(it);
        addPrefix(dir);
    }
    // end KDEDIRS

    // begin XDG_CONFIG_XXX
    QStringList xdgdirList;
    QString xdgdirs = readEnvPath("XDG_CONFIG_DIRS");
    if (!xdgdirs.isEmpty())
    {
        tokenize(xdgdirList, xdgdirs, QString(QLatin1Char(KPATH_SEPARATOR)));
    }
    else
    {
        xdgdirList.clear();
        xdgdirList.append(QString::fromLatin1("/etc/xdg"));
        xdgdirList.append(QFile::decodeName(SYSCONF_INSTALL_DIR "/xdg"));
    }

    QString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty()) {
        if (!localXdgDir.endsWith(QLatin1Char('/')))
            localXdgDir += QLatin1Char('/');
    } else {
        localXdgDir = QDir::homePath() + QString::fromLatin1("/.config/");
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.constBegin();
         it != xdgdirList.constEnd(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    QStringList kdedirDataDirs;
    foreach (const QString it, kdedirList) {
        if (!it.endsWith(QLatin1Char('/'))) {
            kdedirDataDirs.append(it + QLatin1String("/share/"));
        } else {
            kdedirDataDirs.append(it + QLatin1String("share/"));
        }
    }

    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty()) {
        tokenize(xdgdirList, xdgdirs, QString(QLatin1Char(KPATH_SEPARATOR)));
        // Ensure the kdedirDataDirs are in there too,
        // otherwise resourceDirs() will add kdedir/share/applications/kde4
        // as returned by installPath(), and that's incorrect.
        Q_FOREACH(const QString& dir, kdedirDataDirs) {
            if (!xdgdirList.contains(dir, case_sensitivity))
                xdgdirList.append(dir);
        }
    } else {
        xdgdirList = kdedirDataDirs;
        xdgdirList.append(QString::fromLatin1("/usr/local/share/"));
        xdgdirList.append(QString::fromLatin1("/usr/share/"));
        xdgdirList.append(QString::fromLatin1("/share/"));
    }

    localXdgDir = readEnvPath("XDG_DATA_HOME");
    if (!localXdgDir.isEmpty())
    {
        if (localXdgDir[localXdgDir.length()-1] != QLatin1Char('/'))
            localXdgDir += QLatin1Char('/');
    }
    else
    {
        localXdgDir = QDir::homePath() + QLatin1String("/.local/share/");
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.constBegin();
         it != xdgdirList.constEnd(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgDataPrefix(dir);
    }
    // end XDG_DATA_XXX

    addResourceDir("lib", QLatin1String(LIB_INSTALL_DIR "/"), true);
    addResourceDir("exe", QLatin1String(LIBEXEC_INSTALL_DIR), true );

    addResourceType("qtplugins", "lib", QLatin1String("plugins"));

    uint index = 0;
    while (types_indices[index] != -1) {
        addResourceType(types_string + types_indices[index], 0,
            QLatin1String(types_string + types_indices[index+1]), true);
        index+=2;
    }

    addResourceDir("home", QDir::homePath(), false);

    addResourceType("autostart", "xdgconf-autostart", QLatin1String("/")); // merge them, start with xdg autostart
    addResourceType("autostart", NULL, QLatin1String("share/autostart")); // KDE ones are higher priority
}

static QStringList lookupProfiles(const QString &mapFile)
{
    QStringList profiles;

    if (mapFile.isEmpty() || !QFile::exists(mapFile))
    {
        profiles << QString::fromLatin1("default");
        return profiles;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (!pw)
    {
        profiles << QString::fromLatin1("default");
        return profiles; // Not good
    }

    QByteArray user = pw->pw_name;

    gid_t sup_gids[512];
    int sup_gids_nr = getgroups(512, sup_gids);

    KConfig mapCfgFile(mapFile);
    KConfigGroup mapCfg(&mapCfgFile, "Users");
    if (mapCfg.hasKey(user.constData()))
    {
        profiles = mapCfg.readEntry(user.constData(), QStringList());
        return profiles;
    }

    const KConfigGroup generalGrp(&mapCfgFile, "General");
    const QStringList groups = generalGrp.readEntry("groups", QStringList());

    const KConfigGroup groupsGrp(&mapCfgFile, "Groups");

    for( QStringList::ConstIterator it = groups.begin();
         it != groups.end(); ++it )
    {
        QByteArray grp = (*it).toUtf8();
        // Check if user is in this group
        struct group *grp_ent = getgrnam(grp);
        if (!grp_ent) continue;
        gid_t gid = grp_ent->gr_gid;
        if (pw->pw_gid == gid)
        {
            // User is in this group --> add profiles
            profiles += groupsGrp.readEntry(*it, QStringList());
        }
        else
        {
            for(int i = 0; i < sup_gids_nr; i++)
            {
                if (sup_gids[i] == gid)
                {
                    // User is in this group --> add profiles
                    profiles += groupsGrp.readEntry(*it, QStringList());
                    break;
                }
            }
        }
    }

    if (profiles.isEmpty())
        profiles << QString::fromLatin1("default");
    return profiles;
}

extern bool kde_kiosk_admin;

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (!d->m_checkRestrictions) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    const int configdirs = resourceDirs("config").count();

    // reading the prefixes in
    QString group = QLatin1String("Directories");
    KConfigGroup cg(config, group);

    QString kioskAdmin = cg.readEntry("kioskAdmin");
    if (!kioskAdmin.isEmpty() && !kde_kiosk_admin)
    {
        int i = kioskAdmin.indexOf(QLatin1Char(':'));
        QString user = kioskAdmin.left(i);
        QString host = kioskAdmin.mid(i+1);

        KUser thisUser;
        if ((user == thisUser.loginName()) &&
            (host.isEmpty() || (host == QHostInfo::localHostName())))
        {
            kde_kiosk_admin = true;
        }
    }

    bool readProfiles = true;

    if (kde_kiosk_admin && !qgetenv("KDE_KIOSK_NO_PROFILES").isEmpty())
        readProfiles = false;

    QString userMapFile = cg.readEntry("userProfileMapFile");
    QString profileDirsPrefix = cg.readEntry("profileDirsPrefix");
    if (!profileDirsPrefix.isEmpty() && !profileDirsPrefix.endsWith(QLatin1Char('/')))
        profileDirsPrefix.append(QLatin1Char('/'));

    QStringList profiles;
    if (readProfiles)
        profiles = lookupProfiles(userMapFile);
    QString profile;

    bool priority = false;
    while(true)
    {
        KConfigGroup cg(config, group);
        const QStringList list = cg.readEntry("prefixes", QStringList());
        foreach (const QString it, list) {
            addPrefix(it, priority);
            addXdgConfigPrefix(it + QLatin1String("/etc/xdg"), priority);
            addXdgDataPrefix(it + QLatin1String("/share"), priority);
        }
        // If there are no prefixes defined, check if there is a directory
        // for this profile under <profileDirsPrefix>
        if (list.isEmpty() && !profile.isEmpty() && !profileDirsPrefix.isEmpty())
        {
            QString dir = profileDirsPrefix + profile;
            addPrefix(dir, priority);
            addXdgConfigPrefix(dir + QLatin1String("/etc/xdg"), priority);
            addXdgDataPrefix(dir + QLatin1String("/share"), priority);
        }

        // iterating over all entries in the group Directories
        // to find entries that start with dir_$type
        const QMap<QString, QString> entries = config->entryMap(group);
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
                it2 != entries.end(); ++it2)
        {
            const QString key = it2.key();
            if (key.startsWith(QLatin1String("dir_"))) {
                // generate directory list, there may be more than 1.
                const QStringList dirs = (*it2).split(QString(QLatin1Char(',')));
                QStringList::ConstIterator sIt(dirs.begin());
                QString resType = key.mid(4);
                for (; sIt != dirs.end(); ++sIt)
                {
                    addResourceDir(resType.toLatin1(), *sIt, priority);
                }
            }
        }
        if (profiles.isEmpty())
            break;
        profile = profiles.back();
        group = QString::fromLatin1("Directories-%1").arg(profile);
        profiles.pop_back();
        priority = true;
    }

    // Process KIOSK restrictions.
    if (!kde_kiosk_admin || qgetenv("KDE_KIOSK_NO_RESTRICTIONS").isEmpty())
    {
        KConfigGroup cg(config, "KDE Resource Restrictions");
        const QMap<QString, QString> entries = cg.entryMap();
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
             it2 != entries.end(); ++it2)
        {
            const QString key = it2.key();
            if (!cg.readEntry(key, true))
            {
                d->m_restrictionsActive = true;
                const QByteArray cKey = key.toLatin1();
                d->m_restrictions.insert(cKey, true);
                d->m_dircache.remove(cKey);
                d->m_savelocations.remove(cKey);
            }
        }
    }

    // check if the number of config dirs changed
    const bool configDirsChanged = (resourceDirs("config").count() != configdirs);
    // If the config dirs changed, we check kiosk restrictions again.
    d->m_checkRestrictions = configDirsChanged;
    // return true if the number of config dirs changed: reparse config file
    return configDirsChanged;
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
QString KStandardDirs::locate( const char *type,
                               const QString& filename, const KComponentData &cData)
{
    return cData.dirs()->findResource(type, filename);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, const KComponentData &cData)
{
    return locateLocal(type, filename, true, cData);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, bool createDir,
                                    const KComponentData &cData)
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.lastIndexOf(QLatin1Char('/')) + 1;
    if (!slash) { // only one filename
        return cData.dirs()->saveLocation(type, QString(), createDir) + filename;
    }

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return cData.dirs()->saveLocation(type, dir, createDir) + file;
}

bool KStandardDirs::checkAccess(const QString& pathname, int mode)
{
    int accessOK = KDE::access( pathname, mode );
    if ( accessOK == 0 )
        return true;  // OK, I can really access the file

    // else
    // if we want to write the file would be created. Check, if the
    // user may write to the directory to create the file.
    if ( (mode & W_OK) == 0 )
        return false;   // Check for write access is not part of mode => bail out


    if (!KDE::access( pathname, F_OK)) // if it already exists
        return false;

    //strip the filename (everything until '/' from the end
    QString dirName(pathname);
    int pos = dirName.lastIndexOf(QLatin1Char('/'));
    if ( pos == -1 )
        return false;   // No path in argument. This is evil, we won't allow this
    else if ( pos == 0 ) // don't turn e.g. /root into an empty string
        pos = 1;

    dirName.truncate(pos); // strip everything starting from the last '/'

    accessOK = KDE::access( dirName, W_OK );
    // -?- Can I write to the accessed diretory
    if ( accessOK == 0 )
        return true;  // Yes
    else
        return false; // No
}

