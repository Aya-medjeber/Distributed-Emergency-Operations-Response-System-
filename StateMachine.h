#pragma once
#include "ServerState.h"
#include "CommandType.h"
#include <string>

class StateMachine
{
public:
    StateMachine();

    ServerState GetCurrentState() const;
    bool CanExecute(CommandType command) const;
    bool Apply(CommandType command);
    std::string GetStateAsString() const;

private:
    ServerState m_currentState;
};