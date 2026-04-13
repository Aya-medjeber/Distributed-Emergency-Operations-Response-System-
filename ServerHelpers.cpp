#include "ServerHelpers.h"

CommandType ParseCommandFromTextHelper(const std::string& text)
{
    if (text == "DECLARE_ALERT") return CommandType::DECLARE_ALERT;
    if (text == "ESCALATE_ALERT") return CommandType::ESCALATE_ALERT;
    if (text == "RESOLVE_ALERT") return CommandType::RESOLVE_ALERT;
    if (text == "RESET_SYSTEM") return CommandType::RESET_SYSTEM;
    if (text == "REQUEST_SITUATION_REPORT") return CommandType::REQUEST_SITUATION_REPORT;
    return CommandType::ERROR_RESPONSE;
}

bool IsValidCredentialHelper(const std::string& credentials)
{
    return credentials == "aya:1234";
}