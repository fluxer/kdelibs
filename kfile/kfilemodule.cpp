/* This file is part of the KDE libraries
    Copyright 2007 David Faure <faure@kde.org>

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

#include "kfilemodule.h"
#include "kfilewidget.h"
#include "kpluginfactory.h"

K_PLUGIN_FACTORY(KFileModuleFactory, registerPlugin<KFileModule>();)
K_EXPORT_PLUGIN(KFileModuleFactory("kfilemodule", "kio4"))

KFileModule::KFileModule(QObject* parent, const QVariantList&)
    : KAbstractFileModule(parent)
{
}

QWidget* KFileModule::createFileWidget(const KUrl& startDir, QWidget *parent)
{
    return new KFileWidget(startDir, parent);
}

KUrl KFileModule::getStartUrl( const KUrl& startDir, QString& recentDirClass )
{
    return KFileWidget::getStartUrl(startDir, recentDirClass);
}

void KFileModule::setStartDir( const KUrl& directory )
{
    KFileWidget::setStartDir(directory);
}

#include "moc_kfilemodule.cpp"
