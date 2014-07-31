/*
 * eclibreply.h
 *
 * Created on: Dec 28, 2011
 * Author: Sergey Stolyarov
 */

#ifndef ECLIBREPLY_H_
#define ECLIBREPLY_H_

#include <QNetworkReply>

class QUrl;

namespace Eclibrus
{
    class EclibReply : public QNetworkReply 
    {
        Q_OBJECT
    public:
        EclibReply(const QUrl & url);
        ~EclibReply();
        void abort();
        qint64 bytesAvailable() const;
        bool isSequential() const;

    protected:
        qint64 readData(char *data, qint64 maxSize);
        QString plainSearch(const QString & text);
        QString authorsSearch(const QString & text);
        QString booksSearch(const QString & text);
        QString showBookCard(int bookId);
        QString showAuthor(int authorId);
        QString showGenre(int genreId, int page);
        QString showSequence(int sequenceId);
        QString showLibrarySummaryInfo();
        QString showGenresSummaryInfo();
        QString generalError(const QString & message, bool escape = true);

    private:
        struct Private;
        Private * p;

    };
}

#endif // ECLIBREPLY_H_
