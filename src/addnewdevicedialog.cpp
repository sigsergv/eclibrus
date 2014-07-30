/*
 * addnewdevicedialog.cpp
 *
 * Created on: Jan 08, 2012
 * Author: Sergei Stolyarov
 */

#include <QtWidgets>
#include <QtDebug>

#include "addnewdevicedialog.h"
#include "ui_addnewdevicedialog.h"
#include "devices.h"

struct AddNewDeviceDialog::Private
{
    Ui::AddNewDeviceDialog ui;
    QMap<int, Eclibrus::DeviceInfo> devicesMap;
    Eclibrus::DeviceInfo selectedDevice;
};

const int ItemIndexRole = Qt::UserRole + 1;

AddNewDeviceDialog::AddNewDeviceDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);

    QListWidget * lw = p->ui.connectedDevicesList;

    QList<Eclibrus::DeviceInfo> connectedDevices = Eclibrus::connectedRemovableDevices();
    QList<Eclibrus::DeviceInfo> registeredDevices = Eclibrus::registeredDevices();

    QStringList skipUuids;
    foreach (const Eclibrus::DeviceInfo & di, registeredDevices) {
        skipUuids << di.uuid;
    }

    int index = 1;
    foreach (const Eclibrus::DeviceInfo & di, connectedDevices) {
        if (skipUuids.contains(di.uuid)) {
            continue;
        }
        QString item_desc;
        item_desc = di.mountPoint;
        QListWidgetItem * item = new QListWidgetItem(item_desc, lw);
        item->setData(ItemIndexRole, index);
        p->devicesMap[index] = di;

        ++index;
    }
}

AddNewDeviceDialog::~AddNewDeviceDialog()
{
    delete p;
}

Eclibrus::DeviceInfo AddNewDeviceDialog::selectedDevice()
{
    return p->selectedDevice;
}

void AddNewDeviceDialog::accept()
{
    QListWidget * lw = p->ui.connectedDevicesList;
    QList<QListWidgetItem *> items = lw->selectedItems();

    if (items.size() == 0) {
        qDebug() << "no selected items";
        return;
    }
    QListWidgetItem * item = items.first();
    int index = item->data(ItemIndexRole).toInt();

    if (!p->devicesMap.contains(index)) {
        return;
    }
    Eclibrus::DeviceInfo dev = p->devicesMap[index];

    QString name = p->ui.deviceNameLineEdit->text();

    p->selectedDevice.uuid = dev.uuid;
    p->selectedDevice.mountPoint = dev.mountPoint;
    p->selectedDevice.name = name;

    QDialog::accept();
}
