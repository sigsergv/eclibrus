#include <QtCore>

#include "propfindparser.h"

const QString DAV_NS_URI("DAV:");

struct PropfindParser::Private
{
public:
    QIODevice * device;
    PropfindParser::Error lastError;
    QXmlStreamReader xml;

    bool isDavElement(const QString name) 
    {
        return xml.name() == name && xml.namespaceUri() == DAV_NS_URI;
    }

    QList<WebDavItem> parse();

private:
    struct Propstat {
        enum ResourceType {None, Default, Collection}; 
        ResourceType resourceType;
        QString status;
        QString displayname;
    };

    WebDavItem parseResponse();
    Propstat parsePropstat();
};

QList<WebDavItem> PropfindParser::Private::parse()
{
    QList<WebDavItem> items;

    if (!isDavElement("multistatus")) {
        lastError = UnexpectedElement;
        qDebug() << "Unexpected element" << xml.qualifiedName() << "instead of <dav:multistatus>";
        return QList<WebDavItem>();
    }

    while (xml.readNextStartElement()) {
        // read <dav:response> elements
        WebDavItem item = parseResponse();
        if (lastError != NoError) {
            break;
        }
        items.append(item);
    }
    return items;
}


WebDavItem PropfindParser::Private::parseResponse()
{
    WebDavItem item;

    if (!isDavElement("response")) {
        lastError = UnexpectedElement;
        qDebug() << "Unexpected element" << xml.qualifiedName() << "instead of <dav:response>";
        return WebDavItem();
    }
    bool foundHref = false;
    bool foundPropstat = false;

    while (xml.readNextStartElement()) {
        if (isDavElement("href")) {
            if (foundHref) {
                lastError = UnexpectedElement;
                qDebug() << "Unexpected element" << xml.qualifiedName() << "it is read already";
                return WebDavItem();
            }
            foundHref = true;
            item.href = xml.readElementText();
        } else if (isDavElement("propstat")) {
            /*
            if (foundPropstat) {
                lastError = UnexpectedElement;
                qDebug() << "Unexpected element" << xml.qualifiedName() << "it is read already";
                return WebDavItem();
            }
            foundPropstat = true;
            */
            Propstat propstat = parsePropstat();
            if (lastError != NoError) {
                return WebDavItem();
            }
            if (!propstat.status.isEmpty()) {
                item.davStatus = propstat.status;
            }
            if (!propstat.displayname.isEmpty()) {
                item.name = propstat.displayname;
            }

            switch (propstat.resourceType) {
                case Propstat::Default:
                    item.type = WebDavItemFile;
                    break;

                case Propstat::Collection:
                    item.type = WebDavItemDirectory;
                    break;
            }
        } else {
            qDebug() << "Unexpected element" << xml.qualifiedName() << "instead of <dav:href> or <dav:propstat>";
            return WebDavItem();
        }
    }
    return item;
}

PropfindParser::Private::Propstat PropfindParser::Private::parsePropstat()
{
    Propstat propstat;
    propstat.resourceType = Propstat::None;

    bool foundStatus = false;
    bool foundProp = false;

    while (xml.readNextStartElement()) {
        if (isDavElement("status")) {
            if (foundStatus) {
                lastError = UnexpectedElement;
                qDebug() << "Unexpected element" << xml.qualifiedName() << "it is read already";
                return Propstat();
            }
            foundStatus = true;
            propstat.status = xml.readElementText();
        } else if (isDavElement("prop")) {
            if (foundProp) {
                lastError = UnexpectedElement;
                qDebug() << "Unexpected element" << xml.qualifiedName() << "it is read already";
                return Propstat();
            }
            foundProp = true;
            // parse props list
            while (xml.readNextStartElement()) {
                if (isDavElement("displayname")) {
                    propstat.displayname = xml.readElementText();
                } else if (isDavElement("resourcetype")) {
                    propstat.resourceType = Propstat::Default;

                    while (xml.readNextStartElement()) {
                        if (isDavElement("collection")) {
                            propstat.resourceType = Propstat::Collection;
                        }
                        xml.skipCurrentElement();
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            qDebug() << "Unexpected element" << xml.qualifiedName() << "instead of <dav:href> or <dav:propstat>";
            return Propstat();
        }
    }

    return propstat;
}

PropfindParser::PropfindParser(QObject * parent)
    :QObject(parent)
{
    p = new Private;
    p->device = 0;
}

PropfindParser::~PropfindParser()
{
    delete p;
}


void PropfindParser::setDevice(QIODevice * device)
{
    p->device = device;
    p->xml.setDevice(device);
    p->xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("dav", DAV_NS_URI));
}

PropfindParser::Error PropfindParser::lastError()
{
    return p->lastError;
}

QList<WebDavItem> PropfindParser::parse()
{
    QList<WebDavItem> res;
    p->lastError = NoError;

    if (p->xml.readNextStartElement()) {
        // expected <dav:multistatus>
        res = p->parse();
    } else {
        qDebug() << "invalid document";
        p->lastError = InvalidDocument;
        return QList<WebDavItem>();
    }

    return res;
}