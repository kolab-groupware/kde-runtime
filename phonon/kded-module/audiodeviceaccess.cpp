/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "audiodeviceaccess.h"
#include <klocale.h>

namespace PS
{

AudioDeviceAccess::AudioDeviceAccess(const QStringList& deviceIds, int accessPreference,
        AudioDeviceAccess::AudioDriver driver,
        bool capture,
        bool playback):
    m_deviceIds(deviceIds),
    m_accessPreference(accessPreference),
    m_driver(driver),
    m_capture(capture),
    m_playback(playback)
{
}

bool AudioDeviceAccess::operator<(const AudioDeviceAccess& rhs) const
{
    return m_accessPreference > rhs.m_accessPreference;
}

bool AudioDeviceAccess::operator==(const AudioDeviceAccess& rhs) const
{
    return m_deviceIds == rhs.m_deviceIds && m_capture == rhs.m_capture
        && m_playback == rhs.m_playback;
}

bool AudioDeviceAccess::operator!=(const AudioDeviceAccess& rhs) const
{
     return !operator==(rhs);
}

const QString& AudioDeviceAccess::udi() const
{
    return m_udi;
}

AudioDeviceAccess::AudioDriver AudioDeviceAccess::driver() const
{
    return m_driver;
}

const QString AudioDeviceAccess::driverName() const
{
    switch (m_driver) {
    case InvalidDriver:
        return i18n("Invalid Driver");
    case AlsaDriver:
        return i18n("ALSA");
    case OssDriver:
        return i18n("OSS");
    case JackdDriver:
        return i18n("Jack");
    }
    return QString();
}

const QStringList& AudioDeviceAccess::deviceIds() const
{
    return m_deviceIds;
}

int AudioDeviceAccess::accessPreference() const
{
    return m_accessPreference;
}

bool AudioDeviceAccess::isCapture() const
{
    return m_capture;
}

bool AudioDeviceAccess::isPlayback() const
{
    return m_playback;
}

QDebug operator<<(QDebug &s, const AudioDeviceAccess &a)
{
    s.nospace() << "deviceIds: " << a.m_deviceIds
        << "accessPreference: " << a.m_accessPreference
        << "driver " << a.driverName();
    if (a.m_capture) {
        s.nospace() << " capture";
    }
    if (a.m_playback) {
        s.nospace() << " playback";
    }
    return s;
}

} // namespace PS
