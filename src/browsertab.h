/*  
 * browsertab.h
 *  
 * Created on: Dec 28, 2011
 * Author: Sergei Stolyarov
 */ 
#ifndef BROWSERTAB_H_
#define BROWSERTAB_H_

#include <QWidget>

namespace Eclibrus
{
    struct DeviceInfo;
}
class QUrl;

class BrowserTab : public QWidget
{
    Q_OBJECT
public:
    BrowserTab(QWidget * parent = 0);
    ~BrowserTab();
    void exportBookToDevice(const Eclibrus::DeviceInfo & di, int bookId);
    void exportAllBooksFromPageToDevice(const Eclibrus::DeviceInfo & di);
    void setUrl(const QUrl & url);

private:
    struct Private;
    Private * p;

protected slots:
    void linkClicked(const QUrl & url);
    void linkMiddleClicked(const QUrl & url);
    void downloadBook(int bookId); 
    void startSearch();
};
#endif // BROWSERTAB_H_

