/*
 * devices.cpp
 *
 * This file is written for linux only, it won't compile for windows.
 *
 * Created on: Jan 07, 2012
 * Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#ifdef Q_OS_LINUX
#include "blkid/blkid.h"
#endif

#include "devices.h"
#include "settings.h"
#include "webdav/qwebdav_types.h"
#include "webdav/qwebdav.h"

namespace Eclibrus
{
    static QList<DeviceInfo> _connectedDevicesCache;
    static bool _connectedDevicesCacheInitialized = false;

    static QList<DeviceInfo> _connectedRegisteredDevicesCache;
    static bool _connectedRegisteredDevicesCacheInitialized = false;

    bool DeviceInfo::isEmpty()
    {
        return uuid.isEmpty();
    }

    /**
     * List of all connected removable devices (MSD only, linux only).
     */
    QList<DeviceInfo> connectedRemovableDevices(bool force)
    {
#ifdef Q_OS_LINUX
        if (force || !_connectedDevicesCacheInitialized) {
            _connectedDevicesCacheInitialized = true;
            _connectedRegisteredDevicesCacheInitialized = false;
            QList<DeviceInfo> & devices = _connectedDevicesCache;
            devices.clear();
            
            QFile mounts("/proc/mounts");
            mounts.open(QIODevice::ReadOnly);
            if (mounts.error() != QFile::NoError) {
                return QList<DeviceInfo>();
            }
            QString raw = QString::fromLatin1(mounts.readAll());
            mounts.close();

            QStringList lines = raw.split('\n', QString::SkipEmptyParts);
            QFileInfo fi;
            QHash<QString, bool> devRemovableMap;
            foreach (const QString & line, lines) {
                QStringList p = line.split(' ', QString::SkipEmptyParts);
                // p[0] filesystem
                // p[1] mount point
                // p[2] type
                // p[3] options
                // p[4] dump
                // p[5] pass
                QString fs = p[0];
                QString mp = p[1];
                if (p.size() != 6 || !fs.startsWith("/dev/")) {
                    continue;
                }
                if (mp == "/" || mp == "/home") {
                    // also ignore standard mount points
                    //continue;
                }
                // resolve "fs" to remove symlinks etc
                fi.setFile(fs);
                fs = fi.canonicalFilePath();
                
                // at this moment "fs" must looks like "/dev/XXXX"
                // find, what kind of device is p[0]
                QString sysPath = "/sys/class/block/" + fs.mid(5);

                // and again eliminate symlinks etc
                fi.setFile(sysPath);
                sysPath = fi.canonicalFilePath();

                QDir d;

                // detect is "sysPath" points to partition
                d.setPath(sysPath);
                d.cdUp();
                QString sysDevPath;

                if (d.dirName() == "block") {
                    sysDevPath = sysPath;
                } else {
                    sysDevPath = d.canonicalPath();
                }

                if (!devRemovableMap.contains(sysDevPath)) {
                    // check is device removable
                    QString removableFlagFileName = sysDevPath + QDir::separator() + "/removable";
                    fi.setFile(removableFlagFileName);
                    if (!fi.exists()) {
                        // we need removable devices only
                        continue;
                    }
                    QFile flag;
                    flag.setFileName(removableFlagFileName);
                    flag.open(QIODevice::ReadOnly);
                    char buf[5];
                    qint64 read = flag.read(buf, 4);
                    flag.close();

                    if (read == 0) {
                        continue;
                    }
                    devRemovableMap[sysDevPath] = buf[0] == '1';
                }

                if (!devRemovableMap[sysDevPath]) {
                    continue;
                }

                // fetch device id using libblkid
                blkid_probe pr;
                pr = blkid_new_probe_from_filename(p[0].toLatin1().data());
                if (!pr) {
                    // failed to open device or somethin like that
                    continue;
                }
                blkid_do_probe(pr);
                const char * buf;
                blkid_probe_lookup_value(pr, "UUID", &buf, NULL);
                QByteArray uuid(buf);
                blkid_free_probe(pr);

                DeviceInfo dev;
                dev.mountPoint = p[1];
                dev.uuid = QString::fromLatin1(uuid);
                dev.devType = DeviceInfo::MSD;
                devices << dev;
            }
        }
#endif
        return _connectedDevicesCache;
    }

    QList<DeviceInfo> registeredDevices()
    {
        QList<DeviceInfo> devices;

        QSettings * settings = Config::settings();
        QList<QVariant> items = settings->value("Devices/registered").toList();
        foreach (const QVariant & v, items) {
            if (!v.canConvert(QVariant::Map)) {
                continue;
            }
            QMap<QString, QVariant> devRecord = v.toMap();
            // fill DeviceInfo structure if possible
            // devRecord MUST contain the following QString values: 
            // "name", "uuid"
            QStringList strKeys;
            strKeys << "name" << "uuid";
            bool valid = true;

            foreach (const QString & k, strKeys) {
                if (!devRecord.contains(k) || !devRecord[k].canConvert(QVariant::String)) {
                    valid = false;
                    break;
                }
            }

            if (!valid) {
                continue;
            }
            DeviceInfo di;
            di.name = devRecord["name"].toString();
            di.uuid = devRecord["uuid"].toString();

            QString devType = devRecord["type"].toString();

            if (devType == "WEBDAV") {
                di.devType = DeviceInfo::WEBDAV;
                di.uri = devRecord["uri"].toString();
            } else {
                di.devType = DeviceInfo::MSD;
            }
            devices << di;
        }

        return devices;
    }

    QList<DeviceInfo> connectedRegisteredDevices()
    {
        if (!_connectedRegisteredDevicesCacheInitialized) {
            _connectedRegisteredDevicesCache.clear();
            // merge registered and connected devices
            QList<DeviceInfo> connected = connectedRemovableDevices();
            QList<DeviceInfo> registered = registeredDevices();
            QMap<QString, DeviceInfo> connectedMap;

            foreach (const DeviceInfo & di, connected) {
                connectedMap[di.uuid] = di;
            }
            foreach (DeviceInfo di, registered) {
                if (di.devType == DeviceInfo::MSD && connectedMap.contains(di.uuid)) {
                    QString name = di.name;
                    di = connectedMap[di.uuid];
                    di.name = name;
                    _connectedRegisteredDevicesCache << di;
                }
                if (di.devType == DeviceInfo::WEBDAV) {
                    // check 
                    qDebug() << "WebDav device check";
                    QUrl uri(di.uri);

                    QString host = uri.host();
                    int port = uri.port();
                    if (port == -1) {
                        port = 80;
                    }

                    // check connection and add to cache
                    QWebDav wd;
                    wd.connectToHost(host, port, uri.path(), uri.userName(), uri.password());
                    if (QWebDav::NoError == wd.lastError()) {
                        _connectedRegisteredDevicesCache << di;
                    }
                }
            }
        }
        return _connectedRegisteredDevicesCache;
    }

    void _updateRegistered(QList<DeviceInfo> & devices)
    {
        QSettings * settings = Config::settings();
        QList<QVariant> items;
        foreach (const DeviceInfo & di, devices) {
            QMap<QString, QVariant> rec;
            switch (di.devType) {
                case DeviceInfo::MSD:
                    // default type is MSD
                    break;

                case DeviceInfo::WEBDAV:
                    rec["type"] = "WEBDAV";
                    break;
            }
            rec["uuid"] = di.uuid;
            rec["name"] = di.name;
            if (!di.uri.isEmpty()) {
                rec["uri"] = di.uri;
            }
            items << rec;
        }
        settings->setValue("Devices/registered", items);
        
    }

    bool registerDevice(const DeviceInfo & device)
    {
        QList<DeviceInfo> registered = registeredDevices();
        bool found = false;

        foreach (const DeviceInfo & di, registered) {
            if (di.uuid == device.uuid) {
                found = true;
                break;
            }
        }

        if (found) {
            qDebug() << "device already registered";
            return false;
        }

        registered << device;
        _updateRegistered(registered);
        qDebug() << "device registered";
        return true;
    }

    bool unregisterDevice(const QString & uuid)
    {
        QList<DeviceInfo> registered = registeredDevices();
        QList<DeviceInfo> newRegistered;

        foreach (const DeviceInfo & di, registered) {
            if (di.uuid == uuid) {
                continue;
            }
            newRegistered << di;
        }
        _updateRegistered(newRegistered);
        return true;
    }

    QString prepareLibraryDir(const DeviceInfo & device, const QString & subdir)
    {
        QString path;

        switch (device.devType) {
            case DeviceInfo::MSD: {
                QDir target_dir(device.mountPoint);
                QString dev_lib_dir = Eclibrus::Config::deviceLibraryPath();
                if (!target_dir.exists(dev_lib_dir)) {
                    // try to create though
                    if (!target_dir.mkdir(dev_lib_dir)) {
                        qWarning() << "Cannot create library directory on the device.";
                        return QString();
                    }
                }
                target_dir.cd(dev_lib_dir);

                if (!target_dir.exists(subdir)) {
                    // again, try to create
                    if (!target_dir.mkdir(subdir)) {
                        qWarning() << "Cannot create directory in the device's library dir.";
                        return QString();
                    }
                }
                target_dir.cd(subdir);
                path = target_dir.canonicalPath();
                break;
            }

            case DeviceInfo::WEBDAV: {
                // try to connect device and create directory "subdir" there
                QWebDav wd;
                QUrl uri(device.uri);

                QString host = uri.host();
                int port = uri.port();
                if (port == -1) {
                    port = 80;
                }

                wd.connectToHost(host, port, uri.path(), uri.userName(), uri.password());
                if (QWebDav::NoError != wd.lastError()) {
                    qWarning() << "Failed to init WebDav link: " << device.uri;
                    return QString();
                }

                QString rootPath = uri.path();
                if (!rootPath.endsWith("/")) {
                    rootPath += "/";
                }

                wd.mkdir(rootPath + subdir);
                if (QWebDav::NoError == wd.lastError()) {
                    path = rootPath + subdir;
                }
                break;
            }
        }

        return path;
    }

    static const QStringList FB2_ZIP_NAME_FILTER("*.fb2.zip");
    static void scanLibraryDir(QDir dir, QList<DeviceBookInfo> & res);

    static void scanLibraryDir(QDir dir, QList<DeviceBookInfo> & res)
    {
        QString canonical_path = dir.canonicalPath() + QDir::separator();
        QStringList files = dir.entryList(FB2_ZIP_NAME_FILTER, QDir::Files, QDir::Name);
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        QString full_filename;
        QFileInfo fi;

        foreach (const QString & f, files) {
            DeviceBookInfo bi;
            full_filename = canonical_path + QDir::separator() + f;
            fi.setFile(full_filename);
            bi.path = canonical_path;
            bi.filename = f;
            bi.filesize = fi.size();
            res << bi;
        }

        foreach (const QString & d, dirs) {
            scanLibraryDir(QDir(canonical_path + d), res);
        }
    }

    QList<DeviceBookInfo> deviceLibraryBooks(const DeviceInfo & device)
    {
        QList<DeviceBookInfo> books;

        DeviceBookInfo bi;

        if (device.devType == DeviceInfo::MSD) {
            QString library_root_path = device.mountPoint + QDir::separator() 
                + Eclibrus::Config::deviceLibraryPath();

            // recursively find all fictionbook files in this directory
            scanLibraryDir(QDir(library_root_path), books);
        } else if (device.devType == DeviceInfo::WEBDAV) {
            QWebDav wd;
            QUrl uri(device.uri);

            QString host = uri.host();
            int port = uri.port();
            if (port == -1) {
                port = 80;
            }

            wd.connectToHost(host, port, uri.path(), uri.userName(), uri.password());
            if (QWebDav::NoError != wd.lastError()) {
                qWarning() << "Failed to init WebDav link: " << device.uri;
            }

            // fetch items
            QString path = uri.path();
            if (!path.endsWith("/")) {
                path += "/";
            }
            QList<WebDavItem> items = wd.list(path, true);
            // do not rely upon item.name, always parse item.href
            foreach (const WebDavItem & item, items) {
                if (item.type == WebDavItemDirectory) {
                    // we need files only, so ignore directories
                    continue;
                }
                QUrl url;
                url.setPath(item.href, QUrl::StrictMode);
                QString fullPath = url.toDisplayString(QUrl::RemoveScheme|\
                    QUrl::RemoveAuthority|QUrl::RemoveQuery|QUrl::RemoveFragment);
                QString fileName = fullPath.split("/").last();

                DeviceBookInfo bi;
                bi.path = url.toDisplayString(QUrl::RemoveFilename|QUrl::RemoveScheme|\
                    QUrl::RemoveAuthority|QUrl::RemoveQuery|QUrl::RemoveFragment);
                bi.filesize = 0;
                bi.filename = fileName;
                bi.metaId = item.href;
                books << bi;
            }
        }
        return books;
    }

    bool deleteDeviceBook(const DeviceInfo & device, DeviceBookInfo & bi)
    {
        QString full_book_filename = bi.path + QDir::separator() + bi.filename;
        if (device.devType == DeviceInfo::MSD) {
            bool res;
            QFileInfo fi(full_book_filename);
            res = QFile::remove(full_book_filename);

            // also try to delete the directory
            QDir d = fi.dir();
            d.rmdir(d.canonicalPath());
            return res;
        }
        return false;
    }
}

