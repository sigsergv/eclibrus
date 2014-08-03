/*
 * webdavdeviceconfigdialog.cpp
 *
 * Created on: Aug 02, 2014
 * Author: Sergey Stolyarov
 */

#include <QMessageBox>
#include <QUrl>
#include <QtDebug>

#include "webdavdeviceconfigdialog.h"
#include "ui_webdavdeviceconfigdialog.h"

struct WebDavDeviceConfigDialog::Private
{
    Ui::WebDavDeviceConfigDialog ui;
    Eclibrus::DeviceInfo di;
};

WebDavDeviceConfigDialog::WebDavDeviceConfigDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);

    p->ui.serverHostPortErrorLabel->hide();
    p->ui.serverPort->setValidator(new QIntValidator(1, 65535));
}

WebDavDeviceConfigDialog::~WebDavDeviceConfigDialog()
{
    delete p;
}

Eclibrus::DeviceInfo WebDavDeviceConfigDialog::device()
{
    return p->di;
}

void WebDavDeviceConfigDialog::accept()
{
    // check entered data
    Eclibrus::DeviceInfo di;

    QString hostname = p->ui.serverHost->text();
    QString port = p->ui.serverPort->text();
    QString username = p->ui.username->text();
    QString password = p->ui.password->text();
    QString path = p->ui.path->text();

    if (hostname.isEmpty()) {
        p->ui.serverHostPortErrorLabel->setText(tr("Empty hostname is not allowed"));
        p->ui.serverHostPortErrorLabel->show();
        p->ui.serverHost->setFocus();
        return;
    }
    p->ui.serverHostPortErrorLabel->hide();

    if (port.isEmpty()) {
        p->ui.serverHostPortErrorLabel->setText(tr("Empty server port is not allowed"));
        p->ui.serverHostPortErrorLabel->show();
        p->ui.serverPort->setFocus();
        return;
    }
    p->ui.serverHostPortErrorLabel->hide();

    if (path.isEmpty()) {
        path = "/";
    }

    di.uuid = "uuid"; // mark that device is not empty
    di.devType = Eclibrus::DeviceInfo::WEBDAV;
    QUrl uri;
    uri.setScheme("http");
    uri.setHost(hostname);
    uri.setPort(port.toInt());
    uri.setPath(path);

    if (!username.isEmpty()) {
        uri.setUserName(username);
    }

    if (!password.isEmpty()) {
        uri.setPassword(password);
    }

    di.uri = uri.toString();
    di.name = "DAV://" + hostname + ":" + port;

    p->di = di;

    QDialog::accept();
}