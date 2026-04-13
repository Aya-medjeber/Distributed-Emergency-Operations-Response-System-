#include "Packet.h"
#include "Constants.h"
#include <cstring>

Packet::Packet()
    : magic(Constants::HeaderMagic),
    commandType(CommandType::LOGIN_REQUEST),
    payloadSize(0),
    sessionId(0),
    sequenceNumber(0),
    checksum(0)
{
}

Packet::Packet(CommandType type, int sessionIdValue, const std::vector<char>& payloadData)
    : magic(Constants::HeaderMagic),
    commandType(type),
    payloadSize(static_cast<int32_t>(payloadData.size())),
    sessionId(sessionIdValue),
    sequenceNumber(0),
    payload(payloadData),
    checksum(ComputeChecksum(payloadData))
{
}

void Packet::AppendInt(std::vector<char>& buffer, int32_t value)
{
    const char* bytes = reinterpret_cast<const char*>(&value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(int32_t));
}

bool Packet::ReadInt(const std::vector<char>& buffer, size_t& offset, int32_t& value)
{
    if (offset + sizeof(int32_t) > buffer.size())
        return false;

    std::memcpy(&value, buffer.data() + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    return true;
}

int32_t Packet::ComputeChecksum(const std::vector<char>& data)
{
    int32_t sum = 0;
    for (char byte : data)
    {
        sum += static_cast<unsigned char>(byte);
    }
    return sum;
}

std::vector<char> Packet::Serialize() const
{
    std::vector<char> raw;
    AppendInt(raw, magic);
    AppendInt(raw, static_cast<int32_t>(commandType));
    AppendInt(raw, payloadSize);
    AppendInt(raw, sessionId);
    AppendInt(raw, sequenceNumber);
    AppendInt(raw, checksum);

    raw.insert(raw.end(), payload.begin(), payload.end());
    return raw;
}

bool Packet::Deserialize(const std::vector<char>& raw, Packet& outPacket)
{
    size_t offset = 0;
    int32_t cmd = 0;

    if (!ReadInt(raw, offset, outPacket.magic)) return false;
    if (!ReadInt(raw, offset, cmd)) return false;
    if (!ReadInt(raw, offset, outPacket.payloadSize)) return false;
    if (!ReadInt(raw, offset, outPacket.sessionId)) return false;
    if (!ReadInt(raw, offset, outPacket.sequenceNumber)) return false;
    if (!ReadInt(raw, offset, outPacket.checksum)) return false;

    outPacket.commandType = static_cast<CommandType>(cmd);

    if (offset + static_cast<size_t>(outPacket.payloadSize) > raw.size())
        return false;

    outPacket.payload.assign(raw.begin() + offset, raw.begin() + offset + outPacket.payloadSize);

    return outPacket.checksum == ComputeChecksum(outPacket.payload);
}

std::vector<char> Packet::StringToPayload(const std::string& text)
{
    return std::vector<char>(text.begin(), text.end());
}

std::string Packet::PayloadToString(const std::vector<char>& payload)
{
    return std::string(payload.begin(), payload.end());
}