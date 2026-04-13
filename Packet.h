#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "CommandType.h"

class Packet
{
public:
    Packet();
    Packet(CommandType type, int sessionId, const std::vector<char>& payloadData);

    int32_t magic;
    CommandType commandType;
    int32_t payloadSize;
    int32_t sessionId;
    int32_t sequenceNumber;
    std::vector<char> payload;
    int32_t checksum;

    std::vector<char> Serialize() const;
    static bool Deserialize(const std::vector<char>& raw, Packet& outPacket);

    static std::vector<char> StringToPayload(const std::string& text);
    static std::string PayloadToString(const std::vector<char>& payload);

private:
    static void AppendInt(std::vector<char>& buffer, int32_t value);
    static bool ReadInt(const std::vector<char>& buffer, size_t& offset, int32_t& value);
    static int32_t ComputeChecksum(const std::vector<char>& data);
};