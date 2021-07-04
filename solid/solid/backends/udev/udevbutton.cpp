/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Library General Public                      *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2 of the License, or (at your option) any later version.                 *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Library General Public License for more details.                                 *
 *                                                                                   *
 *  You should have received a copy of the GNU Library General Public License        *
 *  along with this library; see the file COPYING.LIB.  If not, write to             *
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,             *
 *  Boston, MA 02110-1301, USA.                                                      *
 *************************************************************************************/

#include "udevbutton.h"

using namespace Solid::Backends::UDev;

Button::Button(UDevDevice* device)
    : DeviceInterface(device),
    m_type(Solid::Button::UnknownButtonType)
{
    if (m_device->devicePropertyExists("KEY")) {
        m_type = Solid::Button::PowerButton;
    } else if (m_device->devicePropertyExists("SW")) {
        m_type = Solid::Button::LidButton;
    }
    // TODO: implement pressed() signal
}

Button::~Button()
{

}

bool Button::hasState() const
{
    return true;
}

bool Button::stateValue() const
{
    return true;
}

Solid::Button::ButtonType Button::type() const
{
    return m_type;
}
