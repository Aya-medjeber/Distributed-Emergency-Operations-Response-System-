#pragma once

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class TcpClient
{
public:
    TcpClient();
    ~TcpClient();

    bool Initialize();
    bool Connect(const std::string& serverIp, int port);
    bool SendText(const std::string& message);
    void Disconnect();

private:
    SOCKET m_socket;
    bool m_initialized;
};