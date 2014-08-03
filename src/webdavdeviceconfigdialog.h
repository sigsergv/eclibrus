/*
 * webdavdeviceconfigdialog.h
 *
 * Created on: Aug 02, 2014
 * Author: Sergey Stolyarov
 */

#ifndef _WEBDAVDEVICECONFIGDIALOG_H_
#define _WEBDAVDEVICECONFIGDIALOG_H_

#include <QDialog>
 #include "devices.h"

class WebDavDeviceConfigDialog : public QDialog
{
    Q_OBJECT
public:
    WebDavDeviceConfigDialog(QWidget * parent = 0);
    ~WebDavDeviceConfigDialog();

    Eclibrus::DeviceInfo device();

public slots:
    void accept();

private:
    struct Private;
    Private * p;
};

#endif // _WEBDAVDEVICECONFIGDIALOG_H_
