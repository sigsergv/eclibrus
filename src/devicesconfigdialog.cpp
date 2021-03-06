/*
 * devicesconfigdialog.cpp
 *
 * Created on: Jan 7, 2012
 * Author: Sergey Stolyarov
 */

#include <QtWidgets>
#include <QtDebug>

#include "devicesconfigdialog.h"
#include "addnewdevicedialog.h"
#include "webdavdeviceconfigdialog.h"
#include "devices.h"
#include "ui_devicesconfigdialog.h"

struct DevicesConfigDialog::Private
{
    Ui::DevicesConfigDialog ui;
};

const int DeviceInfoRole = Qt::UserRole + 1;

DevicesConfigDialog::DevicesConfigDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);

    // connect signals
    connect(p->ui.findNewDeviceButton, SIGNAL(clicked()),
        this, SLOT(addNewDevice()));
    connect(p->ui.forgetDeviceButton, SIGNAL(clicked()),
        this, SLOT(deleteSelectedDevices()));
    connect(p->ui.addWebDavDeviceButton, SIGNAL(clicked()),
        this, SLOT(addWebDavDevice()));

    reloadDevicesList();
}

void DevicesConfigDialog::reloadDevicesList()
{
    QList<Eclibrus::DeviceInfo> devices = Eclibrus::registeredDevices();

    QListWidget * lw = p->ui.devicesList;

    lw->clear();
    foreach (const Eclibrus::DeviceInfo & di, devices) {
        QString dev_name = di.name;
        if (dev_name.isEmpty()) {
            dev_name = QString("UUID: %1").arg(di.uuid);
        }
        QListWidgetItem * item = new QListWidgetItem(dev_name, lw);
        QVariant v;
        v.setValue(di);
        item->setData(DeviceInfoRole, v);
    }
}


DevicesConfigDialog::~DevicesConfigDialog()
{
    delete p;
}

void DevicesConfigDialog::addNewDevice()
{
    AddNewDeviceDialog dlg;
    if (QDialog::Accepted != dlg.exec()) {
        return;
    }

    Eclibrus::DeviceInfo di = dlg.selectedDevice();
    qDebug() << "device" << di.uuid;
    Eclibrus::registerDevice(di);
    reloadDevicesList();
}

void DevicesConfigDialog::addWebDavDevice()
{
    WebDavDeviceConfigDialog dlg;
    if (QDialog::Accepted == dlg.exec()) {
        // remember this device
        Eclibrus::DeviceInfo di = dlg.device();
        Eclibrus::registerDevice(di);
        reloadDevicesList();
    }
}

void DevicesConfigDialog::deleteSelectedDevices()
{
    QListWidget * lw = p->ui.devicesList;
    QList<QListWidgetItem *> selected = lw->selectedItems();

    foreach (QListWidgetItem * item, selected) {
        Eclibrus::DeviceInfo di = item->data(DeviceInfoRole).value<Eclibrus::DeviceInfo>();   
        Eclibrus::unregisterDevice(di);
        lw->takeItem(lw->row(item));
    }
}
