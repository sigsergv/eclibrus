#include <QtCore>
#include <QtDebug>
#include <QtNetwork>

#include "qwebdav.h"
#include "eventloop.h"
#include "propfindparser.h"
#include "qwebdav_types.h"


struct QWebDav::Private
{
    QUrl baseUrl;
    QString username;
    QString password;
    QWebDav::Error lastError;

    QNetworkReply * lastAuthReply;
};

QWebDav::QWebDav(QObject* parent)
    : QNetworkAccessManager(parent)
{
    p = new Private();
    p->lastError = NoError;

    connect(this, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(provideAuthentication(QNetworkReply*,QAuthenticator*)));
}

QWebDav::~QWebDav()
{
    delete p;
}

QWebDav::Error QWebDav::lastError()
{
    return p->lastError;
}


void QWebDav::connectToHost(const QString & hostName, quint16 port)
{
    Q_UNUSED(hostName);
    Q_UNUSED(port);
    // Do nothing
}

void QWebDav::connectToHost(const QString & hostName, quint16 port, const QString & startPath, const QString & username, const QString & password) 
{
    p->baseUrl.setScheme("http");
    p->baseUrl.setHost(hostName);
    p->baseUrl.setPort(port);
    p->username = username;
    p->password = password;

#ifdef Q_OS_MAC
    // workaround for MacOSX, see http://stackoverflow.com/questions/15707124/
    QNetworkProxy proxy = this->proxy();
    proxy.setHostName(" ");
    setProxy(proxy);
#endif

    QUrl reqUrl(p->baseUrl);
    QNetworkRequest request;

    reqUrl.setPath(startPath);
    request.setUrl(reqUrl);

    p->lastError = NoError;
    EventLoop * loop;
    loop = new EventLoop();
    QNetworkReply * reply = davRequest("GET", request, QByteArray());
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    QTimer::singleShot(2000, loop, SLOT(quitTimeout()));  // 2 seconds timeout
    loop->exec();

    if (loop->status() == EventLoop::StatusTimeout && reply->isRunning()) {
        // connection is not established, so terminated it and emit corresponding signal
        reply->abort();
        reply->deleteLater();
        qDebug() << "connection timeout";
        p->lastError = ConnectionTimeoutError;
        return;
    }
    if (p->lastError != AuthFailedError && reply->error() != QNetworkReply::NoError) {
        p->lastError = NetworkError;
        qDebug() << reply->errorString();
        reply->abort();
        reply->deleteLater();
        return;
    }
    delete loop;

    // connection successful, so destroy reply and continue
    reply->abort();
    reply->deleteLater();
}


QNetworkReply * QWebDav::davRequest(const QString& method, QNetworkRequest& req, QIODevice* outgoingData)
{
    if(outgoingData != 0 && outgoingData->size() !=0) {
        req.setHeader(QNetworkRequest::ContentLengthHeader, outgoingData->size());
        req.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=utf-8");
    }

    return sendCustomRequest(req, method.toLatin1(), outgoingData);
}


QNetworkReply * QWebDav::davRequest(const QString & method, QNetworkRequest & req, const QByteArray& outgoingData)
{ 
    QBuffer* dataIO = new QBuffer;
    dataIO->setData(outgoingData);
    dataIO->open(QIODevice::ReadOnly);

    QNetworkReply* reply = davRequest(method, req, dataIO);
    return reply;
}

/**
 * List of items for given path
 * @param path
 */
QList<WebDavItem> QWebDav::list(const QString & path, bool recursive)
{
    QUrl reqUrl(p->baseUrl);
    QNetworkRequest request;

    reqUrl.setPath(path, QUrl::StrictMode);
    request.setUrl(reqUrl);
    request.setRawHeader(QByteArray("Depth"), "1");

    p->lastError = NoError;
    EventLoop * loop;
    loop = new EventLoop();

    QByteArray query;
    query = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    query += "<D:propfind xmlns:D=\"DAV:\" >";
    query += "<D:prop>";

    // query += "<D:creationdate/>";
    query += "<D:getcontentlength/>";
    query += "<D:displayname/>";
    query += "<D:resourcetype/>";
    // query += "<D:getcontentlanguage/>";

    query += "</D:prop>";
    query += "</D:propfind>";

    QNetworkReply * reply = davRequest("PROPFIND", request, query);
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    loop->exec();

    if (p->lastError == AuthFailedError) {
        qDebug() << "Auth failed";
        reply->abort();
        reply->deleteLater();
        return QList<WebDavItem>();
    }

    if (reply->error() != QNetworkReply::NoError) {
        p->lastError = NetworkError;
        qDebug() << reply->errorString();
        reply->abort();
        reply->deleteLater();
        return QList<WebDavItem>();
    }
    delete loop;

    // parse response
    PropfindParser parser;
    parser.setDevice(reply);
    QList<WebDavItem> items;
    foreach (const WebDavItem & item, parser.parse()) {
        // exclude some items
        if (item.type == WebDavItemDirectory && item.href == path) {
            continue;
        }
        items.append(item);
    }

    if (parser.lastError() != PropfindParser::NoError) {
        qDebug() << "failed to parse " << parser.lastError();
        p->lastError = XmlParsingError;
        return QList<WebDavItem>();
    }

    if (recursive) {
        QList<WebDavItem> newItems;
        // fetch all child nodes
        foreach (const WebDavItem & item, items) {
            if (item.type == WebDavItemDirectory) {
                QList<WebDavItem> subItems = list(item.href, true);
                if (p->lastError != NoError) {
                    qDebug() << "Failed to list subitems for collection" << item.href;
                    return QList<WebDavItem>();
                }
                newItems.append(subItems);
            } else {
                newItems.append(item);
            }
        }
        items = newItems;
    }

    return items;
}


void QWebDav::mkdir(const QString & path)
{
    QUrl reqUrl(p->baseUrl);
    QNetworkRequest request;

    reqUrl.setPath(path, QUrl::StrictMode);
    request.setUrl(reqUrl);

    p->lastError = NoError;
    EventLoop * loop;
    loop = new EventLoop();

    QByteArray query;
    QNetworkReply * reply = davRequest("MKCOL", request, query);
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    loop->exec();

    if (p->lastError == AuthFailedError) {
        qDebug() << "Auth failed";
        reply->abort();
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        p->lastError = NetworkError;
        qDebug() << reply->errorString();
        reply->abort();
        reply->deleteLater();
        return;
    }
    delete loop;
}


void QWebDav::put(const QString & localPath, const QString & webdavPath)
{
    QUrl reqUrl(p->baseUrl);
    QNetworkRequest request;

    reqUrl.setPath(webdavPath, QUrl::DecodedMode);
    request.setUrl(reqUrl);
    request.setRawHeader(QByteArray("Translate"), "f");
    request.setRawHeader(QByteArray("Content-Type"), "application/octet-stream");

    p->lastError = NoError;
    EventLoop * loop;
    loop = new EventLoop();

    QFile f(localPath);
    f.open(QIODevice::ReadOnly);

    QByteArray data = f.readAll();

    if (data.isEmpty()) {
        p->lastError = NoSourceLocalFile;
        return;
    }

    request.setRawHeader(QByteArray("Content-Length"), QByteArray::number(data.size()));

    QNetworkReply * reply = davRequest("PUT", request, data);
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    loop->exec();

    if (p->lastError == AuthFailedError) {
        qDebug() << "Auth failed";
        reply->abort();
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        p->lastError = NetworkError;
        qDebug() << reply->errorString();
        reply->abort();
        reply->deleteLater();
        return;
    }
    delete loop;
}

void QWebDav::remove(const QString & path)
{
    QUrl reqUrl(p->baseUrl);
    QNetworkRequest request;

    reqUrl.setPath(path, QUrl::DecodedMode);
    request.setUrl(reqUrl);

    reqUrl.setPath(path, QUrl::DecodedMode);
    request.setUrl(reqUrl);

    p->lastError = NoError;
    EventLoop * loop;
    loop = new EventLoop();

    QNetworkReply * reply = davRequest("DELETE", request, QByteArray());
    connect(reply, SIGNAL(finished()), loop, SLOT(quit()));
    loop->exec();

    if (p->lastError == AuthFailedError) {
        qDebug() << "Auth failed";
        reply->abort();
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        p->lastError = NetworkError;
        qDebug() << reply->errorString();
        reply->abort();
        reply->deleteLater();
        return;
    }
    delete loop;
}


void QWebDav::provideAuthentication(QNetworkReply * reply, QAuthenticator * authenticator)
{
    // qDebug() << "auth requested";

    if (reply == p->lastAuthReply) {
        reply->abort();
        reply->deleteLater();
        p->lastAuthReply = 0;
        p->lastError = AuthFailedError;
        return;
    }

    authenticator->setUser(p->username);
    authenticator->setPassword(p->password);

    p->lastAuthReply = reply;
}
