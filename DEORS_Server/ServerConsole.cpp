#include "ServerConsole.h"
#include <iostream>

void ServerConsole::ShowStartup() const
{
    std::cout << "=========================================\n";
    std::cout << " DEORS Server - Emergency Control Center\n";
    std::cout << "=========================================\n";
}

void ServerConsole::ShowState(ServerState state) const
{
    std::cout << "[Server] Current State: ";

    switch (state)
    {
    case ServerState::NORMAL:
        std::cout << "NORMAL";
        break;
    case ServerState::ALERT_ACTIVE:
        std::cout << "ALERT_ACTIVE";
        break;
    case ServerState::ESCALATED:
        std::cout << "ESCALATED";
        break;
    case ServerState::RESOLVED:
        std::cout << "RESOLVED";
        break;
    }

    std::cout << "\n";
}

void ServerConsole::ShowMessage(const char* message) const
{
    std::cout << "[Server] " << message << "\n";
}