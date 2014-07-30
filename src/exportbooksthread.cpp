/*
 * exportbooksthread.cpp
 *
 * Created on: Jan 09, 2012
 * Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "exportbooksthread.h"
#include "db.h"
#include "eclibrenderers.h"
#include "settings.h"
#include "fb2/fb2.h"

struct ExportThread::Private
{
    QList<int> books;
    QString outputDir;
};

ExportThread::ExportThread(const QList<int> & books, const QString & outputDir)
{
    p = new Private;
    p->books = books;
    p->outputDir = outputDir;
}

ExportThread::~ExportThread()
{
    delete p;
}

void ExportThread::run()
{
    QList< QStringList > archived_books;
    QHash<QString, QList<QStringList> > archive_map; // archive_name => book_info
    QStringList books_ids;
    QPair<QString, QString> pair;
    QStringList bi;
    QString basename;

    foreach (int book_id, p->books) {
        pair = Eclibrus::Db::archivedBookFile(book_id);
        basename = Eclibrus::Plain::bookFileName(book_id);
        if (!archive_map.contains(pair.first)) {
            archive_map[pair.first] = QList<QStringList>();
        }
        bi.clear();
        bi << pair.second << basename;
        archive_map[pair.first] << bi;
    }

    int total_books = p->books.size();
    QString library_path = Eclibrus::Config::librusecLibraryPath() + QDir::separator();
    QString archive_path;
    QString output_path;
    QFileInfo fi;

    const int INITIAL_PROGRESS = 2;
    emit progress(INITIAL_PROGRESS);
    float ppf = (100 - INITIAL_PROGRESS) / total_books;
    int n = 0;
    int percent = 0;

    foreach (const QString & archive, archive_map.keys()) {
        archive_path = library_path + archive;

        foreach (const QStringList & book_info, archive_map[archive]) {
            ++n;
            output_path = p->outputDir + QDir::separator() + book_info[1] + ".fb2.zip";
            fi.setFile(output_path);
            if (true/*!fi.exists()*/) {
                FB2::exportBookToArchive(archive_path, book_info[0], output_path);
            } else {
                qDebug() << "file already exists:" << output_path;
            }
            percent = (int)(ppf*n);
            emit progress(percent+INITIAL_PROGRESS);
        }
    }
}
