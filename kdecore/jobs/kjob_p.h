/*  This file is part of the KDE project
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef KJOB_P_H
#define KJOB_P_H

#include "kjob.h"

#include <QTimer>
#include <QEventLoop>

#include <array>

class KJobUiDelegate;

// This is a private class, but it's exported for
// KIO::Job's usage. Other Job classes in kdelibs may
// use it too.
class KDECORE_EXPORT KJobPrivate
{
public:
    KJobPrivate();
    virtual ~KJobPrivate();

    KJob *q_ptr;

    KJobUiDelegate *uiDelegate;
    int error;
    QString errorText;
    KJob::Unit progressUnit;
    std::array<qulonglong, 3> processedAmount;
    std::array<qulonglong, 3> totalAmount;
    unsigned long percentage;
    bool suspended;
    KJob::Capabilities capabilities;
    QTimer *speedTimer;
    bool isAutoDelete;
    QEventLoop *eventLoop;

    void _k_speedTimeout();

    bool isFinished;

    Q_DECLARE_PUBLIC(KJob)
};

#endif
