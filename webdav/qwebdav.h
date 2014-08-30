#ifndef QWEBDAV_H
#define QWEBDAV_H

#include <QtCore>
#include <QtNetwork>

#include "qwebdav_types.h"

/**
 * WebDav connection class.
 *
 * It implements very basic support of WEBDAV protocol.
 */
class QWebDav : public QNetworkAccessManager
{
    Q_OBJECT
public:
    enum Error {NoError=0, AuthFailedError, ConnectionTimeoutError, NetworkError, XmlParsingError, NoSourceLocalFile, NotFound};

    QWebDav(QObject* parent = 0);
    ~QWebDav();

    Error lastError();
    void connectToHost(const QString & hostName, quint16 port, const QString & startPath, const QString & username, const QString & password);

    QList<WebDavItem> list(const QString & path, bool recursive = false);
    void checkdir(const QString & path);
    void mkdir(const QString & path);
    void put(const QString & localPath, const QString & webdavPath);
    void remove(const QString & path, bool decoded = true);

protected:
    QNetworkReply * davRequest(const QString& method, QNetworkRequest& req, const QByteArray& outgoingData);
    QNetworkReply * davRequest(const QString& method, QNetworkRequest& req, QIODevice* outgoingData);
    void connectToHost(const QString & hostName, quint16 port = 80);

protected slots:
    void provideAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);

private:
    struct Private;
    Private * p;
    // hide this method

signals:
    void authFailed();
};
#endif
