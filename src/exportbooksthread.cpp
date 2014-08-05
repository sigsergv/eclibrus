/*
 * exportbooksthread.cpp
 *
 * Created on: Jan 09, 2012
 * Author: Sergey Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "devices.h"
#include "exportbooksthread.h"
#include "db.h"
#include "eclibrenderers.h"
#include "settings.h"
#include "fb2/fb2.h"
#include "webdav/qwebdav_types.h"
#include "webdav/qwebdav.h"

struct ExportThread::Private
{
    QList<int> books;
    QString outputDir;
    Eclibrus::DeviceInfo di;
};

ExportThread::ExportThread(const QList<int> & books, const Eclibrus::DeviceInfo & di, const QString & outputDir)
{
    p = new Private;
    p->books = books;
    p->outputDir = outputDir;
    p->di = di;
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

    switch (p->di.devType) {
        case Eclibrus::DeviceInfo::MSD: {
            // copy books to mass storage devuce
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
            break;
        }

        case Eclibrus::DeviceInfo::WEBDAV: {
            QWebDav wd;
            QUrl uri(p->di.uri);

            QString host = uri.host();
            int port = uri.port();
            if (port == -1) {
                port = 80;
            }

            wd.connectToHost(host, port, uri.path(), uri.userName(), uri.password());
            if (QWebDav::NoError != wd.lastError()) {
                qWarning() << "Failed to init WebDav link: " << p->di.uri;
                return;
            }

            // upload books to WEBDAV device
            // p->outputDir is a collection path on webdav device, files should be placed there
            foreach (const QString & archive, archive_map.keys()) {
                archive_path = library_path + archive;
                foreach (const QStringList & book_info, archive_map[archive]) {
                    ++n;

                    // export to temporary file
                    QTemporaryFile tf;
                    tf.setAutoRemove(false);
                    tf.open();
                    output_path = tf.fileName();
                    qDebug() << "filename:" << output_path;
                    tf.close();
                    FB2::exportBookToArchive(archive_path, book_info[0], output_path);

                    // now copy file "output_path" to the device
                    QString webdavName = p->outputDir + "/" + book_info[1] + ".fb2.zip";
                    qDebug() << "webdav name" << webdavName;
                    wd.put(output_path, webdavName);
                    if (QWebDav::NoError != wd.lastError()) {
                        qWarning() << "Failed to copy local file to WebDav device";
                    }
                    tf.remove();
                }
            }
            break;
        }
    }
}
