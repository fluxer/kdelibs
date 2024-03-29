/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BATTERY_H
#define SOLID_BATTERY_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class BatteryPrivate;
    class Device;

    /**
     * This device interface is available on batteries.
     */
    class SOLID_EXPORT Battery : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(BatteryType ChargeState)
        Q_PROPERTY(bool plugged READ isPlugged)
        Q_PROPERTY(bool powerSupply READ isPowerSupply)
        Q_PROPERTY(BatteryType type READ type)
        Q_PROPERTY(int chargePercent READ chargePercent)
        Q_PROPERTY(int capacity READ capacity)
        Q_PROPERTY(bool rechargeable READ isRechargeable)
        Q_PROPERTY(ChargeState chargeState READ chargeState)
        Q_DECLARE_PRIVATE(Battery)
        friend class Device;

    public:
        /**
         * This enum type defines the type of the device holding the battery
         *
         * - UnknownBattery : A battery in an unknown device
         * - PrimaryBattery : A primary battery for the system (for example laptop battery)
         * - UpsBattery : A battery in an Uninterruptible Power Supply
         * - UsbBattery : A battery in a is of USB device (for example mouse battery)
         */
        enum BatteryType { UnknownBattery, PrimaryBattery, UpsBattery,
                           UsbBattery };

        /**
         * This enum type defines charge state of a battery
         *
         * - UnknownCharge : Battery charge state is unknown
         * - Charging : Battery is charging
         * - Discharging : Battery is discharging
         * - FullyCharged : Battery is fully charged
         */
        enum ChargeState { UnknownCharge, Charging, Discharging, FullyCharged };


    private:
        /**
         * Creates a new Battery object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Battery(QObject *backendObject);

    public:
        /**
         * Destroys a Battery object.
         */
        virtual ~Battery();


        /**
         * Get the Solid::DeviceInterface::Type of the Battery device interface.
         *
         * @return the Battery device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Battery; }

        /**
         * Indicates if this battery is plugged. If true is returned from this
         * method the device status will be shown by applets for example.
         *
         * @return true if the battery is plugged, false otherwise
         */
        bool isPlugged() const;

        /**
         * Indicates if this battery is powering the machine or from an attached deviced.
         *
         * @since 4.11
         * @return true the battery is a powersupply, false otherwise
         */
        bool isPowerSupply() const;

        /**
         * Retrieves the type of device holding this battery.
         *
         * @return the type of device holding this battery
         * @see Solid::Battery::BatteryType
         */
        BatteryType type() const;

        /**
         * Retrieves the current charge level of the battery normalised
         * to percent.
         *
         * @return the current charge level normalised to percent
         */
        int chargePercent() const;

        /**
         * Retrieves the battery capacity normalised to percent,
         * meaning how much energy can it hold compared to what it is designed to.
         * The capacity of the battery will reduce with age.
         * A capacity value less than 75% is usually a sign that you should renew your battery.
         *
         * @since 4.11
         * @return the battery capacity normalised to percent
         */
        int capacity() const;


        /**
         * Indicates if the battery is rechargeable.
         *
         * @return true if the battery is rechargeable, false otherwise (one time usage)
         */
        bool isRechargeable() const;

        /**
         * Retrieves the current charge state of the battery. It can be in a stable
         * state (no charge), charging or discharging.
         *
         * @return the current battery charge state
         * @see Solid::Battery::ChargeState
         */
        ChargeState chargeState() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         * @param udi the UDI of the battery with the new charge percent
         */
        void chargePercentChanged(int value, const QString &udi);

        /**
         * This signal is emitted when the capacity of this battery has changed.
         *
         * @param value the new capacity of the battery
         * @param udi the UDI of the battery with the new capacity
         * @since 4.11
         */
        void capacityChanged(int value, const QString &udi);

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Battery::ChargeState
         * @see Solid::Battery::ChargeState
         * @param udi the UDI of the battery with the new charge state
         */
        void chargeStateChanged(int newState, const QString &udi);

        /**
         * This signal is emitted if the battery get plugged in/out of the
         * battery bay.
         *
         * @param newState the new plugging state of the battery, type is boolean
         * @param udi the UDI of the battery with the new plugging state
         */
        void plugStateChanged(bool newState, const QString &udi);

        /**
         * This signal is emitted when the power supply state of the battery
         * changes.
         *
         * @param newState the new power supply state, type is boolean
         * @param udi the UDI of the battery with the new power supply state
         * @since 4.11
         */
        void powerSupplyStateChanged(bool newState, const QString &udi);
    };
}

#endif
