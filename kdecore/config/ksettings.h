/*
   This file is part of the KDE libraries
   Copyright (C) 2019 Ivailo Monev <xakepa10@gmail.com>

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

#ifndef KSETTINGS_H
#define KSETTINGS_H

#include "kdecore_export.h"

#include <QSettings>

class KSettingsPrivate;

/**
 * \class KSettings ksettings.h <KSettings>
 *
 * \brief The preferred class of the KDE configuration data system.
 */
class KDECORE_EXPORT KSettings : public QSettings
{
    Q_OBJECT
public:
    /**
     * Determines how the system-wide and user's global settings will affect
     * the reading of the configuration.
     *
     * IncludeGlobals if specified, global settings will be merged.
     *
     * Note that the main configuration source overrides all other sources
     */
    enum OpenFlag {
        SimpleConfig    = 0x00, ///< Just a single config file.
        IncludeGlobals  = 0x01, ///< Blend kdeglobals into the config object.
        FullConfig      = SimpleConfig|IncludeGlobals ///< Fully-fledged config, including globals
    };
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    /**
     * Creates a KSettings object to manipulate a configuration file
     *
     * @param file         If an absolute path is specified for @p file, that file will be used
     *                     as the store for the configuration settings.  If a non-absolute path
     *                     is provided, the file will be looked for in the standard config
     *                     directory.
     *
     * @param mode         determines whether the user, global or both settings will be allowed
     *                     to influence the values returned by this object.  See OpenFlags for
     *                     more details.
     */
    KSettings(const QString &file, const OpenFlags mode);

    ~KSettings();

    /**
     * Adds configuration source to the merge stack.
     *
     * Currently only files are accepted as configuration sources.
     *
     * Source will be merged into the current stack not overriding the
     * current settings stack thus order is important.
     *
     * @param source Extra config source.
     */
    void addSource(const QString &source);

private:
    KSettingsPrivate *d_ptr;

    Q_DISABLE_COPY(KSettings)
    Q_DECLARE_PRIVATE(KSettings)
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KSettings::OpenFlags)

#endif // KSETTINGS_H
