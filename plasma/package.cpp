/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>                   *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "package.h"
#include "config-plasma.h"

#include <QDir>
#include <QFile>
#include <QRegExp>

#include <kcomponentdata.h>
#include <kdesktopfile.h>
#include <kmimetype.h>
#include <kplugininfo.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <kdebug.h>

#include "packagemetadata.h"
#include "private/package_p.h"
#include "private/plasmoidservice_p.h"
#include "private/service_p.h"

namespace Plasma
{

bool copyFolder(QString sourcePath, QString targetPath)
{
    QDir source(sourcePath);
    if(!source.exists())
        return false;

    QDir target(targetPath);
    if(!target.exists()) {
        QString targetName = target.dirName();
        target.cdUp();
        target.mkdir(targetName);
        target = QDir(targetPath);
    }

    foreach (const QString &fileName, source.entryList(QDir::Files)) {
        QString sourceFilePath = sourcePath + QDir::separator() + fileName;
        QString targetFilePath = targetPath + QDir::separator() + fileName;

        if (!QFile::copy(sourceFilePath, targetFilePath)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, source.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        QString sourceSubFolderPath = sourcePath + QDir::separator() + subFolderName;
        QString targetSubFolderPath = targetPath + QDir::separator() + subFolderName;

        if (!copyFolder(sourceSubFolderPath, targetSubFolderPath)) {
            return false;
        }
    }

    return true;
}

bool removeFolder(QString folderPath)
{
    QDir folder(folderPath);
    if(!folder.exists())
        return false;

    foreach (const QString &fileName, folder.entryList(QDir::Files)) {
        if (!QFile::remove(folderPath + QDir::separator() + fileName)) {
            return false;
        }
    }

    foreach (const QString &subFolderName, folder.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)) {
        if (!removeFolder(folderPath + QDir::separator() + subFolderName)) {
            return false;
        }
    }

    QString folderName = folder.dirName();
    folder.cdUp();
    return folder.rmdir(folderName);
}

Package::Package()
    : d(new PackagePrivate(PackageStructure::Ptr(0), QString()))
{
}

Package::Package(const QString &packageRoot, const QString &package,
                 PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packageRoot, package))
{
}

Package::Package(const QString &packagePath, PackageStructure::Ptr structure)
    : d(new PackagePrivate(structure, packagePath))
{
}

Package::Package(const Package &other)
    : d(new PackagePrivate(*other.d))
{
}

Package::~Package()
{
    delete d;
}

Package &Package::operator=(const Package &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }

    return *this;
}

bool Package::isValid() const
{
    return d->isValid();
}

bool PackagePrivate::isValid()
{
    if (!valid || !structure) {
        return false;
    }

    //search for the file in all prefixes and in all possible paths for each prefix
    //even if it's a big nested loop, usually there is one prefix and one location
    //so shouldn't cause too much disk access
    QStringList prefixes = structure->contentsPrefixPaths();
    if (prefixes.isEmpty()) {
        prefixes << QString();
    }

    foreach (const char *dir, structure->requiredDirectories()) {
        bool failed = true;
        foreach (const QString &path, structure->searchPath(dir)) {
            foreach (const QString &prefix, prefixes) {
                QDir prefixdir(structure->path() + prefix + path);
                if (prefixdir.exists()) {
                    failed = false;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }

        if (failed) {
            kWarning() << "Could not find required directory" << dir;
            valid = false;
            return false;
        }
    }

    foreach (const char *file, structure->requiredFiles()) {
        bool failed = true;
        foreach (const QString &path, structure->searchPath(file)) {
            foreach (const QString &prefix, prefixes) {
                if (QFile::exists(structure->path() + prefix + path)) {
                    failed = false;
                    break;
                }
            }
            if (!failed) {
                break;
            }
        }

        if (failed) {
            kWarning() << "Could not find required file" << file;
            valid = false;
            return false;
        }
    }

    valid = true;
    return true;
}

QString Package::filePath(const char *fileType, const QString &filename) const
{
    if (!d->valid) {
        //kDebug() << "package is not valid";
        return QString();
    }

    QStringList paths;

    if (qstrlen(fileType) != 0) {
        paths = d->structure->searchPath(fileType);

        if (paths.isEmpty()) {
            //kDebug() << "no matching path came of it, while looking for" << fileType << filename;
            return QString();
        }
    } else {
        //when filetype is empty paths is always empty, so try with an empty string
        paths << QString();
    }

    //Nested loop, but in the medium case resolves to just one iteration
    QStringList prefixes = d->structure->contentsPrefixPaths();
    if (prefixes.isEmpty()) {
        prefixes << QString();
    }

    //kDebug() << "prefixes:" << prefixes.count() << prefixes;
    foreach (const QString &contentsPrefix, prefixes) {
        const QString prefix(d->structure->path() + contentsPrefix);

        foreach (const QString &path, paths) {
            QString file = prefix + path;

            if (!filename.isEmpty()) {
                file.append("/").append(filename);
            }

            //kDebug() << "testing" << file << QFile::exists("/bin/ls") << QFile::exists(file);
            if (QFile::exists(file)) {
                if (d->structure->allowExternalPaths()) {
                    //kDebug() << "found" << file;
                    return file;
                }

                // ensure that we don't return files outside of our base path
                // due to symlink or ../ games
                QDir dir(file);
                QString canonicalized = dir.canonicalPath() + QDir::separator();

                //kDebug() << "testing that" << canonicalized << "is in" << d->structure->path();
                if (canonicalized.startsWith(d->structure->path())) {
                    //kDebug() << "found" << file;
                    return file;
                }
            }
        }
    }

    //kDebug() << fileType << filename << "does not exist in" << prefixes << "at root" << d->structure->path();
    return QString();
}

QStringList Package::entryList(const char *fileType) const
{
    if (!d->valid) {
        return QStringList();
    }

    return d->structure->entryList(fileType);
}

PackageMetadata Package::metadata() const
{
    if (d->structure) {
        return d->structure->metadata();
    }

    return PackageMetadata();
}

void Package::setPath(const QString &path)
{
    d->setPathFromStructure(path);
}

const QString Package::path() const
{
    return d->structure ? d->structure->path() : QString();
}

const PackageStructure::Ptr Package::structure() const
{
    return d->structure;
}


//TODO: provide a version of this that allows one to ask for certain types of packages, etc?
//      should we be using KService here instead/as well?
QStringList Package::listInstalled(const QString &packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString &sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + sdir + "/metadata.desktop";
        if (QFile::exists(metadata)) {
            PackageMetadata m(metadata);
            packages << m.pluginName();
        }
    }

    return packages;
}

QStringList Package::listInstalledPaths(const QString &packageRoot) // static
{
    QDir dir(packageRoot);

    if (!dir.exists()) {
        return QStringList();
    }

    QStringList packages;

    foreach (const QString &sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
        QString metadata = packageRoot + '/' + sdir + "/metadata.desktop";
        if (QFile::exists(metadata)) {
            packages << sdir;
        }
    }

    return packages;
}

PackagePrivate::PackagePrivate(const PackageStructure::Ptr st, const QString &p)
        : structure(st),
          service(0)
{
    setPathFromStructure(p);
}

PackagePrivate::PackagePrivate(const PackageStructure::Ptr st, const QString &packageRoot, const QString &path)
        : structure(st),
          service(0)
{
    setPathFromStructure(packageRoot.isEmpty() ? path : packageRoot + "/" + path);
}

PackagePrivate::PackagePrivate(const PackagePrivate &other)
        : structure(other.structure),
          service(other.service),
          valid(other.valid)
{
}

PackagePrivate::~PackagePrivate()
{
}

PackagePrivate &PackagePrivate::operator=(const PackagePrivate &rhs)
{
    structure = rhs.structure;
    service = rhs.service;
    valid = rhs.valid;
    return *this;
}

void PackagePrivate::setPathFromStructure(const QString &path)
{
    if (!structure) {
        valid = false;
        return;
    }

    QStringList paths;
    if (path.isEmpty()) {
        paths << structure->defaultPackageRoot();
    } else if (QDir::isRelativePath(path)) {
        QString p = structure->defaultPackageRoot() + "/" + path + "/";

        if (QDir::isRelativePath(p)) {
            paths = KGlobal::dirs()->findDirs("data", p);
        } else {
            paths << p;
        }
    } else {
        paths << path;
    }

    foreach (const QString &p, paths) {
        structure->setPath(p);
        // reset valid, otherwise isValid() short-circuits
        valid = true;
        if (isValid()) {
            return;
        }
    }

    valid = false;
    structure->setPath(QString());
}

} // Namespace
