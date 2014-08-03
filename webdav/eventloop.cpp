#include <QtDebug>

#include "eventloop.h"

struct EventLoop::Private
{
    EventLoop::Status status;
};

EventLoop::EventLoop(QObject * parent)
    :QEventLoop(parent)
{
    p = new Private();
    p->status = NoStatus;
}

EventLoop::~EventLoop()
{
    delete p;
}

EventLoop::Status EventLoop::status()
{
    return p->status;
}

void EventLoop::quitMetadata()
{
    p->status = StatusMetadata;
    qDebug() << "QUIT by metadata";
    QEventLoop::quit();
}

void EventLoop::quitTimeout()
{
    p->status = StatusTimeout;
    qDebug() << "QUIT by timeout";
    QEventLoop::quit();
}