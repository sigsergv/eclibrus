/*
 * eclibrenderers.h
 *
 * Common HTML-renderers for EclibReply class
 *
 * Created on: Jan 3, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _ECLIBRENDERERS_H_
#define _ECLIBRENDERERS_H_

#include <QString>

#define QV_S(n) (query.value((n))).toString()
#define QV_I(n) (query.value((n))).toInt()
#define Q_VS(q,n) ((q).value((n))).toString()
#define Q_VI(q,n) ((q).value((n))).toInt()

class QSqlQuery;

namespace Eclibrus
{
namespace Html
{
    extern const QString PAGE_TEMPLATE;

    QString link(const QString & linkUrl, const QString & linkText, const QString & linkClass = QString(),
            bool escapeText = true);
    QString genreLink(int genreId, const QString & genreName);
    QString sequenceLink(int seqId, const QString & seqName, int seqNum);
    QString authorLink(int authorId, const QString & fname, const QString & mname, 
            const QString & lname, const QString & nick, const QString & role);
    QString bookIcons(int bookId);
    QString bookLink(int bookId, const QString & title, const QString & comment);
    QString booksList(QSqlQuery & q, int page = 0, int * totalBooksRef = 0);
    QString pager(int page, int totalItems, const QString & linkTpl);
}

namespace Plain
{
    /*
     * Construct book file name using data from the database
     */
    QString bookFileName(int bookId);
    QString fileSizeHumanReadable(qint64 num);
}
}

#endif // _ECLIBRENDERERS_H_
