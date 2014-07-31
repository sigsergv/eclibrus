/*
 * db.cpp
 *
 * Created on: Dec 30, 2011
 * Author: Sergey Stolyarov
 */

#include <QtSql>
#include <QtDebug>

#include "db.h"
#include "settings.h"

namespace Eclibrus
{
namespace Db
{

    QSqlError init()
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        /*
         * to use database further in code you should obtain db connection handle:
         * 
         * QSqlDatabase db = QSqlDatabase::database();
         * QSqlQuery query(db);
         */
        db.setDatabaseName(Config::filenameInProfile("librusec.eclib"));

        if (!db.open()) {
            return db.lastError();
        }

        // TODO: check db integrity and scheme

        return QSqlError();
    }

    QPair<QString, QString> archivedBookFile(int bookId)
    {
        QString filename = QString("%1.fb2").arg(bookId);
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);

        query.prepare("SELECT a.archive_filename \
            FROM eclib_book_archive a JOIN eclib_book b \
                ON b.archive_file_id = a.archive_file_id AND b.book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();

        if (!query.next()) {
            return QPair<QString, QString>();
        }
        QString archive = query.value(0).toString();

        return QPair<QString, QString>(archive, filename);
    }
}
}
