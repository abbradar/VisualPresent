#include <QHostAddress>
#include <QtAlgorithms>
#include "vpserver.h"

VPServer::VPServer(QObject *parent) :
    QObject(parent)
{
    this->listening = false;
    this->server = new QTcpServer(this);
    QObject::connect(this->server, SIGNAL(newConnection()),
                     this, SLOT(server_newConnection()));
    this->clients = new QList<QTcpSocket*>();
    this->pendingClients = new QList<QTcpSocket*>();
    this->text = new QString("");
    this->highlightStart = 0;
    this->highlightEnd = 0;
}

VPServer::~VPServer()
{
    // i *really* don't know how to do server disposal in other way
    // w/out SEGFAULTs and/or memory leaks.
    this->server->close();
    for (QList<QTcpSocket*>::iterator i = this->clients->begin(); i !=
                                this->clients->end(); i++) {
        (*i)->disconnectFromHost();
    }
    delete this->clients;
    for (QList<QTcpSocket*>::iterator i = this->pendingClients->begin(); i !=
                                this->pendingClients->end(); i++) {
        (*i)->disconnectFromHost();
    }
    delete this->pendingClients;
    delete this->server;
    delete this->text;
}

bool VPServer::start(quint16 port)
{
    if (this->listening) {
        this->stop();
    }
    this->listening = server->listen(QHostAddress::Any, port);
    return this->listening;
}

bool VPServer::stop()
{
    this->listening = false;
    this->server->close();
    for (QList<QTcpSocket*>::iterator i = this->clients->begin(); i !=
                                this->clients->end(); i++) {
        (*i)->disconnectFromHost();
    }
    this->clients->clear();
    for (QList<QTcpSocket*>::iterator i = this->pendingClients->begin(); i !=
                                this->pendingClients->end(); i++) {
        (*i)->disconnectFromHost();
    }
    this->pendingClients->clear();
    return false;
}

void VPServer::server_newConnection()
{
    QTcpSocket *socket = this->server->nextPendingConnection();
    QObject::connect(socket, SIGNAL(readyRead()), this,
                     SLOT(pending_dataReceived()));
    pendingClients->push_back(socket);
    QObject::connect(socket, SIGNAL(disconnected()), this,
                     SLOT(pending_disconnected()));
}

void VPServer::pending_dataReceived()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(QObject::sender());
    Q_ASSERT(this->pendingClients->removeOne(socket));
    QObject::disconnect(socket, SIGNAL(disconnected()),
                        this, SLOT(pending_disconnected()));
    QString line = QString(socket->readLine());
    if (line == "LISTEN") {
        this->clients->push_back(socket);
        QObject::connect(socket, SIGNAL(disconnected()),
                         this, SLOT(client_disconnected()));
        QObject::disconnect(socket, SIGNAL(readyRead()),
                            this, SLOT(pending_dataReceived()));
        socket->write("OK\n");
        QByteArray coded = this->text->toUtf8();
        socket->write("TEXT DATA " + QString::number(coded.length())
                              .toAscii() + "\n" + coded + "\n");
        socket->write("TEXT HIGHLIGHT " +
                      QString::number(this->highlightStart).toAscii() + " " +
                      QString::number(this->highlightEnd).toAscii() + "\n");
    } else {
        delete socket;
    }
}

void VPServer::client_disconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(QObject::sender());
    Q_ASSERT(this->clients->removeOne(socket));
}

void VPServer::pending_disconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(QObject::sender());
    Q_ASSERT(this->pendingClients->removeOne(socket));
}

const bool VPServer::isListening()
{
    return this->listening;
}

const QString VPServer::getText()
{
    return *(this->text);
}

void VPServer::setText(const QString text)
{
    *(this->text) = text;
   if (this->listening) {
        QByteArray coded = this->text->toUtf8();
        for (QList<QTcpSocket*>::iterator i = this->clients->begin();
             i != this->clients->end(); i++) {
             (*i)->write("TEXT DATA " + QString::number(coded.length())
                         .toAscii() + "\n" + coded + "\n");
        }
    }
}

const int VPServer::getHighlightStart()
{
    return this->highlightStart;
}

const int VPServer::getHighlightEnd()
{
    return this->highlightEnd;
}

void VPServer::setHighlight(int start, int end)
{
    this->highlightStart = start;
    this->highlightEnd = end;
    if (this->listening) {
        for (QList<QTcpSocket*>::iterator i = this->clients->begin();
             i != this->clients->end(); i++) {
             (*i)->write("TEXT HIGHLIGHT " +
                         QString::number(this->highlightStart).toAscii()
                         + " " + QString::number(this->highlightEnd)
                         .toAscii() + "\n");
        }
    }
}
