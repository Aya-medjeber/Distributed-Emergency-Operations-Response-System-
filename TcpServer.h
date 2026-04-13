#pragma once

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    bool Initialize();
    bool StartListening(int port);
    bool AcceptClient();
    bool ReceiveText(std::string& outMessage);
    void Shutdown();

private:
    SOCKET m_listenSocket;
    SOCKET m_clientSocket;
    bool m_initialized;
};