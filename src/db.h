/*
 * db.h
 *
 * Created on: Dec 30, 2011
 * Author: Sergey Stolyarov
 */
#ifndef DB_H_
#define DB_H_

#include <QSqlError>
#include <QPair>
#include <QString>

namespace Eclibrus
{

namespace Db
{
    /*
     * Initialize db, prepare db connection
     */
    QSqlError init();

    QPair<QString, QString> archivedBookFile(int bookId);
}
}
#endif
