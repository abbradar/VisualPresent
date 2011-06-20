#ifndef VPSERVER_H
#define VPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QList>

class VPServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText)
    Q_PROPERTY(int highlightStart READ getHighlightStart)
    Q_PROPERTY(int highlightEnd READ getHighlightEnd)

private:
    QTcpServer *server;
    QList<QTcpSocket*> *clients;
    QList<QTcpSocket*> *pendingClients;
    QString *text;
    int highlightStart, highlightEnd;
    bool listening;

public:
    explicit VPServer(QObject *parent = 0);
    ~VPServer();
    const bool isListening();
    bool start(quint16 port);
    bool stop();
    const QString getText();
    const int getHighlightStart();
    const int getHighlightEnd();

private slots:
    void server_newConnection();
    void pending_dataReceived();
    void pending_disconnected();
    void client_disconnected();

public slots:
    void setText(QString value);
    void setHighlight(int start, int end);
};

#endif // VPSERVER_H
