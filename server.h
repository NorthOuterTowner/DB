#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <QString>
class QTcpSocket;

class Server {
private:
    static Server* instance;  // Static instance member
    std::string ip = "127.0.0.1";
    int port = 4000;
    QTcpSocket* socket;
    Server();

public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    ~Server();
    static Server* getInstance();
    void setIP(std::string ip);
    void setPort(int port);
    void sendMessageToServer(const QString &message);
    void connectToServer();
    void disconnectToServer();
    void sendMessage(const QString &message);
};

#endif // SERVER_H
