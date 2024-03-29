/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_POWERMANAGEMENT_H
#define SOLID_POWERMANAGEMENT_H

#include <QtCore/QObject>
#include <QtCore/QSet>

#include <solid/solid_export.h>

namespace Solid
{
    /**
     * This namespace allows to query the underlying system to obtain information
     * about the hardware available.
     *
     * It is the single entry point for power management. Applications should use
     * it to control or query the power management features of the system.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    namespace PowerManagement
    {
        /**
         * This enum type defines the different suspend methods.
         *
         * - SuspendState: Most devices are deactivated, only RAM is powered (ACPI S3)
         * - HibernateState: State of the machine is saved to disk, and the machine is powered down (ACPI S4)
         * - HybridSleepState: The contents of RAM are first copied to non-volatile storage like for regular hibernation,
         *   but then, instead of powering down, the computer enters sleep mode
         */
        enum SleepState {
            SuspendState = 1,
            HibernateState = 2,
            /// @since 4.11
            HybridSuspendState = 4
        };

        /**
         * Retrieves a high level indication of how applications should behave according to the
         * power management subsystem. For example, when on battery power, this method will return
         * true.
         *
         * @return whether apps should conserve power
         */
        SOLID_EXPORT bool appShouldConserveResources();


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::PowerManagement::SleepState
         */
        SOLID_EXPORT QSet<SleepState> supportedSleepStates();

        /**
         * Requests that the system go to sleep
         *
         * @param state the sleep state use
         */
        SOLID_EXPORT void requestSleep(SleepState state);

        /**
         * Tell the power management subsystem to suppress automatic system sleep until further
         * notice.
         *
         * @param reason Give a reason for not allowing sleep, to be used in giving user feedback
         * about why a sleep event was prevented
         * @return a 'cookie' value representing the suppression request.  Used by the power manager to
         * track the application's outstanding suppression requests.  Returns -1 if the request was
         * denied.
         */
        SOLID_EXPORT uint beginSuppressingSleep(const QString &reason = QString());

        /**
         * Tell the power management that a particular sleep suppression is no longer needed.  When
         * no more suppressions are active, the system will be free to sleep automatically
         * @param cookie The cookie acquired when requesting sleep suppression
         * @return true if the suppression was stopped, false if an invalid cookie was given
         */
        SOLID_EXPORT bool stopSuppressingSleep(uint cookie);

        /**
         * Tell the power management subsystem to suppress automatic screen power management until
         * further notice.
         *
         * @param reason Give a reason for not allowing screen power management, to be used in giving user feedback
         * about why a screen power management event was prevented
         * @return a 'cookie' value representing the suppression request.  Used by the power manager to
         * track the application's outstanding suppression requests.  Returns -1 if the request was
         * denied.
         *
         * @since 4.6
         */
        SOLID_EXPORT uint beginSuppressingScreenPowerManagement(const QString &reason = QString());

        /**
         * Tell the power management that a particular screen power management suppression is no longer needed.  When
         * no more suppressions are active, the system will be free to handle screen power management automatically
         * @param cookie The cookie acquired when requesting screen power management suppression
         * @return true if the suppression was stopped, false if an invalid cookie was given
         *
         * @note Since 4.8, this function also inhibits screensaver
         *
         * @since 4.6
         */
        SOLID_EXPORT bool stopSuppressingScreenPowerManagement(uint cookie);

        class SOLID_EXPORT Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
             * This signal is emitted when the AC adapter is plugged or unplugged.
             *
             * @param newState whether the system runs on battery
             */
            void appShouldConserveResourcesChanged(bool newState);

            /**
             * This signal is emitted whenever the system is resuming from suspend. Applications should connect
             * to this signal to perform actions due after a wake up (such as updating clocks, etc.).
             *
             * @since 4.7
             */
            void resumingFromSuspend();

            /**
             * This signal is emitted whenever the supported sleep states change.
             *
             * @since 4.23
             */
            void supportedSleepStatesChanged();

        protected:
            Notifier();
        };

        SOLID_EXPORT Notifier *notifier();
    }
}

#endif
