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

#ifndef KSPEECH_H
#define KSPEECH_H

#include "kspeech_export.h"

#include <QObject>

class KSpeechPrivate;

/*!
    Class for Text-To-Speech.

    @since 4.22
    @warning the API is subject to change
*/
class KSPEECH_EXPORT KSpeech : public QObject
{
    Q_OBJECT
public:
    enum KSpeechState {
        JobStarted = 0,
        JobFinished = 1,
        JobCanceled = 2
    };

    KSpeech(QObject* parent = nullptr);
    ~KSpeech();

    static bool isSupported();

    void setSpeechID(const QString &id);

    int volume() const;
    bool setVolume(const int volume);

    int pitch() const;
    bool setPitch(const int pitch);

    QList<QByteArray> voices() const;
    bool setVoice(const QByteArray &voice);

public Q_SLOTS:
    int say(const QString &text);

    bool removeAllJobs();
    bool removeJob(int jobNum);

Q_SIGNALS:
    void jobStateChanged(int jobNum, int state);

private Q_SLOTS:
    void _jobStateChanged(int jobNum, int state);

private:
    Q_DISABLE_COPY(KSpeech);
    KSpeechPrivate* const d;
};

#endif // KSPEECH_H
