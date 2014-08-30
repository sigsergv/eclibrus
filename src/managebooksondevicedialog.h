/*
 * managebooksondevicedialog.h
 *
 * Created on: Jan 13, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _MANAGEBOOKSONDEVICEDIALOG_H_
#define _MANAGEBOOKSONDEVICEDIALOG_H_

#include <QDialog>


class ManageBooksOnDeviceDialog : public QDialog
{
    Q_OBJECT
public:
    ManageBooksOnDeviceDialog(QWidget * parent = 0);
    ~ManageBooksOnDeviceDialog();
    virtual void setVisible(bool visible);

protected slots:
    void deviceChanged(int);
    void deleteSelectedBooks();

protected:
    enum Role { DeviceInfoRole = Qt::UserRole+1 };

    void rememberGeometry();
    void moveEvent(QMoveEvent * event);
    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);

private:
    struct Private;
    Private * p;
};

#endif // _MANAGEBOOKSONDEVICEDIALOG_H_
