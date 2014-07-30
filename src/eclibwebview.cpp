/*
 * eclibwebview.cpp
 *
 * Created on: Jan 11, 2012
 * Author: Sergei Stolyarov
 */

#include <QtWebKit>
#include <QWebFrame>
#include <QtWidgets>
#include <QtDebug>

#include "eclibwebview.h"

struct EclibWebView::Private
{
    QMenu bookLinkContextMenu;
    int bookLinkBookId;

    QPoint mousePressEventPos;
};


EclibWebView::EclibWebView(QWidget * parent)
    : QWebView(parent)
{
    p = new Private();
    QAction * downloadBookAction = p->bookLinkContextMenu.addAction(tr("Download book"));
    p->bookLinkBookId = 0;
    setStyleSheet("QToolTip {\
        color: black; background-color: yellow;\
        min-width: 300px; padding: 5px;\
        border: 1px dotted red;}");

    connect(downloadBookAction, SIGNAL(triggered()),
        this, SLOT(downloadBook()));
}

EclibWebView::~EclibWebView()
{
    delete p;
}

void EclibWebView::downloadBook()
{
    if (p->bookLinkBookId) {
        emit bookDownloadRequested(p->bookLinkBookId);
    }
    p->bookLinkBookId = 0;
}

void EclibWebView::contextMenuEvent(QContextMenuEvent * ev)
{
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(ev->pos());
    QUrl linkUrl = r.linkUrl();
    QUrlQuery linkUrlQuery(linkUrl);
    QMenu * menu = 0;

    if (!linkUrl.isEmpty()) {
        if (linkUrl.scheme() == "eclib" && linkUrl.path() == "book") {
            int id = linkUrlQuery.queryItemValue("id").toInt();
            if (id > 0) {
                p->bookLinkBookId = id;
                menu = &(p->bookLinkContextMenu);
            }
        }
    }

    if (menu != 0) {
        menu->exec(mapToGlobal(ev->pos()));
    }
}

void EclibWebView::mousePressEvent(QMouseEvent * ev)
{
    p->mousePressEventPos = ev->pos();
    QWebView::mousePressEvent(ev);
}

void EclibWebView::mouseReleaseEvent(QMouseEvent * ev)
{
    if (ev->pos() == p->mousePressEventPos) {
        // the same spot so process
        if (ev->button() == Qt::MiddleButton) {
            // middle button is processed separately
            QWebHitTestResult r = page()->mainFrame()->hitTestContent(ev->pos());
            QUrl linkUrl = r.linkUrl();
            if (!linkUrl.isEmpty()) {
                emit linkMiddleClicked(linkUrl);
            }
            return;
        }
    }
    QWebView::mouseReleaseEvent(ev);
}

