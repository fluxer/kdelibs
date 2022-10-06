/*  This file is part of the KDE libraries
    Copyright (C) 2018 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "karchive.h"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QCoreApplication>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>

#if defined(HAVE_LIBARCHIVE)
#  include <archive.h>
#  include <archive_entry.h>
#endif

#include <sys/stat.h>
#if defined(HAVE_STRMODE)
#  include <string.h>
#endif

#ifndef PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
#endif

// NOTE: many KArchive users are not doing listing and extraction in a thread which means that the
// UI will be "frozen" while that happens so process events while doing so
#define KARCHIVE_TIMEOUT 250
#define KARCHIVE_BUFFSIZE 10240

#if defined(HAVE_LIBARCHIVE)
static const QStringList s_writemimetypes = QStringList()
#if ARCHIVE_VERSION_NUMBER > 3000004
    << QString::fromLatin1("application/x-lzop")
#endif
#if ARCHIVE_VERSION_NUMBER > 3001002
    << QString::fromLatin1("application/x-lz4")
#endif
#if ARCHIVE_VERSION_NUMBER > 3003002
    << QString::fromLatin1("application/zstd")
    << QString::fromLatin1("application/x-zstd-compressed-tar")
#endif
    << QString::fromLatin1("application/x-tar")
    << QString::fromLatin1("application/x-compressed-tar")
    << QString::fromLatin1("application/x-bzip")
    << QString::fromLatin1("application/x-gzip")
    << QString::fromLatin1("application/x-bzip-compressed-tar")
    << QString::fromLatin1("application/x-gzip-compressed-tar")
    << QString::fromLatin1("application/x-tarz")
    << QString::fromLatin1("application/x-xz")
    << QString::fromLatin1("application/x-xz-compressed-tar")
    << QString::fromLatin1("application/x-lzma-compressed-tar")
    << QString::fromLatin1("application/x-java-archive")
    << QString::fromLatin1("application/zip")
    << QString::fromLatin1("application/x-7z-compressed")
    << QString::fromLatin1("application/x-iso9660-image")
    << QString::fromLatin1("application/x-apple-diskimage")
    << QString::fromLatin1("application/x-cd-image")
    << QString::fromLatin1("application/x-raw-disk-image");

static const QStringList s_readmimetypes = QStringList()
#if ARCHIVE_VERSION_NUMBER > 3000004
    << QString::fromLatin1("application/x-lzop")
    << QString::fromLatin1("application/x-lrzip")
#endif
#if ARCHIVE_VERSION_NUMBER > 3001002
    << QString::fromLatin1("application/x-lz4")
    << QString::fromLatin1("application/x-lz4-compressed-tar")
#endif
#if ARCHIVE_VERSION_NUMBER > 3003002
    << QString::fromLatin1("application/zstd")
    << QString::fromLatin1("application/x-zstd")
    << QString::fromLatin1("application/x-zstd-compressed-tar")
#endif
#if ARCHIVE_VERSION_NUMBER > 3003004
    << QString::fromLatin1("application/vnd.rar")
#endif
    << QString::fromLatin1("application/x-archive")
    << QString::fromLatin1("application/x-xar")
    << QString::fromLatin1("application/x-deb")
    << QString::fromLatin1("application/x-cd-image")
    << QString::fromLatin1("application/x-bcpio")
    << QString::fromLatin1("application/x-cpio")
    << QString::fromLatin1("application/x-cpio-compressed")
    << QString::fromLatin1("application/x-sv4cpio")
    << QString::fromLatin1("application/x-sv4crc")
    << QString::fromLatin1("application/x-rpm")
    << QString::fromLatin1("application/x-source-rpm")
    << QString::fromLatin1("application/vnd.ms-cab-compressed")
    << QString::fromLatin1("application/x-servicepack")
    << QString::fromLatin1("application/x-compress")
    << QString::fromLatin1("application/x-lzip");
#else
static const QStringList s_writemimetypes;
static const QStringList s_readmimetypes;
#endif // HAVE_LIBARCHIVE

KArchiveEntry::KArchiveEntry()
    : encrypted(false),
    size(0),
    gid(-1),
    uid(-1),
    mode(0),
    atime(0),
    ctime(0),
    mtime(0)
{
}

QString KArchiveEntry::fancyEncrypted() const
{
    if (encrypted) {
        return QString::fromLatin1("yes");
    }
    return QString::fromLatin1("No");
}

QString KArchiveEntry::fancySize() const
{
    return KGlobal::locale()->formatByteSize(size, 1);
}

QString KArchiveEntry::fancyMode() const
{
#if defined(HAVE_STRMODE)
    if (mode == 0) {
        return QString::fromLatin1("---------");
    }

    char strmodebuffer[20];
    ::memset(strmodebuffer, '\0', sizeof(strmodebuffer) * sizeof(char));
    ::strmode(mode, strmodebuffer);

    return QString::fromLatin1(strmodebuffer);
#elif defined(HAVE_LIBARCHIVE)
    struct archive_entry* archiveentry = archive_entry_new();
    archive_entry_set_mode(archiveentry, mode);
    const QString result = QString::fromLatin1(archive_entry_strmode(archiveentry));
    archive_entry_free(archiveentry);
    return result;
#else
    return QString::number(mode);
#endif // HAVE_STRMODE
}

QString KArchiveEntry::fancyATime() const
{
    return KGlobal::locale()->formatDateTime(QDateTime::fromTime_t(atime));
}

QString KArchiveEntry::fancyCTime() const
{
    return KGlobal::locale()->formatDateTime(QDateTime::fromTime_t(ctime));
}

QString KArchiveEntry::fancyMTime() const
{
    return KGlobal::locale()->formatDateTime(QDateTime::fromTime_t(mtime));
}

QString KArchiveEntry::fancyType() const
{
    if (S_ISREG(mode)) {
        return QString::fromLatin1("File");
    } else if (S_ISDIR(mode)) {
        return QString::fromLatin1("Directory");
    } else if (S_ISLNK(mode)) {
        return QString::fromLatin1("Link");
    } else if (S_ISCHR(mode)) {
        return QString::fromLatin1("Character");
    } else if (S_ISBLK(mode)) {
        return QString::fromLatin1("Block");
    } else if (S_ISFIFO(mode)) {
        return QString::fromLatin1("Fifo");
    } else if (S_ISSOCK(mode)) {
        return QString::fromLatin1("Socket");
    } else {
        return QString::fromLatin1("None");
    }
}

bool KArchiveEntry::isNull() const
{
    return (mode == 0);
}

bool KArchiveEntry::operator==(const KArchiveEntry &karchiveentry) const {
    return (
        encrypted == karchiveentry.encrypted &&
        size == karchiveentry.size &&
        gid == karchiveentry.gid &&
        uid == karchiveentry.uid &&
        mode == karchiveentry.mode &&
        atime == karchiveentry.atime &&
        ctime == karchiveentry.ctime &&
        mtime == karchiveentry.mtime &&
        hardlink == karchiveentry.hardlink &&
        symlink == karchiveentry.symlink &&
        pathname == karchiveentry.pathname &&
        groupname == karchiveentry.groupname &&
        username == karchiveentry.username
    );
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const KArchiveEntry &karchiveentry)
{
    d << "KArchiveEntry( encrypted:" << karchiveentry.fancyEncrypted()
        << ", size:" << karchiveentry.fancySize()
        << ", gid:" << karchiveentry.gid
        << ", uid:" << karchiveentry.uid
        << ", mode:" << karchiveentry.fancyMode()
        << ", atime:" << karchiveentry.fancyATime()
        << ", ctime:" << karchiveentry.fancyCTime()
        << ", mtime:" << karchiveentry.fancyMTime()
        << ", hardlink:" << karchiveentry.hardlink
        << ", symlink:" << karchiveentry.symlink
        << ", pathname:" << karchiveentry.pathname
        << ", groupname:" << karchiveentry.groupname
        << ", username:" << karchiveentry.username
        << ", type:" << karchiveentry.fancyType()
        << ")";
    return d;
}
#endif


class KArchivePrivate
{
public:
    KArchivePrivate();

    QString m_path;
    bool m_writable;
    QString m_error;

#if defined(HAVE_LIBARCHIVE)
    static const char* passphraseCallback(struct archive* archiveptr, void* dataptr);

    struct archive* openRead(const QByteArray &path);
    struct archive* openWrite(const QByteArray &path);
    struct archive* openDisk(const bool preserve);
    static bool closeRead(struct archive*);
    static bool closeWrite(struct archive*);

    bool copyData(struct archive* readarchive, struct archive* writearchive);
    bool copyData(struct archive* readarchive, QByteArray *buffer);
#endif
};

KArchivePrivate::KArchivePrivate()
    : m_writable(false)
{
}

#if defined(HAVE_LIBARCHIVE)
const char* KArchivePrivate::passphraseCallback(struct archive* archiveptr, void* dataptr)
{
    // TODO: ask for password via KPasswordDialog
    kWarning() << "Password-protected archives are not supported";
    Q_UNUSED(archiveptr);
    Q_UNUSED(dataptr);
    return NULL;
}

struct archive* KArchivePrivate::openRead(const QByteArray &path)
{
    struct archive* readarchive = archive_read_new();

    if (readarchive) {
        archive_read_support_filter_all(readarchive);
        archive_read_support_format_all(readarchive);
        archive_read_set_passphrase_callback(readarchive, NULL, KArchivePrivate::passphraseCallback);

        if (archive_read_open_filename(readarchive, path, KARCHIVE_BUFFSIZE) != ARCHIVE_OK) {
            m_error = i18n("archive_read_open_filename: %1", archive_error_string(readarchive));
            kDebug() << m_error;
        }
    }

    return readarchive;
}

struct archive* KArchivePrivate::openWrite(const QByteArray &path)
{
    struct archive* writearchive = archive_write_new();

    if (writearchive) {
        if (archive_write_set_format_filter_by_ext(writearchive, path) != ARCHIVE_OK) {
            m_error = i18n("archive_write_set_format_filter_by_ext: %1", archive_error_string(writearchive));
            kDebug() << m_error;
            archive_write_add_filter_none(writearchive);
        }

        archive_write_set_format_pax_restricted(writearchive);
        archive_write_set_passphrase_callback(writearchive, NULL, KArchivePrivate::passphraseCallback);

        if (archive_write_open_filename(writearchive, path) != ARCHIVE_OK) {
            m_error = i18n("archive_write_open_filename: %1", archive_error_string(writearchive));
            kDebug() << m_error;
        }
    }

    return writearchive;
}

struct archive* KArchivePrivate::openDisk(const bool preserve)
{
    struct archive* writearchive = archive_write_disk_new();

    if (writearchive) {
        int extractFlags = ARCHIVE_EXTRACT_TIME;
        extractFlags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS | ARCHIVE_EXTRACT_SECURE_NODOTDOT;
        if (preserve) {
            extractFlags |= ARCHIVE_EXTRACT_PERM;
            extractFlags |= ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_XATTR;
            extractFlags |= ARCHIVE_EXTRACT_FFLAGS | ARCHIVE_EXTRACT_MAC_METADATA;
        }

        archive_write_disk_set_options(writearchive, extractFlags);
        archive_write_disk_set_standard_lookup(writearchive);
    }

    return writearchive;
}

bool KArchivePrivate::closeRead(struct archive* readarchive)
{
    if (readarchive) {
        if (archive_read_close(readarchive) != ARCHIVE_OK) {
            kDebug() << "archive_read_close" << archive_error_string(readarchive);
            return false;
        }

        if (archive_read_free(readarchive) != ARCHIVE_OK) {
            kDebug() << "archive_read_free" << archive_error_string(readarchive);
            return false;
        }
    }

    return true;
}

bool KArchivePrivate::closeWrite(struct archive* writearchive)
{
    if (writearchive) {
        if (archive_write_close(writearchive) != ARCHIVE_OK) {
            kDebug() << "archive_write_close" << archive_error_string(writearchive);
            return false;
        }

        if (archive_write_free(writearchive) != ARCHIVE_OK) {
            kDebug() << "archive_write_free" << archive_error_string(writearchive);
            return false;
        }
    }

    return true;
}

bool KArchivePrivate::copyData(struct archive* readarchive, struct archive* writearchive)
{
    char readbuffer[KARCHIVE_BUFFSIZE];
    ssize_t readsize = archive_read_data(readarchive, readbuffer, sizeof(readbuffer));
    while (readsize > 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        const int result = archive_errno(readarchive);
        if (result != ARCHIVE_OK) {
            m_error = i18n("archive_read_data: %1", archive_error_string(readarchive));
            kDebug() << m_error;
            return false;
        }

        if (archive_write_data(writearchive, readbuffer, readsize) != readsize) {
            m_error = i18n("archive_write_data: %1", archive_error_string(writearchive));
            kDebug() << m_error;
            return false;
        }

        readsize = archive_read_data(readarchive, readbuffer, sizeof(readbuffer));
    }

    return true;
}

bool KArchivePrivate::copyData(struct archive* readarchive, QByteArray *buffer)
{
    char readbuffer[KARCHIVE_BUFFSIZE];
    ssize_t readsize = archive_read_data(readarchive, readbuffer, sizeof(readbuffer));
    while (readsize > 0) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        const int result = archive_errno(readarchive);
        if (result != ARCHIVE_OK) {
            m_error = i18n("archive_read_data: %1", archive_error_string(readarchive));
            kDebug() << m_error;
            return false;
        }

        buffer->append(readbuffer, readsize);

        readsize = archive_read_data(readarchive, readbuffer, sizeof(readbuffer));
    }

    return true;
}
#endif // HAVE_LIBARCHIVE

KArchive::KArchive(const QString &path)
    : d(new KArchivePrivate())
{
    d->m_path = path;

#if defined(HAVE_LIBARCHIVE)
    if (path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return;
    }

    d->m_writable = QFileInfo(QFileInfo(path).path()).isWritable();
    const KMimeType::Ptr kmimetype = KMimeType::findByPath(path);
    if (kmimetype) {
        if (!s_writemimetypes.contains(kmimetype->name())) {
            d->m_writable = false;
        }
    }
    if (!d->m_writable) {
        d->m_error = i18n("Archive is not writable");
    }
#else
    d->m_error = QString::fromLatin1("Built without LibArchive");
#endif // HAVE_LIBARCHIVE
}

KArchive::~KArchive()
{
    delete d;
}

bool KArchive::isSupported()
{
#if defined(HAVE_LIBARCHIVE)
    return true;
#else
    return false;
#endif
}

bool KArchive::add(const QStringList &paths, const QByteArray &strip, const QByteArray &destination) const
{
    bool result = false;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    } else if (!d->m_writable) {
        d->m_error = i18n("Archive is not writable");
        kDebug() << d->m_error;
        return result;
    }

    QFileInfo fileinfo(d->m_path);
    KTemporaryFile tmpfile;
    tmpfile.setPrefix(QString::fromLatin1("karchive_"));
    tmpfile.setSuffix(QString::fromLatin1(".%1").arg(fileinfo.completeSuffix()));
    if (!tmpfile.open()) {
        d->m_error = i18n("Could not open temporary file: %1", tmpfile.errorString());
        kDebug() << d->m_error;
        return result;
    }

    const QByteArray tmppath = QFile::encodeName(tmpfile.fileName());
    struct archive* writearchive = d->openWrite(tmppath);
    if (!writearchive) {
        d->m_error = i18n("Could not open temporary archive: %1", tmpfile.fileName());
        kDebug() << d->m_error;
        return result;
    }

    QStringList recursivepaths;
    foreach (const QString &path, paths) {
        if (QDir(path).exists()) {
            QDirIterator iterator(path, QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (iterator.hasNext()) {
                QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

                recursivepaths << QDir::cleanPath(iterator.next());
            }
        } else {
            recursivepaths << QDir::cleanPath(path);
        }
    }
    recursivepaths.removeDuplicates();

    if (QFile::exists(d->m_path)) {
        struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
        if (!readarchive) {
            d->m_error = i18n("Could not open archive: %1", d->m_path);
            kDebug() << d->m_error;
            return result;
        }

        struct archive_entry* archiveentry = archive_entry_new();
        int ret = archive_read_next_header(readarchive, &archiveentry);
        while (ret != ARCHIVE_EOF) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

            if (ret < ARCHIVE_OK) {
                d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
                kDebug() << d->m_error;
                result = false;
                break;
            }

            const QByteArray pathname = archive_entry_pathname(archiveentry);
            if (recursivepaths.contains(strip + pathname)) {
                kDebug() << "Removing (update)" << pathname;
                archive_read_data_skip(readarchive);
                ret = archive_read_next_header(readarchive, &archiveentry);
                continue;
            }

            if (archive_write_header(writearchive, archiveentry) != ARCHIVE_OK) {
                d->m_error = i18n("archive_write_header: %1", archive_error_string(writearchive));
                kDebug() << d->m_error;
                result = false;
                break;
            }

            if (!d->copyData(readarchive, writearchive)) {
                result = false;
                break;
            }

            if (archive_write_finish_entry(writearchive) != ARCHIVE_OK) {
                d->m_error = i18n("archive_write_finish_entry: %1", archive_error_string(writearchive));
                kDebug() << d->m_error;
                result = false;
                break;
            }

            ret = archive_read_next_header(readarchive, &archiveentry);
        }

        KArchivePrivate::closeRead(readarchive);
    }

    foreach (const QString &path, recursivepaths) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);
        const QByteArray localpath = QFile::encodeName(path);

        struct stat statistic;
        if (::lstat(localpath, &statistic) != 0) {
            const int savederrno = errno;
            d->m_error = i18n("lstat: %1", qt_error_string(savederrno));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        QByteArray pathname = localpath;
        if (pathname.startsWith(strip)) {
            pathname.remove(0, strip.size());
        }
        pathname.prepend(destination);
        if (pathname.isEmpty()) {
            kWarning() << "Not adding empty pathname";
            continue;
        }
        kDebug() << "Adding" << path << "as" << pathname;

        // NOTE: archive_entry_copy_stat doesn't work
        // http://linux.die.net/man/2/stat
        struct archive_entry* newentry = archive_entry_new();
        archive_entry_set_pathname(newentry, pathname);
        archive_entry_set_size(newentry, statistic.st_size);
        archive_entry_set_gid(newentry, statistic.st_gid);
        archive_entry_set_uid(newentry, statistic.st_uid);
        archive_entry_set_atime(newentry, statistic.st_atim.tv_sec, statistic.st_atim.tv_nsec);
        archive_entry_set_ctime(newentry, statistic.st_ctim.tv_sec, statistic.st_ctim.tv_nsec);
        archive_entry_set_mtime(newentry, statistic.st_mtim.tv_sec, statistic.st_mtim.tv_nsec);

        // filetype and mode are supposedly the same, permissions are set when mode is set
        archive_entry_set_mode(newentry, statistic.st_mode);

        if (statistic.st_nlink > 1) {
            // TODO: archive_entry_set_hardlink(newentry, pathname);
        }

        if (S_ISLNK(statistic.st_mode)) {
            QByteArray linkbuffer(PATH_MAX + 1, char('\0'));
            if (::readlink(localpath, linkbuffer.data(), PATH_MAX) == -1) {
                const int savederrno = errno;
                d->m_error = i18n("readlink: %1", qt_error_string(savederrno));
                result = false;
                break;
            }

            if (linkbuffer.startsWith(strip)) {
                linkbuffer.remove(0, strip.size());
            }

            archive_entry_set_symlink(newentry, linkbuffer.constData());
        }

        if (archive_write_header(writearchive, newentry) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_header: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            archive_entry_free(newentry);
            result = false;
            break;
        }
        archive_entry_free(newentry);

        if (S_ISREG(statistic.st_mode)) {
            QFile file(path);
            if (!file.open(QFile::ReadOnly)) {
                d->m_error = i18n("Could not open source: %1", path);
                kDebug() << d->m_error;
                result = false;
                break;
            }

            const QByteArray data = file.readAll();
            if (data.isEmpty() && statistic.st_size > 0) {
                d->m_error = i18n("Could not read source: %1", path);
                kDebug() << d->m_error;
                result = false;
                break;
            }

            if (statistic.st_size > 0 && data.size() != statistic.st_size) {
                d->m_error = i18n("Read and stat size are different: %1", path);
                kDebug() << d->m_error;
                result = false;
                break;
            }

            if (archive_write_data(writearchive, data.constData(), data.size()) != statistic.st_size) {
                d->m_error = i18n("archive_write_data: %1", archive_error_string(writearchive));
                kDebug() << d->m_error;
                result = false;
                break;
            }
        }

        if (archive_write_finish_entry(writearchive) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_finish_entry: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        result = true;
    }

    KArchivePrivate::closeWrite(writearchive);

    if (result) {
        kDebug() << "Replacing" << d->m_path << "with" << tmppath;

        // NOTE: QFile::rename() can choke on cross-filesystem move
        QFile::remove(d->m_path);
        result = QFile::copy(tmpfile.fileName(), d->m_path);
        QFile::remove(tmpfile.fileName());
        if (!result) {
            d->m_error = i18n("Could not move: %1 to: %2", tmpfile.fileName(), d->m_path);
            kDebug() << d->m_error;
        }
    }
#else
    Q_UNUSED(paths);
    Q_UNUSED(strip);
    Q_UNUSED(destination);
#endif // HAVE_LIBARCHIVE

    return result;
}

bool KArchive::remove(const QStringList &paths) const
{
    bool result = false;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    } else if (!d->m_writable) {
        d->m_error = i18n("Archive is not writable");
        kDebug() << d->m_error;
        return result;
    }

    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (!readarchive) {
        d->m_error = i18n("Could not open archive: %1", d->m_path);
        kDebug() << d->m_error;
        return result;
    }

    QFileInfo fileinfo(d->m_path);
    KTemporaryFile tmpfile;
    tmpfile.setPrefix(QString::fromLatin1("karchive_"));
    tmpfile.setSuffix(QString::fromLatin1(".%1").arg(fileinfo.completeSuffix()));
    if (!tmpfile.open()) {
        d->m_error = i18n("Could not open temporary file: %1", tmpfile.errorString());
        kDebug() << d->m_error;
        return result;
    }
    tmpfile.close();

    const QByteArray tmppath = QFile::encodeName(tmpfile.fileName());
    struct archive* writearchive = d->openWrite(tmppath);
    if (!writearchive) {
        d->m_error = i18n("Could not open temporary archive: %1", tmpfile.fileName());
        kDebug() << d->m_error;
        return result;
    }

    QStringList notfound = paths;

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(readarchive, &entry);
    while (ret != ARCHIVE_EOF) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        if (ret < ARCHIVE_OK) {
            d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        const QString pathnamestring = QFile::decodeName(pathname);
        if (paths.contains(pathnamestring)) {
            kDebug() << "Removing" << pathname;
            notfound.removeAll(pathname);
            archive_read_data_skip(readarchive);
            ret = archive_read_next_header(readarchive, &entry);
            result = true;
            continue;
        }

        if (archive_write_header(writearchive, entry) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_header: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        if (!d->copyData(readarchive, writearchive)) {
            result = false;
            break;
        }

        if (archive_write_finish_entry(writearchive) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_finish_entry: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        ret = archive_read_next_header(readarchive, &entry);
    }

    KArchivePrivate::closeWrite(writearchive);
    KArchivePrivate::closeRead(readarchive);

    if (result) {
        kDebug() << "Replacing" << d->m_path << "with" << tmppath;

        QFile::remove(d->m_path);
        result = QFile::copy(tmpfile.fileName(), d->m_path);
        QFile::remove(tmpfile.fileName());
        if (!result) {
            d->m_error = i18n("Could not move: %1 to: %2", tmpfile.fileName(), d->m_path);
            kDebug() << d->m_error;
        }
    }

    if (!notfound.isEmpty()) {
        d->m_error = i18n("Entries not in archive: %1", notfound.join(QLatin1String(", ")));
        kDebug() << d->m_error;
        result = false;
    }
#else
    Q_UNUSED(paths);
#endif // HAVE_LIBARCHIVE

    return result;
}

bool KArchive::extract(const QStringList &paths, const QString &destination, bool preserve) const
{
    bool result = false;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    }

    const QString currentdir = QDir::currentPath();
    if (!QDir::setCurrent(destination)) {
        d->m_error = i18n("could not change to destination directory: %1", destination);
        kDebug() << d->m_error;
        return result;
    }

    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (!readarchive) {
        d->m_error = i18n("Could not open archive: %1", d->m_path);
        kDebug() << d->m_error;
        return result;
    }

    struct archive* writearchive = d->openDisk(preserve);
    if (!writearchive) {
        d->m_error = i18n("Could not open destination: %1", destination);
        kDebug() << d->m_error;
        return result;
    }

    QStringList notfound = paths;

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(readarchive, &entry);
    while (ret != ARCHIVE_EOF) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        if (ret < ARCHIVE_OK) {
            d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        const QString pathnamestring = QFile::decodeName(pathname);
        if (!paths.contains(pathnamestring)) {
            archive_read_data_skip(readarchive);
            ret = archive_read_next_header(readarchive, &entry);
            continue;
        }
        notfound.removeAll(pathnamestring);
        result = true;

        if (archive_write_header(writearchive, entry) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_header: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        if (archive_read_extract2(readarchive, entry, writearchive) != ARCHIVE_OK) {
            d->m_error = i18n("archive_read_extract2: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        if (archive_write_finish_entry(writearchive) != ARCHIVE_OK) {
            d->m_error = i18n("archive_write_finish_entry: %1", archive_error_string(writearchive));
            kDebug() << d->m_error;
            result = false;
            break;
        }

        ret = archive_read_next_header(readarchive, &entry);
    }

    KArchivePrivate::closeWrite(writearchive);
    KArchivePrivate::closeRead(readarchive);

    Q_ASSERT_X(currentdir == QDir::currentPath(), "KArchive::extract", "Current directory changed");
    if (!QDir::setCurrent(currentdir)) {
        kWarning() << "Could not change to orignal directory" << currentdir;
    }

    if (!notfound.isEmpty()) {
        d->m_error = i18n("Entries not in archive: %1", notfound.join(QLatin1String(", ")));
        kDebug() << d->m_error;
        result = false;
    }
#else
    Q_UNUSED(paths);
    Q_UNUSED(destination);
    Q_UNUSED(preserve);
#endif // HAVE_LIBARCHIVE

    return result;
}

QList<KArchiveEntry> KArchive::list(const QString &path) const
{
    QList<KArchiveEntry> result;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    }

    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (!readarchive) {
        d->m_error = i18n("Could not open archive: %1", d->m_path);
        kDebug() << d->m_error;
        return result;
    }

    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(readarchive, &entry);
    while (ret != ARCHIVE_EOF) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        if (ret < ARCHIVE_OK) {
            d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
            result.clear();
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        if (!path.isEmpty()) {
            const QString pathnamestring = QFile::decodeName(pathname);
            if (!pathnamestring.startsWith(path)) {
                continue;
            }
        }

        KArchiveEntry karchiveentry;
        karchiveentry.encrypted = bool(archive_entry_is_encrypted(entry));
        karchiveentry.size = archive_entry_size(entry);
        karchiveentry.gid = archive_entry_gid(entry);
        karchiveentry.uid = archive_entry_uid(entry);
        karchiveentry.mode = archive_entry_mode(entry);
        karchiveentry.atime = archive_entry_atime(entry);
        karchiveentry.ctime = archive_entry_ctime(entry);
        karchiveentry.mtime = archive_entry_mtime(entry);
        karchiveentry.hardlink =  archive_entry_hardlink(entry);
        karchiveentry.symlink = archive_entry_symlink(entry);
        karchiveentry.pathname = pathname;
        karchiveentry.groupname = archive_entry_gname(entry);
        karchiveentry.username = archive_entry_uname(entry);

        result << karchiveentry;

        ret = archive_read_next_header(readarchive, &entry);
    }

    KArchivePrivate::closeRead(readarchive);
#else
    Q_UNUSED(path);
#endif // HAVE_LIBARCHIVE

    return result;
}

KArchiveEntry KArchive::entry(const QString &path) const
{
    KArchiveEntry result;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    }

    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (!readarchive) {
        d->m_error = i18n("Could not open archive: %1", d->m_path);
        kDebug() << d->m_error;
        return result;
    }

    bool found = false;
    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(readarchive, &entry);
    while (ret != ARCHIVE_EOF) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        if (ret < ARCHIVE_OK) {
            d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
            kDebug() << d->m_error;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        const QString pathnamestring = QFile::decodeName(pathname);
        if (pathnamestring == path) {
            result.encrypted = bool(archive_entry_is_encrypted(entry));
            result.size = archive_entry_size(entry);
            result.gid = archive_entry_gid(entry);
            result.uid = archive_entry_uid(entry);
            result.mode = archive_entry_mode(entry);
            result.atime = archive_entry_atime(entry);
            result.ctime = archive_entry_ctime(entry);
            result.mtime = archive_entry_mtime(entry);
            result.hardlink =  archive_entry_hardlink(entry);
            result.symlink = archive_entry_symlink(entry);
            result.pathname = pathname;
            result.groupname = archive_entry_gname(entry);
            result.username = archive_entry_uname(entry);

            found = true;
            break;
        }

        ret = archive_read_next_header(readarchive, &entry);
    }

    KArchivePrivate::closeRead(readarchive);

    if (!found) {
        d->m_error = i18n("Entry not in archive: %1", path);
        kDebug() << d->m_error;
    }
#else
    Q_UNUSED(path);
#endif // HAVE_LIBARCHIVE

    return result;
}


QByteArray KArchive::data(const QString &path) const
{
    QByteArray result;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    if (d->m_path.isEmpty()) {
        d->m_error = i18n("Empty archive path");
        kDebug() << d->m_error;
        return result;
    }

    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (!readarchive) {
        d->m_error = i18n("Could not open archive: %1", d->m_path);
        kDebug() << d->m_error;
        return result;
    }

    bool found = false;
    struct archive_entry* entry = archive_entry_new();
    int ret = archive_read_next_header(readarchive, &entry);
    while (ret != ARCHIVE_EOF) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, KARCHIVE_TIMEOUT);

        if (ret < ARCHIVE_OK) {
            d->m_error = i18n("archive_read_next_header: %1", archive_error_string(readarchive));
            kDebug() << d->m_error;
            break;
        }

        const QByteArray pathname = archive_entry_pathname(entry);
        const QString pathnamestring = QFile::decodeName(pathname);
        if (pathnamestring == path) {
            d->copyData(readarchive, &result);

            found = true;
            break;
        }

        ret = archive_read_next_header(readarchive, &entry);
    }

    KArchivePrivate::closeRead(readarchive);

    if (!found) {
        d->m_error = i18n("Entry not in archive: %1", path);
        kDebug() << d->m_error;
        result.clear();
    }
#else
    Q_UNUSED(path);
#endif // HAVE_LIBARCHIVE

    return result;
}

bool KArchive::isReadable() const
{
    bool result = false;

#if defined(HAVE_LIBARCHIVE)
    d->m_error.clear();
    struct archive* readarchive = d->openRead(QFile::encodeName(d->m_path));
    if (readarchive) {
        result = true;
    }
    KArchivePrivate::closeRead(readarchive);
#endif

    return result;
}

bool KArchive::isWritable() const
{
    return d->m_writable;
}

QString KArchive::errorString() const
{
    return d->m_error;
}

QStringList KArchive::readableMimeTypes()
{
    return s_readmimetypes;
}

QStringList KArchive::writableMimeTypes()
{
    QStringList result = s_readmimetypes;
    result.append(s_writemimetypes);
    result.removeDuplicates();
    return result;
}
