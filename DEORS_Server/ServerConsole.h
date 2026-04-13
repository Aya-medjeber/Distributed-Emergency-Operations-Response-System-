#pragma once
#include "ServerState.h"

class ServerConsole
{
public:
    void ShowStartup() const;
    void ShowState(ServerState state) const;
    void ShowMessage(const char* message) const;
};