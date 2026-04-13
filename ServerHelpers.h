#pragma once
#include <string>
#include "CommandType.h"

CommandType ParseCommandFromTextHelper(const std::string& text);
bool IsValidCredentialHelper(const std::string& credentials);