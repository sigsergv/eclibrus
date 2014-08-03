#ifndef PROPFINDPARSER_H
#define PROPFINDPARSER_H

#include <QObject>
#include <QList>

#include "qwebdav_types.h"

class QIODevice;

class PropfindParser : public QObject
{
    Q_OBJECT
public:
    enum Error {NoError, InvalidDocument, UnexpectedElement, UnexpectedDocumentEnd};

    PropfindParser(QObject * parent = 0);
    ~PropfindParser();

    void setDevice(QIODevice * device);
    QList<WebDavItem> parse();
    Error lastError();

private:
    struct Private;
    Private * p;
};

#endif