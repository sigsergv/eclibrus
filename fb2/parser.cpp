/*      
 * parser.cpp
 *      
 * Created on: Jan 1, 2012
 * Author: Sergei Stolyarov
 */     

#include <QtCore>
#include <QtDebug>
#include <QtXml>

#include "parser.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#define TAG_IN(_t_) if (localName == "_t_") { is_##_t_ = true; return true; }
#define TAG_OUT(_t_) if (localName == "_t_") { is_##_t_ = false; return true; }
namespace FB2
{
    const QString NS = "http://www.gribuser.ru/xml/fictionbook/2.0";
    const QString XLINK_NS = "http://www.w3.org/1999/xlink";

    /*
     * book metadata are stored in this struct, valid pointer to the structure
     * must be passed to the constructor of FictionBook2Handler
     */
    struct Metadata
    {
        QString annotation;
        QString base64Cover;
    };

    // class definition is here because we don't plan to use
    // it outside of this file
    class FictionBook2Handler : public QXmlDefaultHandler
    {
    public:
        FictionBook2Handler(Metadata * md);
        ~FictionBook2Handler();

        bool startElement(const QString & namespaceURI, const QString & localName,
            const QString & qName, const QXmlAttributes & attributes);
        bool endElement(const QString & namespaceURI, const QString & localName,
             const QString & qName);
        bool characters(const QString & str);
        //bool fatalError(const QXmlParseException & exception);
        QString errorString() const;

    private:
        Metadata * md;

        QStringList pathElements;
        QString path;
        QString error;
        QString tagName;

        // flags
        bool isFB;
        bool isCoverEl;
        QString coverLink;
    };

    FictionBook2Handler::FictionBook2Handler(Metadata * md)
    {
        this->md = md;

        isFB = false;
        isCoverEl = false;
        coverLink = "";
    }

    FictionBook2Handler::~FictionBook2Handler()
    {
    }

    bool FictionBook2Handler::startElement(const QString & namespaceURI, const QString & localName,
        const QString & /*qName*/, const QXmlAttributes & attributes)
    {
        if (namespaceURI != NS) {
            // ignore unknown elements
            return true;
        }
        if (!isFB && localName != "FictionBook" ) {
            error = "only FictionBook documents are supported";
            return false;
        }
        isFB = true;
        
        pathElements << localName;
        path = pathElements.join("/");
        tagName = localName;

        if (path == "FictionBook/description/title-info/annotation") {
            md->annotation = "";
            return true;
        }

        if (path == "FictionBook/description/title-info/coverpage/image") {
            QString v = attributes.value(XLINK_NS, "href");
            if (!v.isEmpty()) {
                if (v.startsWith("#")) {
                    v.remove(0, 1);
                }
                coverLink = v;
            }
            return true;
        }
        
        if (!coverLink.isEmpty() && path == "FictionBook/binary" && attributes.value("", "id") == coverLink) {
            isCoverEl = true;
        }

        //if (localName != "p") { qDebug() << localName; }
        //qDebug() << localName;
        return true;
    }

    bool FictionBook2Handler::endElement(const QString & namespaceURI, const QString & /*localName*/,
         const QString & /*qName*/)
    {
        if (namespaceURI != NS) {
            // ignore unknown elements
            return true;
        }

        if (isCoverEl && path == "FictionBook/binary") {
            isCoverEl = false;
        }

        pathElements.removeLast();
        path = pathElements.join("/");

        //qDebug() << "/" + localName;
        return true;
    }

    bool FictionBook2Handler::characters(const QString & str)
    {
        if (path == "FictionBook/description/title-info/annotation") {
            md->annotation += str;
        } else if (path.startsWith("FictionBook/description/title-info/annotation")) {
            md->annotation += QString("<%1>%2</%1>").arg(tagName).arg(str);
        } else if (isCoverEl) {
            md->base64Cover += str;
        }

        return true;
    }

    /*
    bool FictionBook2Handler::fatalError(const QXmlParseException & exception)
    {
        return false;
    }
    */

    QString FictionBook2Handler::errorString() const
    {
        return error; 
    }


    struct Parser::Private
    {
        QIODevice * iodevice;
        // we need to store QuaZip instance because if it destroyed before all
        // date are read the segfault will come and eat you
        QuaZip * zip;
        Metadata md;
    };

    Parser::Parser(QIODevice * device, QObject * parent)
        : QObject(parent), error(Parser::NoError)
    {
        p = new Private();
        p->iodevice = device;
        p->zip = 0;
    }

    Parser::Parser(QuaZip * zip, QuaZipFile * zipfile, QObject * parent)
        : QObject(parent), error(Parser::NoError)
    {
        p = new Private();
        p->iodevice = zipfile;
        p->zip = zip;
    }

    Parser::~Parser()
    {
        if (p->zip) {
            delete p->zip;
        }
        delete p;
    }

    void Parser::parse()
    {
        FictionBook2Handler handler(&(p->md));
        QXmlSimpleReader reader;
        reader.setContentHandler(&handler);
        reader.setErrorHandler(&handler);
        QXmlInputSource * source = new QXmlInputSource(p->iodevice);

        qDebug() << "start FB2 parsing";
        if (!reader.parse(source)) {
            qDebug() << "failed to parse XML:" << handler.errorString();
        } else {
            // "p->md.annotation" contains raw unparsed contents of corresponding xml element
            // i.e. it has to be cleaned up before passing outside

            //qDebug() << p->md.base64Cover;
        }
        qDebug() << "FB2 parsed";
    }

    QString Parser::annotation()
    {
        return p->md.annotation;
    }

    QString Parser::base64Cover()
    {
        return p->md.base64Cover;
    }

    Parser * Parser::fromZip(const QString & zipFile, const QString & filename, QObject * parent)
    {
        QFileInfo fi(zipFile);
        if (!fi.isFile() || !fi.isReadable()) {
            Parser * p = new Parser(0, parent);
            p->setError(ZipArchiveFileNotFound);
            return p;
        }

        // open "zipFile" archive
        QuaZip * z = new QuaZip(zipFile);
        z->open(QuaZip::mdUnzip);
        if (z->getZipError() != 0) {
            Parser * p = new Parser(0, parent);
            p->setError(ZipError);
            delete z;
            return p;
        }

        z->setCurrentFile(filename);

        QuaZipFile * zf = new QuaZipFile(z, parent);

        if (!zf->open(QIODevice::ReadOnly)) {
            Parser * p = new Parser(0, parent);
            p->setError(ZipError);
            zf->close();
            z->close();
            delete z;
            return p;
        }

        if (zf->getActualFileName().isEmpty()) {
            Parser * p = new Parser(0, parent);
            p->setError(ZipFileInArchiveNotFound);
            zf->close();
            z->close();
            delete z;
            return p;
        }

        //qDebug() << "test zip read" << zf->read(0).size();
        Parser * p = new Parser(z, zf, parent);

        return p;
    }

    Parser::Error Parser::getError()
    {
        return error;
    }

    void Parser::setError(Parser::Error error)
    {
        this->error = error;
    }
}
