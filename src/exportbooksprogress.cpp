/*
 * exportbooksprogress.cpp
 *
 * Created on: Jan 09, 2012
 * Author: Sergey Stolyarov
 */

#include <QtWidgets>
#include <QtCore>
#include <QtDebug>

#include "devices.h"
#include "exportbooksprogress.h"
#include "exportbooksthread.h"
#include "ui_exportbooksprogress.h"
#include "webdav/qwebdav_types.h"
#include "webdav/qwebdav.h"

struct ExportBooksProgress::Private
{
    Ui::ExportBooksProgress ui;
    ExportThread * thread;
};

ExportBooksProgress::ExportBooksProgress(const QList<int> books, const Eclibrus::DeviceInfo & di, const QString & outputDir, QWidget * parent)
    : QDialog(parent)
{
    p = new Private;
    p->ui.setupUi(this);
    p->thread = new ExportThread(books, di, outputDir);
    p->ui.progressBar->setMaximum(100);
    p->ui.progressBar->setMinimum(0);
    
    connect(p->thread, SIGNAL(progress(int)),
        this, SLOT(progressChanged(int)));
    connect(p->thread, SIGNAL(finished()),
        this, SLOT(reject()));
}

ExportBooksProgress::~ExportBooksProgress()
{
    delete p->thread;
    delete p;
}

void ExportBooksProgress::setVisible(bool visible)
{
    qDebug() << "show/hide";
    QDialog::setVisible(visible);
    if (visible) {
        p->thread->start();
    }
}

void ExportBooksProgress::progressChanged(int percent)
{
    p->ui.progressBar->setValue(percent);
}
