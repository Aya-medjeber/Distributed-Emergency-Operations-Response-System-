#pragma once

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    bool Initialize();
    bool StartListening(int port);
    bool AcceptClient();
    bool ReceiveBytes(std::vector<char>& outData);
    bool SendBytes(const std::vector<char>& data);
    void Shutdown();

private:
    bool SendAll(const char* data, int totalBytes);
    bool ReceiveAll(char* buffer, int totalBytes);

private:
    SOCKET m_listenSocket;
    SOCKET m_clientSocket;
    bool m_initialized;
};