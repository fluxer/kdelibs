/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

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

#include "kaudioplayer.h"
#include "kpluginfactory.h"

K_PLUGIN_FACTORY(KAudioPlayerModuleFactory, registerPlugin<KAudioPlayerModule>();)
K_EXPORT_PLUGIN(KAudioPlayerModuleFactory("kaudioplayer"))

KAudioPlayerModule::KAudioPlayerModule(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
}

void KAudioPlayerModule::play(const QString &path)
{
    play(path, QString::fromLatin1("kded_kaudioplayer"));
}

void KAudioPlayerModule::play(const QString &path, const QString &playerID)
{
    KAudioPlayer* newplayer = new KAudioPlayer(this);
    newplayer->setPlayerID(playerID);
    connect(newplayer, SIGNAL(finished()), this, SLOT(_removeFinished()));
    newplayer->load(path);
}

void KAudioPlayerModule::_removeFinished()
{
    KAudioPlayer* player = qobject_cast<KAudioPlayer*>(sender());
    disconnect(player, SIGNAL(finished()), this, SLOT(_removeFinished()));
    player->deleteLater();
}

#include "moc_kaudioplayer.cpp"
