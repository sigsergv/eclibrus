/*
 * aboutdialog.cpp
 *
 * Created on: Jan 5, 2012
 * Author: Sergey Stolyarov
 */

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

struct AboutDialog::Private
{
    Ui::AboutDialog ui;
};


AboutDialog::AboutDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);

    // modify About text, add version info
    QString aboutText = p->ui.aboutLabel->text();
    aboutText.replace("{version}", QString(ECLIBRUS_VERSION));
    aboutText.replace("{qt_version}", QString(QT_VERSION_STR));
    p->ui.aboutLabel->setText(aboutText);
}
