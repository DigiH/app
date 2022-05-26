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

#include "device_hygrotemp_cgp1w.h"

#include <cstdint>
#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QSqlQuery>
#include <QSqlError>

#include <QDateTime>
#include <QDebug>

/* ************************************************************************** */

DeviceHygrotempCGP1W::DeviceHygrotempCGP1W(const QString &deviceAddr, const QString &deviceName, QObject *parent):
    DeviceThermometer(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_ENVIRONMENTAL;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_ADVERTISEMENT;
    m_deviceCapabilities = DeviceUtils::DEVICE_BATTERY;
    m_deviceSensors += DeviceUtils::SENSOR_TEMPERATURE;
    m_deviceSensors += DeviceUtils::SENSOR_HUMIDITY;
    m_deviceSensors += DeviceUtils::SENSOR_PRESSURE;
}

DeviceHygrotempCGP1W::DeviceHygrotempCGP1W(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceThermometer(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_ENVIRONMENTAL;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_ADVERTISEMENT;
    m_deviceCapabilities = DeviceUtils::DEVICE_BATTERY;
    m_deviceSensors += DeviceUtils::SENSOR_TEMPERATURE;
    m_deviceSensors += DeviceUtils::SENSOR_HUMIDITY;
    m_deviceSensors += DeviceUtils::SENSOR_PRESSURE;
}

DeviceHygrotempCGP1W::~DeviceHygrotempCGP1W()
{
    //
}

/* ************************************************************************** */

void DeviceHygrotempCGP1W::serviceScanDone()
{
    //qDebug() << "DeviceHygrotempCGP1W::serviceScanDone(" << m_deviceAddress << ")";
}

void DeviceHygrotempCGP1W::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DeviceHygrotempCGP1W::addLowEnergyService(" << uuid.toString() << ")";
    Q_UNUSED (uuid)
}

/* ************************************************************************** */

void DeviceHygrotempCGP1W::parseAdvertisementData(const QByteArray &value, const uint16_t identifier)
{
    //qDebug() << "DeviceHygrotempCGP1W::parseAdvertisementData(" << m_deviceAddress << ")" << value.size();
    //qDebug() << "DATA: 0x" << value.toHex();

    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    Q_UNUSED (data)

    if (value.size() >= 21) // Qingping data protocol // 21 bytes messages
    {
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        QString mac;
        mac += value.mid(7,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(6,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(5,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(4,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(3,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(2,1).toHex().toUpper();

        // Save mac address
        setSetting("mac", mac);
#else
        QString mac;
        Q_UNUSED(mac)
#endif
        int batt = -99;
        float temp = -99.f;
        float humi = -99.f;
        int pres = -99.f;

        // get data
        if ((data[0] == 0x88 && data[1] == 0x16) || // CGG1
            (data[0] == 0x08 && data[1] == 0x07) || // CGG1
            (data[0] == 0x88 && data[1] == 0x10) || // CGDK2
            (data[0] == 0x08 && data[1] == 0x10) || // CGDK2
            (data[0] == 0x08 && data[1] == 0x09) || // CGP1W
            (data[0] == 0x08 && data[1] == 0x0c))   // CGD1
        {
            temp = static_cast<int16_t>(data[10] + (data[11] << 8)) / 10.f;
            if (temp != m_temperature)
            {
                if (temp > -30.f && temp < 100.f)
                {
                    m_temperature = temp;
                    Q_EMIT dataUpdated();
                }
            }

            humi = static_cast<int16_t>(data[12] + (data[13] << 8)) / 10.f;
            if (humi != m_humidity)
            {
                if (humi >= 0.f && humi <= 100.f)
                {
                    m_humidity = humi;
                    Q_EMIT dataUpdated();
                }
            }

            pres = static_cast<int16_t>(data[16] + (data[17] << 8)) / 10.f;
            if (pres != m_pressure)
            {
                if (pres >= 0 && pres <= 2000)
                {
                    m_pressure = pres;
                    Q_EMIT dataUpdated();
                }
            }

            batt = static_cast<int8_t>(data[21]);
            setBattery(batt);
        }

        if (m_temperature > -99.f && m_humidity > -99.f && m_pressure > -99)
        {
            m_lastUpdate = QDateTime::currentDateTime();

            if (needsUpdateDb())
            {
                if (m_dbInternal || m_dbExternal)
                {
                    // TODO
                }
            }

            refreshDataFinished(true);
        }
/*
        if (batt > -99 || temp > -99.f || humi > -99.f || pres > -99)
        {
            qDebug() << "* CGP1W service data:" << getName() << getAddress() << "(" << value.size() << ") bytes";
            if (!mac.isEmpty()) qDebug() << "- MAC:" << mac;
            if (batt > -99) qDebug() << "- battery:" << batt;
            if (temp > -99) qDebug() << "- temperature:" << temp;
            if (humi > -99) qDebug() << "- humidity:" << humi;
            if (pres > -99) qDebug() << "- pressure:" << pres;
        }
*/
    }
}

/* ************************************************************************** */
