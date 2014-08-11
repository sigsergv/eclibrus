#include <QtCore>
#include <QtDebug>

#include "qwebdav.cpp"
#include "qwebdav_types.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QWebDav wd;
    QWebDav::Error error;
    wd.connectToHost("192.168.1.35", 48073, "dav", "dav");
    
    error = wd.lastError();

    if (error != QWebDav::NoError) {
        switch (error) {
            case QWebDav::AuthFailedError:
                qDebug() << "Auth failed";
                break;

            case QWebDav::ConnectionTimeoutError:
                qDebug() << "Connection timeout";
                break;

            case QWebDav::NetworkError:
                qDebug() << "Network error";
                break;

            case QWebDav::XmlParsingError:
                qDebug() << "XML parsing error";
                break;

            case QWebDav::NoError:
                break;
        }
        return 1;
    }

    QList<WebDavItem> items = wd.list("/%D0%92%D0%B0%D1%81%D0%B8%D0%BB%D0%B8%D0%B9%20%D0%95%D1%80%D1%88%D0%BE%D0%B2/");
    error = wd.lastError();

    if (error != QWebDav::NoError) {
        switch (error) {
            case QWebDav::AuthFailedError:
                qDebug() << "Auth failed";
                break;

            case QWebDav::ConnectionTimeoutError:
                qDebug() << "Connection timeout";
                break;

            case QWebDav::NetworkError:
                qDebug() << "Network error";
                break;

            case QWebDav::XmlParsingError:
                qDebug() << "XML parsing error";
                break;

            case QWebDav::NoError:
                break;
        }
    }

    foreach (const WebDavItem & item, items) {
        qDebug() << item.davStatus << item.name << item.type << item.href;
    }
    
}
