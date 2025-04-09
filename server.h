#ifndef SERVER_H
#define SERVER_H
#include <QString>

class Server
{
public:
    Server();
    void sendMessageToServer(const QString &message);
    void setIP(std::string);
    void setPort(int);

private:
    std::string ip;
    int port;
};
#endif // SERVER_H
