/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "kde_file.h"
#include "kdebug.h"

#include <errno.h>
#include <unistd.h>

// for reference:
// https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html

namespace KDE {

// NOTE: rename() does not support cross-filesystem move so that is emulated
int rename(const QString &in, const QString &out)
{
    const QByteArray encodedin(QFile::encodeName(in));
    const QByteArray encodedout(QFile::encodeName(out));
#ifdef EXDEV
    const int result = ::rename(encodedin.constData(), encodedout.constData());
    if (result == -1 && errno == EXDEV) {
        if (::unlink(encodedout.constData()) == -1) {
            errno = EXDEV;
            return -1;
        }
        if (QFile::copy(in, out) == false) {
            errno = EXDEV;
            return -1;
        }
        if (::unlink(encodedin.constData()) == -1) {
            errno = EXDEV;
            return -1;
        }

        errno = 0;
        return 0;
    }
    return result;
#else
    return ::rename(encodedin.constData(), encodedout.constData());
#endif // EXDEV
}

} // namespace KDE
