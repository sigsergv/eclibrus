/*
 * aboutdialog.h
 *
 * Created on: Jan 5, 2012
 * Author: Sergey Stolyarov
 */
#ifndef _ABOUTDIALOG_H_
#define _ABOUTDIALOG_H_

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget * parent = 0);

private:
    struct Private;
    Private * p;
};

#endif

