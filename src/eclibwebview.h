/*
 * eclibwebview.h
 *
 * Created on: Jan 11, 2012
 * Author: Sergey Stolyarov
 */

#ifndef _ECLIBWEBVIEW_H_
#define _ECLIBWEBVIEW_H_

#include <QWebView>

class QContextMenuEvent;
class QMouseEvent;

class EclibWebView : public QWebView
{
    Q_OBJECT
public:
    EclibWebView(QWidget * parent = 0);
    ~EclibWebView();

signals:
    void bookDownloadRequested(int bookId);
    void linkMiddleClicked(const QUrl & url);

protected slots:
    void downloadBook();

protected:
    void contextMenuEvent(QContextMenuEvent * ev);
    void mousePressEvent(QMouseEvent * ev);
    void mouseReleaseEvent(QMouseEvent * ev);

private:
    struct Private;
    Private * p;
};

#endif // _ECLIBWEBVIEW_H_
