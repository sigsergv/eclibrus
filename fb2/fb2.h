/*
 * fb2.h
 *
 * Various useful functions.
 *
 * Created on: Jan 08, 2012
 * Author: Sergey Stolyarov
 */

#ifndef FB2_FB2_H_
#define FB2_FB2_H_

#include <QString>
#include <QList>

namespace FB2
{
    struct ExportBookInfo
    {
        /*
         * Full path to zip file
         */
        QString archivePath;

        /*
         * book filename in the archive
         */
        QString archiveFilename;

        /*
         * filename that will be used for exported book, it's just a file name
         * not a full path, i.e. it must not contain full path characters (/, :, \ etc)
         */
        QString exportFilename;
    };
    typedef QList<ExportBookInfo> ExportBookInfoList;

    /*
     * Export book from archive to another archive
     */
    bool exportBookToArchive(const QString & archivePath, const QString & archiveFilename, 
            const QString & targetPath);
}

#endif // FB2_FB2_H_
