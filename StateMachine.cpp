#include "StateMachine.h"
#include <string>

StateMachine::StateMachine()
    : m_currentState(ServerState::NORMAL)
{
}

ServerState StateMachine::GetCurrentState() const
{
    return m_currentState;
}

bool StateMachine::CanExecute(CommandType command) const
{
    switch (m_currentState)
    {
    case ServerState::NORMAL:
        return command == CommandType::DECLARE_ALERT ||
            command == CommandType::REQUEST_SITUATION_REPORT;

    case ServerState::ALERT_ACTIVE:
        return command == CommandType::ESCALATE_ALERT ||
            command == CommandType::RESOLVE_ALERT ||
            command == CommandType::REQUEST_SITUATION_REPORT;

    case ServerState::ESCALATED:
        return command == CommandType::RESOLVE_ALERT ||
            command == CommandType::REQUEST_SITUATION_REPORT;

    case ServerState::RESOLVED:
        return command == CommandType::RESET_SYSTEM ||
            command == CommandType::REQUEST_SITUATION_REPORT;
    }

    return false;
}

bool StateMachine::Apply(CommandType command)
{
    if (!CanExecute(command))
        return false;

    switch (m_currentState)
    {
    case ServerState::NORMAL:
        if (command == CommandType::DECLARE_ALERT)
            m_currentState = ServerState::ALERT_ACTIVE;
        break;

    case ServerState::ALERT_ACTIVE:
        if (command == CommandType::ESCALATE_ALERT)
            m_currentState = ServerState::ESCALATED;
        else if (command == CommandType::RESOLVE_ALERT)
            m_currentState = ServerState::RESOLVED;
        break;

    case ServerState::ESCALATED:
        if (command == CommandType::RESOLVE_ALERT)
            m_currentState = ServerState::RESOLVED;
        break;

    case ServerState::RESOLVED:
        if (command == CommandType::RESET_SYSTEM)
            m_currentState = ServerState::NORMAL;
        break;
    }

    return true;
}

std::string StateMachine::GetStateAsString() const
{
    switch (m_currentState)
    {
    case ServerState::NORMAL:
        return "NORMAL";
    case ServerState::ALERT_ACTIVE:
        return "ALERT_ACTIVE";
    case ServerState::ESCALATED:
        return "ESCALATED";
    case ServerState::RESOLVED:
        return "RESOLVED";
    default:
        return "UNKNOWN";
    }
}