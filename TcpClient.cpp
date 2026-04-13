#include "TcpClient.h"
#include <iostream>

TcpClient::TcpClient()
    : m_socket(INVALID_SOCKET), m_initialized(false)
{
}

TcpClient::~TcpClient()
{
    Disconnect();
}

bool TcpClient::Initialize()
{
    WSADATA wsaData{};
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "[Client] WSAStartup failed.\n";
        return false;
    }

    m_initialized = true;
    return true;
}

bool TcpClient::Connect(const std::string& serverIp, int port)
{
    if (!m_initialized)
        return false;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        std::cout << "[Client] Socket creation failed.\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0)
    {
        std::cout << "[Client] Invalid server IP address.\n";
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    int result = connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (result == SOCKET_ERROR)
    {
        std::cout << "[Client] Connection failed.\n";
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    std::cout << "[Client] Connected to server.\n";
    return true;
}

bool TcpClient::SendText(const std::string& message)
{
    if (m_socket == INVALID_SOCKET)
        return false;

    int result = send(m_socket, message.c_str(), static_cast<int>(message.size()), 0);
    if (result == SOCKET_ERROR)
    {
        std::cout << "[Client] Send failed.\n";
        return false;
    }

    std::cout << "[Client] Sent message: " << message << "\n";
    return true;
}

void TcpClient::Disconnect()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    if (m_initialized)
    {
        WSACleanup();
        m_initialized = false;
    }
}