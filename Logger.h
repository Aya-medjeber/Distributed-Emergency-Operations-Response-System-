#pragma once

#include <string>

class Logger
{
public:
    explicit Logger(const std::string& fileName);

    void Log(const std::string& direction,
        const std::string& messageType,
        int payloadSize,
        const std::string& result);

private:
    std::string m_fileName;
};