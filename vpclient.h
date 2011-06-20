#ifndef VPCLIENT_H
#define VPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QString>
#include <QMutex>

class VPClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText)
    Q_PROPERTY(int highlightStart READ getHighlightStart)
    Q_PROPERTY(int highlightEnd READ getHighlightEnd)

private:
    enum VPClientState {
        Disconnected, Connecting, Connected
    };

    QTcpSocket *socket;
    VPClientState state;
    QString *text;
    QString *read_buff;
    int text_size;
    QMutex* reading;
    int highlightStart, highlightEnd;

public:
    explicit VPClient(QObject *parent = 0);
    ~VPClient();
    void connectToHost(const QString host, quint16 port);
    void disconnectFromHost();
    const bool isConnected();
    const QString getText();
    const int getHighlightStart();
    const int getHighlightEnd();

signals:
    void connectedToHost();
    void disconnectedFromHost();
    void textChanged();
    void highlightChanged();

private slots:
    void socket_connected();
    void socket_dataReceived();
    void socket_disconnected();
    void socket_error(QAbstractSocket::SocketError error);
};

#endif // VPCLIENT_H
