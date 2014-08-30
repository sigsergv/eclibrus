/*
 * managebooksondevicedialog.cpp
 *
 * Created on: Jan 13, 2012
 * Author: Sergey Stolyarov
 */

#include "managebooksondevicedialog.h"
#include "ui_managebooksondevicedialog.h"

#include <QtWidgets>
#include <QtDebug>

#include "settings.h"
#include "eclibrenderers.h"
#include "devices.h"

struct ManageBooksOnDeviceDialog::Private
{
    Ui::ManageBooksOnDeviceDialog ui;
    QStandardItemModel * model;
    Eclibrus::DeviceInfo currentDevice;
    bool listPopulated;
};

ManageBooksOnDeviceDialog::ManageBooksOnDeviceDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);
    p->listPopulated = false;

    p->model = new QStandardItemModel();
    p->ui.booksTreeView->setModel(p->model);
    p->ui.pages->setCurrentIndex(1);

    // restore position
    QSettings * settings = Eclibrus::Config::settings();
    settings->beginGroup("ManageBooksOnDeviceDialog");
    restoreGeometry(settings->value("geometry").toByteArray());
    settings->endGroup();

    // connect signals
    connect(p->ui.devicesSelector, SIGNAL(activated(int)),
        this, SLOT(deviceChanged(int)));
    connect(p->ui.deleteSelectedBooksButton, SIGNAL(pressed()),
        this, SLOT(deleteSelectedBooks()));
}

ManageBooksOnDeviceDialog::~ManageBooksOnDeviceDialog()
{
    delete p;
}

void ManageBooksOnDeviceDialog::setVisible(bool visible)
{
    QDialog::setVisible(visible);
    if (visible) {
        p->listPopulated = true;
        // we need to populate list of devices
        foreach (const Eclibrus::DeviceInfo & di, Eclibrus::connectedRegisteredDevices()) {
            QString dev_name = di.name;
            if (dev_name.isEmpty()) {
                dev_name = di.uuid;
            }
            QVariant v;
            v.setValue(di);
            p->ui.devicesSelector->addItem(dev_name, v);
        }

        if (p->ui.devicesSelector->count() > 0) {
            deviceChanged(0);
        }
        p->ui.pages->setCurrentIndex(0);
    }

}


void ManageBooksOnDeviceDialog::showEvent(QShowEvent * event)
{
    QDialog::showEvent(event);
}

void ManageBooksOnDeviceDialog::deviceChanged(int row)
{
    // load books list from the device
    // first find the device
    Eclibrus::DeviceInfo device = p->ui.devicesSelector->itemData(row).value<Eclibrus::DeviceInfo>();

    if (device.isEmpty()) {
        return;
    }

    qDebug() << device.uri;

    p->ui.booksTreeView->setEnabled(false);
    QList<Eclibrus::DeviceBookInfo> books = Eclibrus::deviceLibraryBooks(device);
    p->model->clear();
    QStandardItem * item;
    QString prev_book_path;
    QBrush text_brush(QColor(0x80,0x80,0x80));
    QFont text_font;
    //text_font.setItalic(true);
    text_font.setBold(true);

    QMap<QString, QList< Eclibrus::DeviceBookInfo> > tree;

    foreach (const Eclibrus::DeviceBookInfo & bi, books) {
        if (!tree.contains(bi.path)) {
            tree[bi.path] = QList<Eclibrus::DeviceBookInfo>();
        }
        tree[bi.path] << bi;
    }
    
    QStringList keys(tree.keys());
    keys.sort(Qt::CaseInsensitive);
    
    foreach (const QString & key, keys) {
        item = new QStandardItem(key);
        p->model->appendRow(item);
        item->setCheckable(false);
        item->setForeground(text_brush);
        item->setFont(text_font);
        item->setFlags(Qt::NoItemFlags);

        foreach (const Eclibrus::DeviceBookInfo & bi, tree[key]) {
            item = new QStandardItem(bi.filename);
            item->setCheckable(true);
            item->setData(QVariant::fromValue<Eclibrus::DeviceBookInfo>(bi), DeviceInfoRole);
            QStringList parts;
            if (bi.filesize > 0) {
                parts << tr("File size: %1").arg(Eclibrus::Plain::fileSizeHumanReadable(bi.filesize));
            }
            parts << tr("File name: %1").arg(bi.filename);
            item->setToolTip(parts.join("\n"));
            p->model->appendRow(item);
        }
    }

    p->currentDevice = device;
    p->ui.booksTreeView->setEnabled(true);
}

void ManageBooksOnDeviceDialog::deleteSelectedBooks()
{
    QStandardItem * item;
    int count = p->model->rowCount();
    QList<QStandardItem *> delete_items;

    for (int row=0; row<count; ++row) {
        item = p->model->item(row);
        if (item->checkState() == Qt::Checked) {
            delete_items.insert(0, item);
        }
    }

    // we have to try to delete book and in case of success remove row from the list
    foreach (item, delete_items) {
        Eclibrus::DeviceBookInfo bi = item->data(DeviceInfoRole).value<Eclibrus::DeviceBookInfo>();
        if (Eclibrus::deleteDeviceBook(p->currentDevice, bi)) {
            p->model->takeRow(item->row());
            delete item;
        }
    }

}

void ManageBooksOnDeviceDialog::rememberGeometry()
{
    QSettings * settings = Eclibrus::Config::settings();
    settings->beginGroup("ManageBooksOnDeviceDialog");
    settings->setValue("geometry", saveGeometry());
    settings->endGroup();
}

void ManageBooksOnDeviceDialog::moveEvent(QMoveEvent * event)
{
    rememberGeometry();
    event->accept();
}
    
void ManageBooksOnDeviceDialog::resizeEvent(QResizeEvent * event)
{
    rememberGeometry();
    event->accept();
}

void ManageBooksOnDeviceDialog::closeEvent(QCloseEvent *event)
{
    QSettings * settings = Eclibrus::Config::settings();

    // write layout settings and sync
    rememberGeometry();
    settings->sync();
    event->accept();
}

