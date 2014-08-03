/*
 * devicesconfigdialog.h
 *
 * Created on: Jan 7, 2012
 * Author: Sergey Stolyarov
 */
#ifndef _DEVICESCONFIGDIALOG_H_
#define _DEVICESCONFIGDIALOG_H_

#include <QDialog>

class DevicesConfigDialog : public QDialog
{
    Q_OBJECT
public:
    DevicesConfigDialog(QWidget * parent = 0);
    ~DevicesConfigDialog();

protected:
    void reloadDevicesList();

protected slots:
    void addNewDevice();
    void addWebDavDevice();
    void deleteSelectedDevices();

private:
    struct Private;
    Private * p;

};

#endif // _DEVICESCONFIGDIALOG_H_
