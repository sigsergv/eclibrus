/*
 * addnewdevicedialog.h
 *
 * Created on: Jan 08, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _ADDNEWDEVICEDIALOG_H_
#define _ADDNEWDEVICEDIALOG_H_

#include <QDialog>

#include "devices.h"

class AddNewDeviceDialog : public QDialog
{
    Q_OBJECT
public:
    AddNewDeviceDialog(QWidget * parent = 0);
    ~AddNewDeviceDialog();
    Eclibrus::DeviceInfo selectedDevice();

protected slots:
    void accept();

private:
    struct Private;
    Private * p;
};

#endif // _ADDNEWDEVICEDIALOG_H_
