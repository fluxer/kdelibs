/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KPARTS_COMPONENTFACTORY_H
#define KPARTS_COMPONENTFACTORY_H

#include <kparts/factory.h>
#include <kparts/part.h>
#include <kservicetypetrader.h>
#include <kmimetypetrader.h>

#include <QtCore/QFile>

namespace KParts
{
    namespace ComponentFactory
    {
        /**
         * This method creates and returns a KParts part from a serviceType (e.g. a mimetype).
         *
         * You can use this method to create a generic viewer - that can display any
         * kind of file, provided that there is a ReadOnlyPart installed for it - in 5 lines:
         * \code
         * // Given the following: KUrl url, QWidget* parentWidget and QObject* parentObject.
         * QString mimetype = KMimeType::findByURL( url )->name();
         * KParts::ReadOnlyPart* part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, QString(), parentWidget, parentObject );
         * if ( part ) {
         *     part->openUrl( url );
         *     part->widget()->show();  // also insert the widget into a layout, or simply use a KVBox as parentWidget
         * }
         * \endcode
         *
         * @deprecated use KMimeTypeTrader::createPartInstanceFromQuery instead
         *
         * @param mimeType the mimetype which this part is associated with
         * @param constraint an optional constraint to pass to the trader (see KTrader)
         * @param parentWidget the parent widget, will be set as the parent of the part's widget
         * @param parent the parent object for the part itself
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @param error The int passed here will receive an error code in case of errors.
         *              (See enum KLibLoader::ComponentLoadingError)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
    }
}

/*
 * vim: et sw=4
 */

#endif
