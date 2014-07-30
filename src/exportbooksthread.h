/*
 * exportbooksthread.h
 *
 * Created on: Jan 09, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _EXPORTBOOKSTHREAD_H_
#define _EXPORTBOOKSTHREAD_H_

#include <QThread>
#include <QList>

#include "fb2/fb2.h"

class ExportThread : public QThread
{
    Q_OBJECT
public:
    ExportThread(const QList<int> & books, const QString & outputDir);
    ~ExportThread();
    void run();

signals:
    void progress(int percent);

private:
    struct Private;
    Private * p;
};


#endif // _EXPORTBOOKSTHREAD_H_
