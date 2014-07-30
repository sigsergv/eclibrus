/*
 * mountnotifier.cpp
 *
 * Created on: Jan 08, 2012
 * Author: Sergei Stolyarov
 */

#include <QtCore>
#include <QtDebug>

#include "mountnotifier.h"
#include "devices.h"

struct MountNotifier::Private
{
    QFileSystemWatcher * watcher;
};

MountNotifier::MountNotifier(QObject * parent)
    : QObject(parent)
{
    p = new Private;
    p->watcher = new QFileSystemWatcher(this);
    p->watcher->addPath("/etc/mtab");

    connect(p->watcher, SIGNAL(fileChanged(const QString &)),
        this, SLOT(mountsChanged(const QString &)));
}


MountNotifier::~MountNotifier()
{
    delete p;
}

void MountNotifier::mountsChanged(const QString &)
{
    // force read list of devices
    Eclibrus::connectedRemovableDevices(true);
}
