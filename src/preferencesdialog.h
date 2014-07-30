/*
 * preferencesdialog.h
 *
 * Created on: Jan 18, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _PREFERENCESDIALOG_H_
#define _PREFERENCESDIALOG_H_

#include <QDialog>

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget * parent = 0);
    ~PreferencesDialog();

public slots:
    void accept();

protected slots:
    void selectLibrusecArchivesPath();

private:
    struct Private;
    Private * p;
};

#endif // _PREFERENCESDIALOG_H_
