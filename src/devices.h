/*
 * devices.h
 *
 * This module contains various functions to handle removable devices.
 *
 * Created on: Jan 07, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <QList>
#include <QString>
#include <QByteArray>
#include <QMetaType>

namespace Eclibrus
{
    struct DeviceInfo
    {
        enum Type { MSD /*mass storage device*/, MTP /* media transfer protocol */ };
        QString name;
        QString mountPoint;
        QString uuid;
        Type devType;

        bool isEmpty();
    };

    struct DeviceBookInfo
    {
        // directory
        QString path;
        // filename
        QString filename;
        qint64 filesize;
    };

    /*
     * Fetch info about all connected and mounted removable storage devices.
     */
    QList<DeviceInfo> connectedRemovableDevices(bool force=false);

    /*
     * Get list of all registered devices (load them from the settings)
     */
    QList<DeviceInfo> registeredDevices();

    /*
     * Get list of all connected registered devices.
     */
    QList<DeviceInfo> connectedRegisteredDevices();

    /*
     * Register new device (add to the settings)
     */
    bool registerDevice(const DeviceInfo & device);

    /*
     * "Forget" device, remove it from the settings
     */
    bool unregisterDevice(const QString & uuid);

    /*
     * Check and prepare directory in the device's library directory
     * Returns full path to the directory or empty string in case of error
     */
    QString prepareLibraryDir(const DeviceInfo & device, const QString & subdir);

    /*
     * Fetch list of all books from the library on the device
     */
    QList<DeviceBookInfo> deviceLibraryBooks(const DeviceInfo & device);

    /*
     * Delete single book from the device
     *
     * return true on success
     */
    bool deleteDeviceBook(const DeviceInfo & device, DeviceBookInfo & bi);

}
Q_DECLARE_METATYPE(Eclibrus::DeviceBookInfo);
#endif // _DEVICES_H_
