/*
 * eclibreply.cpp
 *
 * Created on: Dec 28, 2011
 * Author: Sergey Stolyarov
 */

#include <QtDebug>
#include <QtCore> 
#include <QTextDocument>
#include <QTimer>
#include <QtSql>

#include "eclibreply.h"
#include "eclibrenderers.h"
#include "settings.h"
#include "fb2/parser.h"

// some QSqlQuery related useful macros

namespace Eclibrus
{

    /*
     * Perform a basic search, display links to more precise results if they are available
     */
    QString EclibReply::plainSearch(const QString & text)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        QString text_lo = text.toLower();
        int limit;

        // first look through genres
        query.prepare("SELECT genre_id, genre_name_ru "
            "FROM eclib_genre WHERE genre_search_str GLOB :s");
        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();

        QStringList genres;
        int genre_id;

        while (query.next()) {
            genre_id = QV_I(0);
            genres << Html::genreLink(genre_id, QV_S(1));
        }
        query.finish();
    
        qDebug() << "genres found" << genres.size();
        if (genres.size()) {
            html += QString("<h2>%1</h2> <div class=\"links-list\">%2</div>")
                .arg(tr("Genres found"))
                .arg(genres.join(", "));
        }

        // then look through the sequences/series
        query.prepare("SELECT seq_id, seq_name FROM eclib_sequence WHERE seq_search_str GLOB :s");
        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();
        
        QStringList sequences;
        int seq_id;

        while (query.next()) {
            seq_id = QV_I(0);
            sequences << Html::sequenceLink(seq_id, QV_S(1), -1);
        }
        query.finish();

        qDebug() << "sequences found" << sequences.size();
        if (sequences.size()) {
            html += QString("<h2>%1</h2> <div class=\"links-list\">%2</div>")
                .arg(tr("Series found"))
                .arg(sequences.join(", "));
        }

        // find matching authors
        query.prepare("SELECT a.author_id, a.author_fname, a.author_mname, "
                "a.author_lname, a.author_nick, COUNT(ba.book_author_id) cnt "
                "FROM eclib_author a LEFT JOIN eclib_book_author ba "
                "ON ba.author_id = a.author_id "
                "WHERE author_search_str GLOB :s "
                "GROUP BY a.author_id "
                "ORDER BY cnt DESC "
                "LIMIT 15");

        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();

        QStringList authors;
        int author_id;

        limit = 0;
        while (query.next()) {
            if (limit++ > 10) {
                break;
            }
            author_id = QV_I(0);
            authors << Html::authorLink(author_id, QV_S(1),
                    QV_S(2), QV_S(3), 
                    QV_S(4), "a");
        }
        query.finish();

        qDebug() << "authors found" << authors.size();
        if (authors.size()) {
            html += QString("<h2>%1</h2> <div class=\"links-list\">%2</div>")
                .arg(tr("Authors found"))
                .arg(authors.join(", "));
        }

        if (limit > 10) {
            QUrl url("eclib:search/authors");
            QUrlQuery urlQuery;
            urlQuery.addQueryItem("q", text);
            url.setQuery(urlQuery);
            html += QString("<div>%1</div>")
                .arg(Html::link(url.toString(), tr("More authors are available, click here to show them."), "more"));
        }

        // find matching books
        query.prepare("SELECT book_id, book_title, book_comment \
                FROM eclib_book \
                WHERE book_search_str GLOB :s LIMIT 15");

        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();

        QStringList books;
        int book_id;
    
        limit = 0;
        while (query.next()) {
            if (limit++ > 10) {
                break;
            }
            book_id = QV_I(0);
            books << Html::bookLink(book_id, QV_S(1), QV_S(2));
        }
        query.finish();

        if (books.size()) {
            html += QString("<h2>%1</h2> <div class=\"links-list\">%2</div>")
                .arg(tr("Books found"))
                .arg(books.join(", "));
        }

        if (limit > 10) {
            QUrl url("eclib:search/books");
            QUrlQuery urlQuery;            
            urlQuery.addQueryItem("q", text);
            url.setQuery(urlQuery);
            html += QString("<div>%1</div>")
                .arg(Html::link(url.toString(), tr("More books are available, click here to show them."), "more"));
        }

        if (html.size() == 0) {
            html = QString("<em>%1</em>")
                .arg(tr("No results for the query."));
        }
        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::authorsSearch(const QString & text)
    {
        QString html;
        QString text_lo = text.toLower();
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);

        query.prepare("SELECT a.author_id, a.author_fname, a.author_mname, a.author_lname, "
                "a.author_nick, COUNT(ba.book_author_id) cnt "
                "FROM eclib_author a LEFT JOIN eclib_book_author ba "
                "ON ba.author_id = a.author_id "
                "WHERE a.author_search_str GLOB :s "
                "GROUP BY a.author_id "
                "ORDER BY cnt DESC");
        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();
        int author_id;
        int cnt;

        html = QString("<h2>%1</h2>")
            .arg(tr("Authors found"));
        html += QString("<div><strong>%1</strong></div>")
            .arg(tr("Author name / number of books"));
        html += "<hr>";


        while (query.next()) {
            author_id = QV_I(0);
            cnt = QV_I(5);

            QString a = Html::authorLink(author_id, QV_S(1),
                    QV_S(2), QV_S(3), 
                    QV_S(4), "a");
            html += QString("<div>%1 / %2</div>")
                .arg(a)
                .arg(cnt);
        }
        query.finish();

        return Html::PAGE_TEMPLATE.arg(html);
    }

    /*
     * Search for books, display all found results
     */
    QString EclibReply::booksSearch(const QString & text)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        QString text_lo = text.toLower();
    
        // we need to display book title, authors
        query.prepare("SELECT b.book_id, b.book_title, b.book_comment, \
            a.author_id, a.author_fname, a.author_mname, a.author_lname, a.author_nick, \
            ba.ba_role \
                FROM eclib_book b \
                    JOIN eclib_book_author ba \
                        ON b.book_search_str GLOB :s AND ba.book_id = b.book_id \
                    JOIN eclib_author a\
                        ON a.author_id = ba.author_id ORDER BY b.book_id");

        query.bindValue(":s", QString("*%1*").arg(text_lo));
        query.exec();

        QString comment;
        QString author;

        int prev_book_id = -1;
        int book_id;
        int author_id;
        bool next_available = true;

        while (true) {
            next_available = query.next();

            if (!next_available) {
                if (prev_book_id != -1) {
                    // finalize book rendering
                    html += "</div></div>";
                }
                break;
            }
            book_id = QV_I(0);
            author_id = QV_I(3); 

            author = Html::authorLink(author_id, QV_S(4), QV_S(5),
                QV_S(6), QV_S(7), QV_S(8));

            if (prev_book_id != -1 && book_id == prev_book_id) {
                // continue adding authors
                // 3,4,5,6,7
                html += ", " + author;
            } else {
                if (prev_book_id != -1) {
                    // close previous section
                    html += "</div></div>";
                }
                // start new section
                html += "<div class=\"book\"><div class=\"title\">";
                html += Html::bookLink(book_id, QV_S(1), QV_S(2));
                html += "</div><div class=\"authors\">" + author;
            }
           
            prev_book_id = book_id;
        }

        html = QString("<h2>Books search</h2><div class=\"books-search\">%1</div>").arg(html);

        return Html::PAGE_TEMPLATE.arg(html);
    }

    /*
     * Display detailed information about book: title, description, authors, genres, cover page
     */
    QString EclibReply::showBookCard(int bookId)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        
        html = "<div class=\"book-card\">"; 

        query.prepare("SELECT b.book_title, b.book_comment, ar.archive_filename \
            FROM eclib_book b JOIN eclib_book_archive ar \
                ON b.archive_file_id = ar.archive_file_id \
            WHERE book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        if (!query.next()) {
            return "404";
        }
        QString book_title = QV_S(0);
        QString book_comment = QV_S(1);
        QString bookArchiveFile = QV_S(2);
        query.finish();

        QString book_icons = Eclibrus::Html::bookIcons(bookId);

        html += QString("<div class=\"title\">%1 %2</div>")
            .arg(book_icons)
            .arg(book_title);

        if (!book_comment.isEmpty()) {
            html += QString("<div class=\"comment\">%1</div>").arg(book_comment);
        }

        html += "<div class=\"mid-vspacer\"></div>";

        qDebug() << "Authors:";
        QStringList authors;
        query.prepare("SELECT a.author_id, a.author_fname, a.author_mname, a.author_lname, a.author_nick, \
                ba.ba_role \
            FROM eclib_book_author ba JOIN eclib_author a \
                ON a.author_id = ba.author_id AND ba.book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        while (query.next()) {
            // fetch all authors
            authors << Html::authorLink(QV_I(0), QV_S(1), QV_S(2),
                QV_S(3), QV_S(4), QV_S(5));
        }
        query.finish();

        html += QString("<div class=\"authors\">%1: %2</div>")
            .arg(tr("Authors"))
            .arg(authors.join(", "));

        //html += "<div class=\"mid-vspacer\"></div>";

        qDebug() << "Genres:";
        QStringList genres;
        query.prepare("SELECT g.genre_id, g.genre_name_ru \
            FROM eclib_book_genre ga JOIN eclib_genre g \
                ON g.genre_id = ga.genre_id AND ga.book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        while (query.next()) {
            // fetch all genres
            genres << Html::genreLink(QV_I(0), QV_S(1));
        }
        query.finish();
        
        if (genres.size()) {
            html += QString("<div class=\"genres\">%1: %2</div>")
                .arg(tr("Genre"))
                .arg(genres.join(", "));
        }

        // fetch sequences chain
        qDebug() << "Sequences:";
        QStringList sequences;
        query.prepare("SELECT s.seq_id, s.seq_name, s.seq_parent, bs.book_seq_num \
            FROM eclib_book_seq bs JOIN eclib_sequence s \
                ON bs.seq_id = s.seq_id AND bs.book_id = :id");
        query.bindValue(":id", bookId);
        query.exec();
        while (query.next()) {
            // fetch all sequences
            qDebug() << QV_S(1);
            sequences << Html::sequenceLink(QV_I(0), QV_S(1), QV_I(3));
        }
        query.finish();

        if (sequences.size()) {
            html += QString("<div class=\"sequences\">%1: %2</div>")
                .arg(tr("Sequences"))
                .arg(sequences.join(", "));
        }

        // now we need to parse book file to fetch description, cover etc
        QString archive = Eclibrus::Config::librusecLibraryPath() + QDir::separator() + bookArchiveFile;
        QString archiveFilename = QString("%1.fb2").arg(bookId);

        html += "<div class=\"mid-vspacer\"></div>";

        FB2::Parser * fbp = FB2::Parser::fromZip(archive, archiveFilename, this);
        if (fbp && fbp->getError() == FB2::Parser::NoError) {
            fbp->parse();

            if (fbp->getError() == FB2::Parser::NoError) {
                // book is found, it seems to be ok, so display now features and data
                // depending on book file
                QString coverHtml;

                // fetch cover image, encoded using base64, decode it and save to the cache
                QString base64Cover = fbp->base64Cover();
                if (!base64Cover.isEmpty()) {
                    QString cachePath = Eclibrus::Config::coversCachePath();
                    QString coverCachePath = cachePath + QDir::separator() + QString("%1.dat").arg(bookId);
                    QFileInfo fi(coverCachePath);

                    if (!fi.exists()) {
                        QByteArray coverRawData = QByteArray::fromBase64(base64Cover.toLatin1());
                        QFile cover(coverCachePath);
                        if (cover.open(QIODevice::WriteOnly)) {
                            cover.write(coverRawData);
                            cover.close();
                        } else {
                            // failed to write
                            coverCachePath.clear();
                        }
                    } else if (!fi.isFile()) {
                        coverCachePath.clear();
                    }

                    if (!coverCachePath.isEmpty()) {
                        fi.setFile(coverCachePath);
                        // file is written successfuly, place img tag on the page
                        QString u = "file://" + fi.canonicalFilePath();
                        // try to detect image dimensions
                        QImage img(coverCachePath);
                        QString imgSize;
                        if (!img.isNull()) {
                            int maxWidth = Eclibrus::Config::maxCoverPreviewWidth();
                            if (img.size().width() > maxWidth) {
                                imgSize = QString(" width=\"%1\"").arg(maxWidth);
                            }
                        }
                        coverHtml = QString("<div class=\"cover\"><img src=\"%1\"%2></div>")
                            .arg(u)
                            .arg(imgSize);
                    }
                }

                // fetch annotation
                QString annotation = fbp->annotation();
                QString annHtml = QString("<div class=\"annotation\">%1</div>").arg(annotation);

                html += "<table border=\"0\"><tr>";
                if (!coverHtml.isEmpty()) {
                    html += QString("<td valign=\"top\">%1</td>").arg(coverHtml);
                }
                html += QString("<td valign=\"top\">%1</td>").arg(annHtml);
                html += "</tr></table>";
            }
        } else {
            // analyze error
            QString error_tpl("<div class=\"error\">%1</div>");
            FB2::Parser::Error e = fbp->getError();
            html += error_tpl.arg(tr("Error: Unable to open ZIP-archive with book files."));

            if (e == FB2::Parser::ZipError) {
                html += error_tpl.arg(tr("Unknown ZIP-error"));
            } else if (e == FB2::Parser::ZipArchiveFileNotFound) {
                html += error_tpl.arg(tr("ZIP-archive file “%1” not found")
                    .arg(archive));
            } else if (e == FB2::Parser::ZipFileInArchiveNotFound) {
                html += error_tpl.arg(tr("File “%1” is not found in the archive “%2”")
                    .arg(archiveFilename)
                    .arg(archive));
            }
        }
        html += "</div>";

        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::showAuthor(int authorId)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        
        // fetch author info
        query.prepare("SELECT a.author_fname, a.author_mname, a.author_lname, a.author_nick \
            FROM eclib_author a \
            WHERE a.author_id = :id");
        query.bindValue(":id", authorId);
        query.exec();
        if (!query.next()) {
            return "<h1>404</h1>";
        }
        
        QString authorFullName = QV_S(0) + " " + QV_S(1) + " " + QV_S(2);
        if (!QV_S(3).isEmpty()) {
            authorFullName += QString(" (%1)").arg(QV_S(3));
        }
        query.finish();

        
        html = "<div class=\"author-card\">"; 
        html += QString("<h2>%1</h2>").arg(authorFullName);

        // fetch books written or translated by the author
        query.prepare("SELECT b.book_id, b.book_title, b.book_comment \
                FROM eclib_book b JOIN eclib_book_author ba \
                    ON b.book_id = ba.book_id AND ba.author_id = :id \
                    GROUP BY b.book_id");
        query.bindValue(":id", authorId);
        query.exec();
        //qWarning() << "query fail" << query.lastError().text();

        html += Html::booksList(query, -1);

        html += "</div>";

        return Html::PAGE_TEMPLATE.arg(html);
        
    }

    QString EclibReply::showGenre(int genreId, int page)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        
        // fetch genre info
        query.prepare("SELECT genre_name_ru FROM eclib_genre WHERE genre_id = :id");
        query.bindValue(":id", genreId);
        query.exec();
        if (!query.next()) {
            return "<h1>404</h1>";
        }
        QString genreName = QV_S(0);
        query.finish();

        html = "<div class=\"genre-card\">"; 
        html += QString("<h2>%1: %2</h2>")
            .arg(tr("Genre"))
            .arg(genreName);

        query.prepare("SELECT b.book_id, b.book_title, b.book_comment \
            FROM eclib_book b JOIN eclib_book_genre bg \
                ON b.book_id = bg.book_id AND bg.genre_id = :id");
        query.bindValue(":id", genreId);
        query.exec();
        
        int total_books;
        int page_size = Eclibrus::Config::itemsPerPage();
        QString books = Html::booksList(query, page, &total_books);

        QString link_tpl = QString("<a href=\"eclib:genre?id=%1&page=%2\">%3</a>")
            .arg(QString::number(genreId), "%1", "%2");
        QString url_tpl = QString("eclib:genre?id=%1&page={page}").arg(QString::number(genreId));
        html += QString("<div>%1</div><div>%2</div>")
            .arg(tr("Total books: %1, showing from %2 to %3")
                .arg(total_books)
                .arg(page_size * page + 1)
                .arg(page_size * (page + 1)))
            .arg(Html::pager(page, total_books, link_tpl, url_tpl));
        html += books;

        html += "</div>";
        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::showSequence(int seqId)
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        
        // fetch genre info
        query.prepare("SELECT s1.seq_name, s1.seq_parent, s2.seq_id, s2.seq_name \
            FROM eclib_sequence s1 LEFT JOIN eclib_sequence s2 \
                ON s1.seq_parent = s2.seq_id \
            WHERE s1.seq_id = :id");
        query.bindValue(":id", seqId);
        query.exec();
        if (!query.next()) {
            return "<h1>404</h1>";
        }
        QString sequenceName = QV_S(0);
        int parentSeqId = QV_I(2);
        QString parentSeqName = QV_S(3);
        query.finish();

        html = "<div class=\"sequence-card\">"; 
        html += QString("<h2>%1: %2</h2>")
            .arg(tr("Series"))
            .arg(sequenceName);

        if (parentSeqId) {
            html += QString("<h3>%1</h3>")
                .arg(tr("Part of the series: %1")
                    .arg(Html::sequenceLink(parentSeqId, parentSeqName, -1)));
        }

        query.prepare("SELECT b.book_id, b.book_title, b.book_comment \
            FROM eclib_book b JOIN eclib_book_seq bs \
                ON b.book_id = bs.book_id AND bs.seq_id = :id \
            ORDER BY bs.book_seq_num");
        query.bindValue(":id", seqId);
        query.exec();
        
        html += Html::booksList(query, -1);

        html += "</div>";
        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::showLibrarySummaryInfo()
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);

        html = "<div class=\"library-summary\">";
        html += QString("<h2>%1</h2>")
            .arg(tr("Library summary"));
        html += QString("<div class=\"p\">%1</div>")
            .arg(tr("Summary and general statistics about data stored in the "
                "library."));

        query.prepare("SELECT COUNT(*) FROM eclib_book");
        query.exec();
        query.next();
        int total_books = QV_I(0);
        query.finish();

        query.prepare("SELECT book_lang, COUNT(book_id) FROM eclib_book"
            " GROUP BY book_lang ORDER BY 2 DESC");
        query.exec();
        QHash<QString, int> book_lang_counts;
        QStringList langs;
        while (query.next()) {
            QString lang = QV_S(0);
            int cnt = QV_I(1);
            langs << lang;
            book_lang_counts[lang] = cnt;
        }

        html += QString("<div class=\"p\">");
        html += tr("Library contains <strong>%n</strong> books ", "", total_books);
        html += tr("on <strong>%n</strong> languages", "", langs.size());
        html += QString("<span id=\"langs-show-link\"> "
            "<span class=\"link\" onclick=\"expandBlock('langs-block', 'langs-show-link', 'langs-hide-link')\">%1</span></span>")
            .arg(tr("show"));
        html += QString("<span id=\"langs-hide-link\" style=\"display:none;\"> "
            "<span class=\"link\" onclick=\"collapseBlock('langs-block', 'langs-show-link', 'langs-hide-link')\">%1</span></span>")
            .arg(tr("hide"));
        html += QString(".");
        html += QString("</div>");

        QHash<QString, QString> lang_titles;
        lang_titles["ru"] = tr("Russian");
        lang_titles["en"] = tr("English");
        lang_titles["es"] = tr("Spanish");
        lang_titles["uk"] = tr("English (UK)");
        lang_titles["pl"] = tr("Polish");
        lang_titles["fr"] = tr("French");
        lang_titles["bg"] = tr("Bulgarian");
        lang_titles["de"] = tr("German");
        lang_titles["lv"] = tr("Latvian");
        lang_titles["be"] = tr("Belarus");
        lang_titles["it"] = tr("Italian");
        lang_titles["cs"] = tr("Czech");
        lang_titles["eo"] = tr("Esperanto");
        lang_titles["hu"] = tr("Hungarian");
        lang_titles["ro"] = tr("Romanian");
        lang_titles["nl"] = tr("Dutch");
        lang_titles["pt"] = tr("Portugese");
        /*
        lang_titles["sr"] = tr("-sr-");
        lang_titles["lt"] = tr("-lt-");
        lang_titles["zh"] = tr("-zh-");
        lang_titles["cu"] = tr("-cu-");
        lang_titles["sk"] = tr("-sk-");
        lang_titles["la"] = tr("-la-");
        lang_titles["uz"] = tr("Uzbek");
        lang_titles["sv"] = tr("-sv-");
        lang_titles["vi"] = tr("-vi-");
        lang_titles["az"] = tr("-az-");
        lang_titles["hr"] = tr("-hr-");
        lang_titles["tr"] = tr("-tr-");
        lang_titles[""] = tr("");
        */

        html += "<div id=\"langs-block\" style=\"display:none;\"><table border=\"0\">";
        html += QString("<tr> <th>%1</th> <th>%2</th> </tr>")
            .arg(tr("language"))
            .arg(tr("books on that language"));
        int total_other = 0;
        foreach (const QString lang, langs) {
            if (!lang_titles.contains(lang)) {
                total_other += book_lang_counts[lang];
                continue;
            }
            QString lang_title = lang_titles[lang];

            html += QString("<tr> <td>%1</td> <td>%2</td> </tr>")
                .arg(lang_title)
                .arg(book_lang_counts[lang]);
        }
        html += QString("<tr> <td>%1</td> <td>%2</td> </tr>")
            .arg(tr("Other"))
            .arg(total_other);
        html += "</table>";
        html += "</div>";

        html += "</div>";

        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::showGenresSummaryInfo()
    {
        QString html;
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);

        query.prepare("SELECT g.genre_id, g.genre_name_ru, COUNT(bg.book_id) cnt "
            "FROM eclib_genre g LEFT JOIN eclib_book_genre bg "
            "ON g.genre_id = bg.genre_id "
            "GROUP BY g.genre_id "
            "ORDER BY cnt DESC");
        query.exec();

        html = QString("<h2>%1</h2>")
            .arg(tr("All genres"));
        html += QString("<div><strong>%1</strong></div><hr>")
            .arg(tr("Genre name / books number"));

        while (query.next()) {
            html += QString("<div>%1 (%2)</div>")
                .arg(Html::genreLink(QV_I(0), QV_S(1)))
                .arg(QV_I(2));
        }
        query.finish();

        return Html::PAGE_TEMPLATE.arg(html);
    }

    QString EclibReply::generalError(const QString & message, bool escape)
    {
        QString msg = message;
        if (escape) {
            msg = message.toHtmlEscaped();
        }
        return Html::PAGE_TEMPLATE.arg(tr("<h3>Error: %1</h3>").arg(msg));
    }

    struct EclibReply::Private
    {
        // this field will contain generated html content
        QByteArray content;
        qint64 contentSize;
        qint64 offset;
    };

    EclibReply::EclibReply(const QUrl & url)
        : QNetworkReply()
    {
        p = new Private();
        /*
         * Parse url, extract request details, perform database query etc
         */
        QUrlQuery urlQuery(url);
        QString method = url.path();
        QString q = urlQuery.queryItemValue("q");
        QString sid = urlQuery.queryItemValue("id");
        QString url_str = url.toString();

        qDebug() << "URL requested: " << url_str;

        QString html = "";
        QString redirect_url;

        if (method == "search/plain" && !q.isEmpty()) {
            html = plainSearch(q);
        } else if (method == "search/authors"&& !q.isEmpty()) {
            html = authorsSearch(q);
        } else if (method == "search/books" && !q.isEmpty()) {
            html = booksSearch(q);
        } else if (method == "book" && !sid.isEmpty()) {
            int id = sid.toInt();
            html = showBookCard(id);
        } else if (method == "author" && !sid.isEmpty()) {
            int id = sid.toInt();
            html = showAuthor(id);
        } else if (method == "sequence") {
            int id = sid.toInt();
            html = showSequence(id);
        } else if (method == "info/genres-summary") {
            html = showGenresSummaryInfo();
        } else if (method == "info/library-summary") {
            html = showLibrarySummaryInfo();
        } else if (method == "genre" && !sid.isEmpty()) {
            int id = sid.toInt();
            int page = urlQuery.queryItemValue("page").toInt();
            html = showGenre(id, page);
            /* // disable caching for now
            // check in cache
            QByteArray b64url = url_str.toAscii().toBase64();
            b64url = b64url.replace("/", "_");
            QString path = Eclibrus::Config::pagesCachePath() + QDir::separator() + QString::fromAscii(b64url) + ".html";
            QFileInfo fi(path);

            qDebug() << path;
            redirect_url = "file://" + path;
            if (!fi.exists()) {
                html = showGenre(id);
                QFile f(path);
                if (!f.open(QIODevice::WriteOnly)) {
                    qDebug() << "failed to open";
                }
                f.write(html.toUtf8());
                f.close();
                html.clear();
            }
            */
        } else {
            html = generalError(tr("Unknown eclib method <em>%1</em>!").arg(method.toHtmlEscaped()), false);
        }

        p->content = html.toUtf8();
        p->contentSize = p->content.size();
        p->offset = 0;

        setUrl(url);
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        
        if (!redirect_url.isEmpty()) {
            setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(redirect_url));
            //setHeader(QNetworkRequest::LocationHeader, redirect_url);
        } else {
            setHeader(QNetworkRequest::ContentTypeHeader, 
                QByteArray("text/html; charset=UTF-8"));
            setHeader(QNetworkRequest::ContentLengthHeader, 
                QByteArray::number(p->contentSize));
        }

        // prepare and execute sql query

        QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
        QTimer::singleShot(0, this, SIGNAL(readyRead()));
        QTimer::singleShot(100, this, SIGNAL(finished()));
    }

    EclibReply::~EclibReply()
    {
        delete p;
    }

    void EclibReply::abort()
    {
    
    }

    qint64 EclibReply::bytesAvailable() const
    {

        qint64 bc = QIODevice::bytesAvailable() + p->contentSize - p->offset;
        return bc;
    }

    bool EclibReply::isSequential() const
    {
        return true;
    }

    qint64 EclibReply::readData(char * data, qint64 maxSize)
    {
        if (p->offset >= p->contentSize) {
            return -1;
        }
        qint64 count = qMin(maxSize, p->contentSize - p->offset);
        memcpy(data, p->content.constData() + p->offset, count);
        p->offset += count;
        return count;
    }
}
