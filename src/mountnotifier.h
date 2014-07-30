/*
 * mountnotifier.h
 *
 * Created on: Jan 08, 2012
 * Author: Sergei Stolyarov
 */

#ifndef _MOUNTNOTIFIER_H_
#define _MOUNTNOTIFIER_H_

#include <QObject>

class MountNotifier : public QObject
{
    Q_OBJECT
public:
    MountNotifier(QObject * parent = 0);
    ~MountNotifier();

protected slots:
    void mountsChanged(const QString &);

private:
    struct Private;
    Private * p;
};

#endif // _MOUNTNOTIFIER_H_
