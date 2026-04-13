#include "TcpServer.h"
#include <iostream>

TcpServer::TcpServer()
    : m_listenSocket(INVALID_SOCKET),
    m_clientSocket(INVALID_SOCKET),
    m_initialized(false)
{
}

TcpServer::~TcpServer()
{
    Shutdown();
}

bool TcpServer::Initialize()
{
    WSADATA wsaData{};
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "[Server] WSAStartup failed.\n";
        return false;
    }

    m_initialized = true;
    return true;
}

bool TcpServer::StartListening(int port)
{
    if (!m_initialized)
        return false;

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        std::cout << "[Server] Socket creation failed.\n";
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    int bindResult = bind(m_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if (bindResult == SOCKET_ERROR)
    {
        std::cout << "[Server] Bind failed.\n";
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    int listenResult = listen(m_listenSocket, 1);
    if (listenResult == SOCKET_ERROR)
    {
        std::cout << "[Server] Listen failed.\n";
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
        return false;
    }

    std::cout << "[Server] Listening on port " << port << "...\n";
    return true;
}

bool TcpServer::AcceptClient()
{
    if (m_listenSocket == INVALID_SOCKET)
        return false;

    sockaddr_in clientAddr{};
    int clientSize = sizeof(clientAddr);

    m_clientSocket = accept(m_listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientSize);
    if (m_clientSocket == INVALID_SOCKET)
    {
        std::cout << "[Server] Accept failed.\n";
        return false;
    }

    std::cout << "[Server] Client connected.\n";
    return true;
}

bool TcpServer::ReceiveBytes(std::vector<char>& outData)
{
    if (m_clientSocket == INVALID_SOCKET)
        return false;

    char buffer[2048]{};
    int bytesReceived = recv(m_clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0)
    {
        std::cout << "[Server] Receive failed or client disconnected.\n";
        return false;
    }

    outData.assign(buffer, buffer + bytesReceived);
    return true;
}

bool TcpServer::SendBytes(const std::vector<char>& data)
{
    if (m_clientSocket == INVALID_SOCKET)
        return false;

    int result = send(m_clientSocket, data.data(), static_cast<int>(data.size()), 0);
    if (result == SOCKET_ERROR)
    {
        std::cout << "[Server] Send failed.\n";
        return false;
    }

    return true;
}

void TcpServer::Shutdown()
{
    if (m_clientSocket != INVALID_SOCKET)
    {
        closesocket(m_clientSocket);
        m_clientSocket = INVALID_SOCKET;
    }

    if (m_listenSocket != INVALID_SOCKET)
    {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }

    if (m_initialized)
    {
        WSACleanup();
        m_initialized = false;
    }
}