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

#include "device_hygrotemp_lywsdcgq.h"
#include "utils/utils_versionchecker.h"

#include <cstdint>
#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

/* ************************************************************************** */

DeviceHygrotempLYWSDCGQ::DeviceHygrotempLYWSDCGQ(const QString &deviceAddr, const QString &deviceName, QObject *parent):
    DeviceThermometer(deviceAddr, deviceName, parent)
{
    m_deviceType = DeviceUtils::DEVICE_THERMOMETER;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_CONNECTION;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_ADVERTISEMENT;
    m_deviceSensors += DeviceUtils::SENSOR_TEMPERATURE;
    m_deviceSensors += DeviceUtils::SENSOR_HUMIDITY;
}

DeviceHygrotempLYWSDCGQ::DeviceHygrotempLYWSDCGQ(const QBluetoothDeviceInfo &d, QObject *parent):
    DeviceThermometer(d, parent)
{
    m_deviceType = DeviceUtils::DEVICE_THERMOMETER;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_CONNECTION;
    m_deviceBluetoothMode += DeviceUtils::DEVICE_BLE_ADVERTISEMENT;
    m_deviceSensors += DeviceUtils::SENSOR_TEMPERATURE;
    m_deviceSensors += DeviceUtils::SENSOR_HUMIDITY;
}

DeviceHygrotempLYWSDCGQ::~DeviceHygrotempLYWSDCGQ()
{
    delete serviceData;
    delete serviceBattery;
    delete serviceInfos;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceHygrotempLYWSDCGQ::serviceScanDone()
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::serviceScanDone(" << m_deviceAddress << ")";

    if (serviceInfos)
    {
        if (serviceInfos->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceInfos, &QLowEnergyService::stateChanged, this, &DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_infos); // custom

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceInfos->discoverDetails(); });
        }
    }

    if (serviceBattery)
    {
        if (serviceBattery->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceBattery, &QLowEnergyService::stateChanged, this, &DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_battery);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceBattery->discoverDetails(); });
        }
    }

    if (serviceData)
    {
        if (serviceData->state() == QLowEnergyService::RemoteService)
        {
            connect(serviceData, &QLowEnergyService::stateChanged, this, &DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_data);
            connect(serviceData, &QLowEnergyService::characteristicChanged, this, &DeviceHygrotempLYWSDCGQ::bleReadNotify);

            // Windows hack, see: QTBUG-80770 and QTBUG-78488
            QTimer::singleShot(0, this, [=] () { serviceData->discoverDetails(); });
        }
    }
}

/* ************************************************************************** */

void DeviceHygrotempLYWSDCGQ::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::addLowEnergyService(" << uuid.toString() << ")";

    if (uuid.toString() == "{0000180a-0000-1000-8000-00805f9b34fb}") // Infos service
    {
        delete serviceInfos;
        serviceInfos = nullptr;

        if (m_deviceFirmware.isEmpty() || m_deviceFirmware == "UNKN")
        {
            serviceInfos = m_bleController->createServiceObject(uuid);
            if (!serviceInfos)
                qWarning() << "Cannot create service (infos) for uuid:" << uuid.toString();
        }
    }
/*
    if (uuid.toString() == "{0000180f-0000-1000-8000-00805f9b34fb}") // Battery service
    {
        delete serviceBattery;
        serviceBattery = nullptr;

        serviceBattery = m_bleController->createServiceObject(uuid);
        if (!serviceBattery)
            qWarning() << "Cannot create service (battery) for uuid:" << uuid.toString();
    }
*/
    if (uuid.toString() == "{226c0000-6476-4566-7562-66734470666d}") // (custom) data
    {
        delete serviceData;
        serviceData = nullptr;

        serviceData = m_bleController->createServiceObject(uuid);
        if (!serviceData)
            qWarning() << "Cannot create service (data) for uuid:" << uuid.toString();
    }
}

/* ************************************************************************** */

void DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_infos(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_infos(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceInfos)
        {
            // Characteristic "Firmware Revision String"
            QBluetoothUuid c(QString("00002a26-0000-1000-8000-00805f9b34fb")); // handle 0x19
            QLowEnergyCharacteristic chc = serviceInfos->characteristic(c);
            if (chc.value().size() > 0)
            {
                QString fw = chc.value();
                setFirmware(fw);
            }

            if (m_deviceFirmware.size() == 8)
            {
                if (Version(m_deviceFirmware) >= Version(LATEST_KNOWN_FIRMWARE_HYGROTEMP_LYWSDCGQ))
                {
                    m_firmware_uptodate = true;
                    Q_EMIT sensorUpdated();
                }
            }
        }
    }
}

void DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_battery(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_battery(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceBattery)
        {
            // Characteristic "Battery Level"
            QBluetoothUuid uuid_batterylevel(QString("00002a19-0000-1000-8000-00805f9b34fb"));
            QLowEnergyCharacteristic cbat = serviceBattery->characteristic(uuid_batterylevel);

            if (cbat.value().size() == 1)
            {
                int lvl = static_cast<uint8_t>(cbat.value().constData()[0]);
                setBattery(lvl);
            }
        }
    }
}

void DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_data(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        //qDebug() << "DeviceHygrotempLYWSDCGQ::serviceDetailsDiscovered_data(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceData)
        {
            // Characteristic "Temp&Humi"
            QBluetoothUuid a(QString("226caa55-6476-4566-7562-66734470666d"));
            QLowEnergyCharacteristic cha = serviceData->characteristic(a);
            m_notificationDesc = cha.clientCharacteristicConfiguration();
            serviceData->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));

            // Characteristic "Message"
            //QBluetoothUuid b(QString("226cbb55-6476-4566-7562-66734470666d"));
        }
    }
}

/* ************************************************************************** */

void DeviceHygrotempLYWSDCGQ::bleWriteDone(const QLowEnergyCharacteristic &, const QByteArray &)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::bleWriteDone(" << m_deviceAddress << ")";
    //qDebug() << "DATA: 0x" << value.toHex();
}

void DeviceHygrotempLYWSDCGQ::bleReadDone(const QLowEnergyCharacteristic &, const QByteArray &)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA: 0x" << value.toHex();
}

void DeviceHygrotempLYWSDCGQ::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    //qDebug() << "DATA: 0x" << value.toHex();

    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    if (c.uuid().toString() == "{226caa55-6476-4566-7562-66734470666d}")
    {
        // BLE temperature & humidity sensor data

        if (value.size() > 0)
        {
            // Validate data format
            if (data[1] != 0x3D && data[8] != 0x3D)
                return;

            m_temperature = value.mid(2, 4).toFloat();
            m_humidity = value.mid(9, 4).toFloat();

            m_lastUpdate = QDateTime::currentDateTime();

            if (m_dbInternal || m_dbExternal)
            {
                // SQL date format YYYY-MM-DD HH:MM:SS
                QString tsStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:00:00");
                QString tsFullStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

                QSqlQuery addData;
                addData.prepare("REPLACE INTO plantData (deviceAddr, ts, ts_full, temperature, humidity)"
                                " VALUES (:deviceAddr, :ts, :ts_full, :temp, :humi)");
                addData.bindValue(":deviceAddr", getAddress());
                addData.bindValue(":ts", tsStr);
                addData.bindValue(":ts_full", tsFullStr);
                addData.bindValue(":temp", m_temperature);
                addData.bindValue(":humi", m_humidity);

                if (addData.exec())
                {
                    m_lastUpdateDatabase = m_lastUpdate;
                }
                else
                {
                    qWarning() << "> DeviceHygrotempLYWSDCGQ addData.exec() ERROR"
                               << addData.lastError().type() << ":" << addData.lastError().text();
                }
            }

            if (m_ble_action == DeviceUtils::ACTION_UPDATE_REALTIME)
            {
                refreshRealtime();
            }
            else
            {
                refreshDataFinished(true);
                m_bleController->disconnectFromDevice();
            }
/*
            qDebug() << "* DeviceHygrotempLYWSDCGQ update:" << getAddress();
            qDebug() << "- m_firmware:" << m_deviceFirmware;
            qDebug() << "- m_battery:" << m_deviceBattery;
            qDebug() << "- m_temperature:" << m_temperature;
            qDebug() << "- m_humidity:" << m_humidity;
*/
        }
    }
}

void DeviceHygrotempLYWSDCGQ::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::confirmedDescriptorWrite!";

    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000"))
    {
        qDebug() << "confirmedDescriptorWrite() disconnect?!";

        //disabled notifications -> assume disconnect intent
        //m_control->disconnectFromDevice();
        //delete m_service;
        //m_service = nullptr;
    }
}

/* ************************************************************************** */

void DeviceHygrotempLYWSDCGQ::parseAdvertisementData(const QByteArray &value, const uint16_t identifier)
{
    //qDebug() << "DeviceHygrotempLYWSDCGQ::parseAdvertisementData(" << m_deviceAddress << ")" << value.size();
    //qDebug() << "DATA: 0x" << value.toHex();

    // MiBeacon protocol / 12-20 bytes messages
    // LYWSDCGQ uses 15 and 18 bytes messages

    if (value.size() >= 12)
    {
        const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

        QString mac;

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
        // Save mac address
        mac += value.mid(10,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(9,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(8,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(7,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(6,1).toHex().toUpper();
        mac += ':';
        mac += value.mid(5,1).toHex().toUpper();

        setSetting("mac", mac);
#else
        Q_UNUSED(mac)
#endif

        if (value.size() >= 15)
        {
            int batt = -99;
            float temp = -99.f;
            float humi = -99.f;

            // get data
            if (data[11] == 4 && value.size() >= 16)
            {
                temp = static_cast<int16_t>(data[14] + (data[15] << 8)) / 10.f;
                if (temp != m_temperature)
                {
                    if (temp > -30.f && temp < 100.f)
                    {
                        m_temperature = temp;
                        Q_EMIT dataUpdated();
                    }
                }
            }
            else if (data[11] == 6 && value.size() >= 16)
            {
                humi = static_cast<int16_t>(data[14] + (data[15] << 8)) / 10.f;
                if (humi != m_humidity)
                {
                    if (humi >= 0.f && humi <= 100.f)
                    {
                        m_humidity = humi;
                        Q_EMIT dataUpdated();
                    }
                }
            }
            else if (data[11] == 13 && value.size() >= 18)
            {
                temp = static_cast<int16_t>(data[14] + (data[15] << 8)) / 10.f;
                if (temp != m_temperature)
                {
                    m_temperature = temp;
                    Q_EMIT dataUpdated();
                }
                humi = static_cast<int16_t>(data[16] + (data[17] << 8)) / 10.f;
                if (humi != m_humidity)
                {
                    m_humidity = humi;
                    Q_EMIT dataUpdated();
                }
            }
            else if (data[11] == 10 && value.size() >= 15)
            {
                batt = static_cast<int8_t>(data[14]);
                setBattery(batt);
            }

            if (m_temperature > -99.f && m_humidity > -99)
            {
                m_lastUpdate = QDateTime::currentDateTime();

                if (needsUpdateDb())
                {
                    if (m_dbInternal || m_dbExternal)
                    {
                        // SQL date format YYYY-MM-DD HH:MM:SS
                        QString tsStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:00:00");
                        QString tsFullStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

                        QSqlQuery addData;
                        addData.prepare("REPLACE INTO plantData (deviceAddr, ts, ts_full, temperature, humidity)"
                                        " VALUES (:deviceAddr, :ts, :ts_full, :temp, :humi)");
                        addData.bindValue(":deviceAddr", getAddress());
                        addData.bindValue(":ts", tsStr);
                        addData.bindValue(":ts_full", tsFullStr);
                        addData.bindValue(":temp", m_temperature);
                        addData.bindValue(":humi", m_humidity);

                        if (addData.exec())
                        {
                            m_lastUpdateDatabase = m_lastUpdate;
                        }
                        else
                        {
                            qWarning() << "> DeviceHygrotempLYWSDCGQ addData.exec() ERROR"
                                       << addData.lastError().type() << ":" << addData.lastError().text();
                        }
                    }
                }

                refreshDataFinished(true);
            }
/*
            if (batt > -99 || temp > -99.f || humi > -99.f)
            {
                qDebug() << "* MiBeacon service data:" << getName() << getAddress() << "(" << value.size() << ") bytes";
                if (!mac.isEmpty()) qDebug() << "- MAC:" << mac;
                if (batt > -99) qDebug() << "- battery:" << batt;
                if (temp > -99) qDebug() << "- temperature:" << temp;
                if (humi > -99) qDebug() << "- humidity:" << humi;
            }
*/
        }
    }
}

/* ************************************************************************** */
