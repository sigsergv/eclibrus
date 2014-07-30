/*
 * exportbooksprogress.h
 *
 * Created on: Jan 09, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _EXPORTBOOKSPROGRESS_H_
#define _EXPORTBOOKSPROGRESS_H_

#include <QDialog>
#include <QList>
#include "fb2/fb2.h"

class ExportBooksProgress : public QDialog
{
    Q_OBJECT
public:
    ExportBooksProgress(const QList<int> books, const QString & outputDir, QWidget * parent = 0);
    ~ExportBooksProgress();
    void setVisible(bool visible);

protected slots:
    void progressChanged(int);

private:
    struct Private;
    Private * p;
};

#endif // _EXPORTBOOKSPROGRESS_H_
