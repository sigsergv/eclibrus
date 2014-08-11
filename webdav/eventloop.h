#include <QEventLoop>
#include <QNetworkReply>

class EventLoop : public QEventLoop
{
    Q_OBJECT
public:
    enum Status {NoStatus, StatusTimeout, StatusMetadata};

    EventLoop(QObject * parent = 0);
    ~EventLoop();

    Status status();

private:
    struct Private;
    Private * p;

public slots:
    void quitMetadata();
    void quitTimeout();
};
