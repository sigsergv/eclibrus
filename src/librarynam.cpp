/*
 * librarynam.cpp
 *
 * Created on: Dec 28, 2011
 * Author: Sergei Stolyarov
 */

#include <QtNetwork>

#include "librarynam.h"
#include "eclibreply.h"

namespace Eclibrus
{
    LibraryNAM::LibraryNAM(QNetworkAccessManager * oldManager, QObject * parent)
        : QNetworkAccessManager(parent)
    {
        setCache(oldManager->cache());
        setCookieJar(oldManager->cookieJar());
        setProxy(oldManager->proxy());
        setProxyFactory(oldManager->proxyFactory());
    }

    /*
     * custom URL scheme handler
     */
    QNetworkReply * LibraryNAM::createRequest(Operation operation, const QNetworkRequest & request, QIODevice * device)
    {
        if (request.url().scheme() != "eclib") {
            return QNetworkAccessManager::createRequest(operation, request, device);
        }

        if (operation == GetOperation) {
            // create custom "network" response object to handle "eclib://" request
            EclibReply * reply = new EclibReply(request.url());
            // trigger query
            return reply;
        } else {
            // only GET requests are supported by now
            return QNetworkAccessManager::createRequest(operation, request, device);
        }

    }
}
