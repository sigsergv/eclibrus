/*      
 * parser.h
 *      
 * Created on: Jan 1, 2012
 * Author: Sergei Stolyarov
 */     

#ifndef FB2_PARSER_H_
#define FB2_PARSER_H_

#include <QObject>
#include <QString>

class QIODevice;
class QuaZip;
class QuaZipFile;

namespace FB2
{
    class FileNotFoundInArchive;

    /*
     * Simple class for reading FictionBook2 file metadata and cover image.
     */
    class Parser : public QObject
    {
        Q_OBJECT
    public:
        enum Error {NoError, 
            ZipError,                 // common ZIP-error
            ZipArchiveFileNotFound,   // i.e. zip-file not found
            ZipFileInArchiveNotFound, // i.e. file in the archive not found
            ZipReadError};

        Parser(QIODevice * device, QObject * parent = 0);
        Parser(QuaZip * zip, QuaZipFile * zipfile, QObject * parent = 0);
        ~Parser();
        void parse();
        QString annotation();
        QString base64Cover();
        Error getError();
        static Parser * fromZip(const QString & zipFile, const QString & filename, QObject * parent = 0);

    protected:
        Error error;
        void setError(Error error);

    private:
        struct Private;
        Private * p;

    };
}

#endif // FB2_PARSER_H_

