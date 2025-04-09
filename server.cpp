#include "server.h"
#include <QString>
#include <QTcpSocket>
#include <QDebug>
Server::Server() {}

void Server::setIP(std::string ip){
        this->ip = ip;
}

void Server::setPort(int port){
    this->port = port;
};

void Server::sendMessageToServer(const QString &message) {
    QTcpSocket *socket = new QTcpSocket();
    const QString & ip = QString::fromStdString(this->ip);
    socket->connectToHost(ip, port);
    if (socket->waitForConnected(3000)) {
        QByteArray data = message.toUtf8();
        socket->write(data);
        socket->waitForBytesWritten(1000);
        if (socket->waitForReadyRead(3000)) {
            QByteArray response = socket->readAll();
            qDebug() << "Server response:" << response;
        }
    } else {
        qDebug() << "Connection failed:" << socket->errorString();
    }
    socket->disconnectFromHost();
    socket->deleteLater();
}
