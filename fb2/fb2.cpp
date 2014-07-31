/*
 * fb2.cpp
 *
 * Created on: Jan 08, 2012
 * Author: Sergey Stolyarov
 */

#include <QtDebug>

#include "fb2.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/quazipnewinfo.h"

namespace FB2
{
    bool exportBookToArchive(const QString & archivePath, const QString & archiveFilename, 
            const QString & targetPath)
    {
        // we need to open ZIP-file "archivePath", extract file "archiveFilename" from there,
        // then compress it again with the name "targetPath"
        QuaZip in_z(archivePath);
        if (!in_z.open(QuaZip::mdUnzip)) {
            qWarning() << "failed: in_z.open(QuaZip::mdUnzip)";
            return false;
        }

        if (!in_z.setCurrentFile(archiveFilename)) {
            qWarning() << "setCurrentFile(archiveFilename) failed";
            return false;
        }
        // TODO: get rid of re-compression
        QuaZipFile in_zf(&in_z);
        if (!in_zf.open(QIODevice::ReadOnly)) {
            qWarning() << "cannot open input ZIP file for reading";
            return false;
        }

        QuaZip out_z(targetPath);
        if (!out_z.open(QuaZip::mdCreate)) {
            return false;
        }
        QuaZipFile out_zf(&out_z);
        QuaZipNewInfo out_zfni(archiveFilename);
        out_zf.open(QIODevice::WriteOnly, out_zfni);

        // now read and write
        const int buf_sz = 4096;
        char buf[buf_sz];
        qint64 read;
        qint64 written;
        int total;

        while (true) {
            read = in_zf.read(buf, buf_sz);
            if (read == 0) {
                break;
            }
            if (read == -1) {
                qWarning() << "Error occured while reading archive";
                return false;
            }
            // write by portions
            total = 0;
            while (true) {
                written = out_zf.write(buf + total, read - total);
                total += written;
                if (total >= read) {
                    // we've written all needed data
                    break;
                }

            }
            if (written != read) {
                qWarning() << "written!=read" << read << written;
                return false;
            }
        }

        out_z.close();
        in_z.close();
        return true;
    }
}
