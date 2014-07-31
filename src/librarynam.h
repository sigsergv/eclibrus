/*
 * librarynam.h
 *
 * Created on: Dec 28, 2011
 * Author: Sergey Stolyarov
 */

#ifndef LIBRARYNAM_H_
#define LIBRARYNAM_H_

#include <QNetworkAccessManager>

namespace Eclibrus
{

    class LibraryNAM : public QNetworkAccessManager
    {
        Q_OBJECT
    public:
        LibraryNAM(QNetworkAccessManager * oldManager, QObject * parent = 0);
    protected:
        QNetworkReply * createRequest(Operation operation, const QNetworkRequest & request, QIODevice * device);
    };
}

#endif /* LIBRARYNAM_H_ */
