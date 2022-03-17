/*
    Theengs - Decode things and devices
    Copyright: (c) Florian ROBERT

    This file is part of Theengs.

    Theengs is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Theengs is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DEVICE_ESS_GENERIC_H
#define DEVICE_ESS_GENERIC_H
/* ************************************************************************** */

#include "device_sensor.h"

#include <QObject>
#include <QList>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

/*!
 * Generic implementation for Bluetooth Low Energy "Environmental Sensing Service".
 *
 * Protocol infos:
 * - https://www.bluetooth.com/specifications/specs/environmental-sensing-profile-1-0/
 * - https://www.bluetooth.com/specifications/specs/environmental-sensing-service-1-0/
 * - https://www.bluetooth.com/specifications/assigned-numbers/environmental-sensing-service-characteristics/
 */
class DeviceEssGeneric: public DeviceSensor
{
    Q_OBJECT

public:
    DeviceEssGeneric(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceEssGeneric(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    ~DeviceEssGeneric();

private:
    // QLowEnergyController related
    void serviceScanDone();
    void addLowEnergyService(const QBluetoothUuid &uuid);
    void serviceDetailsDiscovered_infos(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState);
    void serviceDetailsDiscovered_ess(QLowEnergyService::ServiceState newState);

    QLowEnergyService *serviceInfos = nullptr;
    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceEnvironmentalSensing = nullptr;
};

/* ************************************************************************** */
#endif // DEVICE_ESS_GENERIC_H
