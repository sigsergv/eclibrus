#ifndef QWEBDAV_TYPES_H
#define QWEBDAV_TYPES_H

#include <QString>

enum WebDavItemType {WebDavItemFile, WebDavItemDirectory};
/**
 * Structure that represents one resource item (file or directory).
 *
 * All names are UTF-8 encoded.
 */
struct WebDavItem {
    WebDavItemType type;
    QString href;
    QString name;
    QString davStatus;
};

#endif