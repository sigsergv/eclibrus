/*
 * preferencesdialog.cpp
 *
 * Created on: Jan 18, 2012
 * Author: Sergei Stolyarov
 */

#include <QtWidgets>
#include <QtDebug>

#include "settings.h"
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

struct PreferencesDialog::Private
{
    Ui::PreferencesDialog ui;
};

PreferencesDialog::PreferencesDialog(QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);

    p->ui.librusecArchivesErrorLabel->hide();

    connect(p->ui.selectLibrusecPathButton, SIGNAL(clicked()),
        this, SLOT(selectLibrusecArchivesPath()));

    p->ui.librusecArchivePathLineEdit->setText(Eclibrus::Config::librusecLibraryPath());
    p->ui.fb2ReaderExecutable->setText(Eclibrus::Config::fb2ReaderProgram());
    p->ui.showDownloadIconCheckbox->setChecked(Eclibrus::Config::showDownloadIcon());
}

PreferencesDialog::~PreferencesDialog()
{
    delete p;
}

void PreferencesDialog::accept()
{
    QString librusec_archives_path = p->ui.librusecArchivePathLineEdit->text();
    QFileInfo fi(librusec_archives_path);

    if (!fi.exists() || !fi.isDir()) {
        p->ui.librusecArchivesErrorLabel->setText(tr("Existing directory is required here!"));
        p->ui.librusecArchivesErrorLabel->show();
        return;
    }

    Eclibrus::Config::setLibrusecLibraryPath(librusec_archives_path);
    Eclibrus::Config::setFb2ReaderProgram(p->ui.fb2ReaderExecutable->text());
    Eclibrus::Config::setShowDownloadIcon(p->ui.showDownloadIconCheckbox->isChecked());
    QDialog::accept();
}

void PreferencesDialog::selectLibrusecArchivesPath()
{
    // open directory selection dialog
    QString current_dir = p->ui.librusecArchivePathLineEdit->text();
    QString new_dir = QFileDialog::getExistingDirectory(this, tr("Select LibRusEc archives directory"), 
        current_dir);

    p->ui.librusecArchivePathLineEdit->setText(new_dir);
}
