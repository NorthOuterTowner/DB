#include "server.h"
#include <QString>
#include <QTcpSocket>
#include <QDebug>
#include <iostream>
// Initialize static member
Server* Server::instance = nullptr;

Server::Server() {

}

Server::~Server() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}

Server* Server::getInstance() {
    if (instance == nullptr) {
        instance = new Server();
    }
    return instance;
}

void Server::setIP(std::string ip) {
    this->ip = ip;
}

void Server::setPort(int port) {
    this->port = port;
}

void Server::sendMessageToServer(const QString &message) {
    socket = new QTcpSocket();
    const QString & ip = QString::fromStdString(this->ip);
    socket->connectToHost(ip, port);
    if (socket->waitForConnected(1000)) {
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

void Server::connectToServer(){
    socket = new QTcpSocket();
    const QString & ip = QString::fromStdString(this->ip);
    socket->connectToHost(ip,port);
    if(!socket->waitForConnected(1000)){
        qDebug()<<"Connection failed"<<socket->errorString();
    }
}

void Server::disconnectToServer(){
    socket->disconnectFromHost();
    socket->deleteLater();
}

void Server::sendMessage(const QString &message)
{
    QByteArray data = message.toUtf8();
    socket->write(data);
    socket->waitForBytesWritten(1000);
    if (socket->waitForReadyRead(3000)) {
        QByteArray response = socket->readAll();
        qDebug() << "Server response:" << response;
    }
}

