#include <QStringList>
#include "vpclient.h"

VPClient::VPClient(QObject *parent) :
    QObject(parent)
{
    this->socket = new QTcpSocket(this);
    QObject::connect(this->socket, SIGNAL(connected()), this,
                     SLOT(socket_connected()));
    QObject::connect(this->socket, SIGNAL(disconnected()), this,
                     SLOT(socket_disconnected()));
    QObject::connect(this->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(socket_error(QAbstractSocket::SocketError)));
    QObject::connect(this->socket, SIGNAL(readyRead()), this,
                     SLOT(socket_dataReceived()));
    this->text = new QString("");
    this->highlightStart = 0;
    this->highlightEnd = 0;
    this->read_buff = new QString("");
    this->text_size = 0;
    this->reading = new QMutex();
    this->state = Disconnected;
}

VPClient::~VPClient()
{
    emit this->disconnectedFromHost();
    delete this->socket;
    delete this->text;
    delete this->reading;
    delete this->read_buff;
}

void VPClient::connectToHost(const QString host, quint16 port)
{
    if (this->state != Disconnected) {
        socket->disconnectFromHost();
    }
    this->state = Connecting;
    this->socket->connectToHost(host, port);
}

void VPClient::socket_connected()
{
    this->socket->write("LISTEN");
}

void VPClient::socket_error(QAbstractSocket::SocketError error)
{
    this->disconnectedFromHost();
}

const bool VPClient::isConnected()
{
    return this->state == Connected;
}

void VPClient::socket_dataReceived()
{
    this->reading->lock();
    while (this->socket->bytesAvailable()) {
        if (this->text_size > 0) {
            QByteArray read = this->socket->read(this->text_size);
            this->read_buff->append(QString::fromUtf8(read));
            this->text_size -= read.size();
            Q_ASSERT(this->text_size >= 0);
            if (this->text_size == 0) {
                *(this->text) = *(this->read_buff);
                this->read_buff->clear();;
                this->socket->readLine();
                emit this->textChanged();
            }
            continue;
        }
        this->read_buff->append(this->socket->readLine());
        if (this->read_buff->endsWith("\n")) {
            this->read_buff->chop(1);
        }
        else {
            continue;
        }
        if (this->state == Connecting) {
            if (*(this->read_buff) == "OK") {
                this->state = Connected;
                emit this->connectedToHost();
            } else {
                this->disconnectFromHost();
            }
        }
        else {
            QStringList args = this->read_buff->split(' ');
            QString arg = args.at(0);
            if (arg == "TEXT") {
                arg = args.at(1);
                if (arg == "DATA") {
                    bool convert_ok;
                    int length = args.at(2).toInt(&convert_ok);
                    if (!convert_ok) {
                        continue;
                    }
                    QByteArray read = socket->read(length);
                    *(this->read_buff) = QString::fromUtf8(read);
                    if (read.size() < length) {
                        text_size = length - read.size();
                        continue;
                    }
                    else
                    {
                        *(this->text) = *(this->read_buff);
                        this->read_buff->clear();
                        this->socket->readLine();
                        emit this->textChanged();
                    }
                } if (arg == "HIGHLIGHT") {
                    bool convert_ok1, convert_ok2;
                    int start = args.at(2).toInt(&convert_ok1);
                    int end = args.at(3).toInt(&convert_ok2);
                    if (!convert_ok1 || !convert_ok2) {
                        continue;
                    }
                    this->highlightStart = start;
                    this->highlightEnd = end;
                    emit this->highlightChanged();
                }
            }
        }
        this->read_buff->clear();
    }
    this->reading->unlock();
}

void VPClient::socket_disconnected()
{
    this->text->clear();
    this->read_buff->clear();
    this->text_size = 0;
    this->highlightStart = 0;
    this->highlightEnd = 0;
    this->state = Disconnected;
    emit this->disconnectedFromHost();
}

const QString VPClient::getText()
{
    return *(this->text);
}

const int VPClient::getHighlightStart()
{
    return this->highlightStart;
}

const int VPClient::getHighlightEnd()
{
    return this->highlightEnd;
}

void VPClient::disconnectFromHost()
{
    this->socket->disconnectFromHost();
}
