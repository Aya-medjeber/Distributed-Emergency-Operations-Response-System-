#include "Logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>

Logger::Logger(const std::string& fileName)
    : m_fileName(fileName)
{
}

void Logger::Log(const std::string& direction,
    const std::string& messageType,
    int payloadSize,
    const std::string& result)
{
    std::ofstream file(m_fileName, std::ios::app);
    if (!file.is_open())
        return;

    auto now = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(now);

    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    file << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
        << " | " << direction
        << " | " << messageType
        << " | " << payloadSize << " bytes"
        << " | " << result
        << "\n";
}