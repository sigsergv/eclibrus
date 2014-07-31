/*
 * eclibrenderers.cpp
 *
 * Created on: Jan 3, 2012
 * Author: Sergey Stolyarov
 */
#include <QtCore>
#include <QtSql>
#include <QTextDocument>

#include "eclibrenderers.h"
#include "eclibreply.h"
#include "settings.h"

namespace Eclibrus
{
namespace Html
{
    const QString PAGE_TEMPLATE("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><link rel=\"stylesheet\" href=\"qrc:/default.css\" type=\"text/css\">\
        <script language=\"javascript\" src=\"qrc:default.js\"></script> \
        </head><body>%1</body></html>");
    // how much items display on the page

    QString link(const QString & linkUrl, const QString & linkText, const QString & linkClass,
            bool escapeText)
    {
        QString data;

        if (!linkClass.isEmpty()) {
            data += QString(" class=\"%1\"").arg(linkClass);
        }

        QString lt (linkText);
        if (escapeText) {
            lt = lt.toHtmlEscaped();
        }

        return QString("<a href=\"%1\"%3>%2</a>")
            .arg(linkUrl.toHtmlEscaped())
            .arg(lt)
            .arg(data);
    }

    QString genreLink(int genreId, const QString & genreName)
    {
        return link(QString("eclib:genre?id=%1").arg(genreId), genreName);
    }

    QString sequenceLink(int seqId, const QString & seqName, int seqNum)
    {
        QString s(seqName);
        if (seqNum > 0) {
            s += QString(" (%1)").arg(seqNum);
        }
        return link(QString("eclib:sequence?id=%1").arg(seqId), s);
    }

    QString authorLink(int authorId, const QString & fname, const QString & mname, 
            const QString & lname, const QString & nick, const QString & role)
    {
        QStringList cs;
        QString cls = "author";
        QString post;
        QChar r = 'a';

        if (!fname.isEmpty()) {
            cs << fname;
        }
        if (!mname.isEmpty()) {
            cs << mname;
        }
        if (!lname.isEmpty()) {
            cs << lname;
        }
        if (!nick.isEmpty()) {
            cs << QString("[%1]").arg(nick);
        }

        if (role.size() >= 1) {
            r = role[0];
            switch (r.toLatin1()) {
            case 't':
                cls += " translator";
                post = QString(" <span class=\"translator\">(%1)</span>").arg(EclibReply::tr("translator"));
                break;
            case 'o':
                cls += " editor";
                break;
            }
        }

        QString display = cs.join(" ").toHtmlEscaped();

        return link(QString("eclib:author?id=%1").arg(authorId), display, cls, false) + post;
    }

    QString bookIcons(int bookId)
    {
        QString readIcon = QString("<div title=\"%1\" class=\"read-book-now\"></div>")
            .arg(EclibReply::tr("Read this book right now.").toHtmlEscaped());
        QString saveIcon = QString("<div title=\"%1\" class=\"save-to-device\"></div>")
            .arg(EclibReply::tr("Download this book to the device.").toHtmlEscaped());

        QString res;

        QString read = link(QString("http://%1/read-book-now?id=%2")
                .arg(::Eclibrus::Config::extUrlNamespace())
                .arg(bookId), readIcon, "", false);
        res += read;

        QString save = link(QString("http://%1/save-to-device?id=%2")
                .arg(::Eclibrus::Config::extUrlNamespace())
                .arg(bookId), saveIcon, "", false);
        res += save;

        if (Eclibrus::Config::showDownloadIcon()) {
            QString dlIcon = QString("<div title=\"%1\" class=\"download-book\"></div>")
                .arg(EclibReply::tr("Download book to disk.").toHtmlEscaped());
            QString dl = link(QString("http://%1/download-book?id=%2")
                .arg(::Eclibrus::Config::extUrlNamespace())
                .arg(bookId), dlIcon, "", false);
            res += dl;
        }
        return res; 
    }

    QString bookLink(int bookId, const QString & title, const QString & comment)
    {
        QString b = title;

        if (!comment.isEmpty()) {
            b += QString(" [%1]").arg(comment);
        }
        b = link(QString("eclib:book?id=%1").arg(bookId), b);
        QString icons = bookIcons(bookId);
        
        return QString("<span class=\"book\">%1 %2</span>").arg(icons, b);
    }

    /*
     * Display list of books
     * QSqlQuery must contain valid executed query starting 
     * with the following fields:
     *      book_id, book_title, book_comment
     * pass page==-1 if you want to get all results
     */
    QString booksList(QSqlQuery & q, int page, int * totalBooksRef)
    {
        typedef QPair<int, QString> BookItem;
        QList<BookItem> books;
        QList<int> book_ids;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        QString html;

        QString book;
        int books_count = 0;
        int book_id;
        int pageSize = Config::itemsPerPage();
        int range_left = page * pageSize;
        int range_right = pageSize * (page + 1);

        if (page == -1) {
            range_left = 0;
            range_right = 1000000;
        }

        while (q.next()) {
            // we are interested only in books from the range [range_left, range_right)
            if (books_count >= range_left && books_count < range_right) {
                book = QString("<div class=\"title\">%1. %2</div>")
                    .arg(books_count+1)
                    .arg( bookLink(Q_VI(q, 0), Q_VS(q, 1), Q_VS(q, 2)) );
                book_id = Q_VI(q, 0);
                books << QPair<int, QString>(book_id, book);
                book_ids << book_id;
            }
            ++books_count;
        }

        if (totalBooksRef) {
            *totalBooksRef = books_count;
        }
        //
        // define segment size
        const int sub_ids_limit = 500;

        QStringList subids;
        QStringList id_segments;
        QList<int>::iterator i;
        QList<int>::iterator k = book_ids.begin();
        QList<int>::iterator end = book_ids.end();
        while (true) {
            if (k == end) {
                break;
            }
            int n = 0;
            subids.clear();
            for (i=k; i!=end && n<sub_ids_limit; ++i, ++n) {
                subids << QString::number(*i);
            }
            id_segments << subids.join(",");
            k = i;
        }

        int bid;

        // fetch genres
        QHash<int, QStringList> booksGenresMap;
        foreach (const QString & segment, id_segments) {
            query.prepare(QString("SELECT g.genre_id, g.genre_name_ru, bg.book_id \
                FROM eclib_genre g JOIN eclib_book_genre bg \
                    ON bg.genre_id = g.genre_id AND bg.book_id IN (%1)").arg(segment));
            query.exec();
            while (query.next()) {
                bid = QV_I(2);
                if (!booksGenresMap.contains(bid)) {
                    booksGenresMap[bid] = QStringList();
                }
                booksGenresMap[bid] << genreLink(QV_I(0), QV_S(1));
            }
            query.finish();
        }

        // fetch series
        QHash<int, QStringList> booksSeqencesMap;
        foreach (const QString & segment, id_segments) {
            query.prepare(QString("SELECT s.seq_id, s.seq_name, bs.book_seq_num, bs.book_id \
                FROM eclib_sequence s JOIN eclib_book_seq bs \
                    ON bs.seq_id = s.seq_id AND bs.book_id IN (%1)").arg(segment));
            query.exec();
            while (query.next()) {
                bid = QV_I(3);
                if (!booksSeqencesMap.contains(bid)) {
                    booksSeqencesMap[bid] = QStringList();
                }
                booksSeqencesMap[bid] << sequenceLink(QV_I(0), QV_S(1), QV_I(2));
            }
            query.finish();
        }

        // fetch authors
        QHash<int, QStringList> booksAuthorsMap;
        foreach (const QString & segment, id_segments) {
            query.prepare(QString("SELECT a.author_id, a.author_fname, a.author_mname, a.author_lname, a.author_nick, \
                    ba.ba_role, ba.book_id \
                FROM eclib_book_author ba JOIN eclib_author a \
                    ON a.author_id = ba.author_id AND ba.book_id in (%1)").arg(segment));
            query.exec();
            while (query.next()) {
                bid = QV_I(6);
                if (!booksAuthorsMap.contains(bid)) {
                    booksAuthorsMap[bid] = QStringList();
                }
                booksAuthorsMap[bid] << authorLink(QV_I(0), QV_S(1), QV_S(2), QV_S(3), QV_S(4), QV_S(5));
            }
            query.finish();
        }

        if (books.size()) {
            html += QString("<div><span class=\"item\">%1</span></div>").arg(link(QString("http://%1/export-all-page-books")
                .arg(::Eclibrus::Config::extUrlNamespace()),
                EclibReply::tr("Download all books from this page to e-book reader")));
        }
        foreach (BookItem b, books) {
            book = "<div class=\"book\">";
            book += b.second;
            if (booksGenresMap.contains(b.first)) {
                book += QString("<div class=\"genres\">%1: %2</div>")
                    .arg(EclibReply::tr("Genre"))
                    .arg(booksGenresMap[b.first].join(", "));
            }
            if (booksSeqencesMap.contains(b.first)) {
                book += QString("<div class=\"sequences\">%1: %2</div>")
                    .arg(EclibReply::tr("Sequence"))
                    .arg(booksSeqencesMap[b.first].join(", "));
            }
            if (booksAuthorsMap.contains(b.first)) {
                book += QString("<div class=\"authors\">%1: %2</div>")
                    .arg(EclibReply::tr("Authors"))
                    .arg(booksAuthorsMap[b.first].join(", "));
            }
            book += "</div>";
            html += book;
        }
        
        return html;
    }

    QString pager(int page, int totalItems, const QString & linkTpl)
    {
        QString pager;
        int page_size = Eclibrus::Config::itemsPerPage();
        int total_pages = totalItems / page_size + (totalItems % page_size ? 0 : 1);
        int last_page = total_pages - 1; // remember: pages numbers start from 0
        if (page > 0) {
            // show link to first page
            pager += linkTpl.arg(0).arg(EclibReply::tr("first page"));
        }
        if (page-3 > 0) {
            pager += "…";
        }
        if (page-2 > 0) {
            pager += linkTpl.arg(page-2).arg(page-1);
        }
        if (page-1 > 0) {
            pager += linkTpl.arg(page-1).arg(page);
        }
        pager += QString("<span class=\"item\">%1</span>").arg(page+1);
        if (page+1 < last_page) {
            pager += linkTpl.arg(page+1).arg(page+2);
        }
        if (page+2 < last_page) {
            pager += linkTpl.arg(page+2).arg(page+3);
        }
        if (page+3 < last_page) {
            pager += "…";
        }
        if (page < last_page) {
            pager += linkTpl.arg(total_pages - 1).arg(EclibReply::tr("last page"));
        }
        
        return QString("<div class=\"inline pager\">%1</a>").arg(pager);
    }

}

namespace Plain
{
    QString bookFileName(int bookId)
    {
        // construct filename, use authors' last names, sequence/position in the sequence, title
        QString res;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        QStringList components;

        query.prepare("SELECT book_title FROM eclib_book WHERE book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        
        QString title;
        if (query.next()) {
            title = QV_S(0);
        }
        query.finish();

        query.prepare("SELECT a.author_lname \
            FROM eclib_author a JOIN eclib_book_author ba \
                ON a.author_id = ba.author_id AND ba.ba_role == \"a\" AND ba.book_id = :id ");
        query.bindValue(":id", bookId);
        query.exec();

        QStringList authors_list;
        int limit = 3;
        while (query.next() && limit>0) { 
            authors_list << QV_S(0);
            --limit;
        }
        query.finish();
        QString authors_str = authors_list.join(",");
        
        query.prepare("SELECT s.seq_name, bs.book_seq_num \
            FROM eclib_sequence s JOIN eclib_book_seq bs \
                ON s.seq_id = bs.seq_id AND bs.book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        
        QString sequence;
        while (query.next()) {
            sequence = QV_S(0);
            int sn = QV_I(1);
            if (sn) {
                sequence += QString("-%1").arg(sn);
            }
            break;
        }
        query.finish();

        if (!authors_str.isEmpty()) {
            components << authors_str;
        }
        if (!sequence.isEmpty()) {
            components << sequence;
        }
        if (!title.isEmpty()) {
            components << title;
        }

        res = components.join(" - ");

        // we need to also remove all characters not supported on vfat32 filesystem
        QRegExp restrictedChars("[/\\*?<>|:\"]");
        res = res.replace(restrictedChars, "");
        
        return res;
    }
    
    static QStringList fszHrPrefixes;
    QString fileSizeHumanReadable(qint64 num)
    {
        if (fszHrPrefixes.isEmpty()) {
            fszHrPrefixes << EclibReply::tr("KB") << EclibReply::tr("MB") 
                << EclibReply::tr("GB") << EclibReply::tr("TB");
        }
        
        QString unit = EclibReply::tr("bytes");
        QStringListIterator i(fszHrPrefixes);
        double dnum = num;
        
        while(dnum >= 1024.0 && i.hasNext()) {
            unit = i.next();
            dnum /= 1024.0;
        }
        return QString("%1 %2")
            .arg(QString::number(dnum, 'f', 2))
            .arg(unit);
    }
}
}
