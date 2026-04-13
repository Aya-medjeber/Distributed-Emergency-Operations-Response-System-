#include "ServerConsole.h"
#include "StateMachine.h"
#include "TcpServer.h"
#include "Packet.h"
#include "Logger.h"
#include "ServerHelpers.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

static bool LoadBinaryFile(const std::filesystem::path& path, std::vector<char>& outData)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size <= 0)
        return false;

    outData.resize(static_cast<size_t>(size));
    file.read(outData.data(), size);

    return true;
}

static bool TryLoadReport(std::vector<char>& imageData, std::filesystem::path& foundPath)
{
    std::vector<std::filesystem::path> candidates =
    {
        std::filesystem::current_path() / "assets" / "situation_report.jpg",
        std::filesystem::current_path() / "situation_report.jpg",
        std::filesystem::current_path() / ".." / "assets" / "situation_report.jpg",
        std::filesystem::current_path() / ".." / "DEORS_Server" / "assets" / "situation_report.jpg",
        std::filesystem::path("C:/Users/Aya Ben Medjeber/Desktop/DEORS/DEORS_Server/assets/situation_report.jpg"),
        std::filesystem::path("C:/Users/Aya Ben Medjeber/Desktop/DEORS/DEORS_Server/situation_report.jpg"),
        std::filesystem::path("C:/Users/Aya Ben Medjeber/Desktop/DEORS/x64/Debug/assets/situation_report.jpg")
    };

    std::cout << "[Server] Current working directory: "
        << std::filesystem::current_path().string() << "\n";

    for (const auto& candidate : candidates)
    {
        std::cout << "[Server] Checking: " << candidate.string() << "\n";

        if (std::filesystem::exists(candidate) && LoadBinaryFile(candidate, imageData))
        {
            foundPath = candidate;
            return true;
        }
    }

    return false;
}

int main()
{
    ServerConsole console;
    StateMachine stateMachine;
    TcpServer server;
    Logger logger("server_log.txt");

    console.ShowStartup();
    console.ShowState(stateMachine.GetCurrentState());

    if (!server.Initialize())
    {
        console.ShowMessage("Network initialization failed.");
        return 1;
    }

    if (!server.StartListening(54000))
    {
        console.ShowMessage("Server failed to start listening.");
        return 1;
    }

    console.ShowMessage("Server started.");
    console.ShowMessage("Waiting for client connection...");

    if (!server.AcceptClient())
    {
        console.ShowMessage("No client connection accepted.");
        return 1;
    }

    bool authenticated = false;

    while (true)
    {
        std::vector<char> rawData;
        if (!server.ReceiveBytes(rawData))
        {
            console.ShowMessage("Client disconnected or receive failed.");
            break;
        }

        Packet receivedPacket;
        if (!Packet::Deserialize(rawData, receivedPacket))
        {
            console.ShowMessage("Failed to deserialize packet.");
            logger.Log("RX", "UNKNOWN_PACKET", static_cast<int>(rawData.size()), "FAIL");
            continue;
        }

        if (receivedPacket.commandType == CommandType::LOGIN_REQUEST)
        {
            logger.Log("RX", "LOGIN_REQUEST", receivedPacket.payloadSize, "OK");

            std::string credentials = Packet::PayloadToString(receivedPacket.payload);
            std::cout << "[Server] Received credentials: " << credentials << "\n";

            bool loginSuccess = IsValidCredentialHelper(credentials);
            authenticated = loginSuccess;

            std::string responseText = loginSuccess ? "LOGIN_SUCCESS" : "LOGIN_FAILED";

            Packet responsePacket(
                CommandType::LOGIN_RESPONSE,
                loginSuccess ? 1 : 0,
                Packet::StringToPayload(responseText)
            );

            if (!server.SendBytes(responsePacket.Serialize()))
            {
                console.ShowMessage("Failed to send login response.");
                logger.Log("TX", "LOGIN_RESPONSE", responsePacket.payloadSize, "FAIL");
                continue;
            }

            logger.Log("TX", "LOGIN_RESPONSE", responsePacket.payloadSize, "OK");
            std::cout << "[Server] Login response sent: " << responseText << "\n";
            continue;
        }

        if (!authenticated)
        {
            std::string responseText = "ERROR: NOT_AUTHENTICATED";
            Packet responsePacket(
                CommandType::ERROR_RESPONSE,
                0,
                Packet::StringToPayload(responseText)
            );

            server.SendBytes(responsePacket.Serialize());
            logger.Log("TX", "ERROR_RESPONSE", responsePacket.payloadSize, "NOT_AUTHENTICATED");
            continue;
        }

        std::string commandText = Packet::PayloadToString(receivedPacket.payload);
        CommandType command = ParseCommandFromTextHelper(commandText);

        logger.Log("RX", commandText, receivedPacket.payloadSize, "OK");

        if (command == CommandType::ERROR_RESPONSE)
        {
            std::string responseText = "ERROR: UNKNOWN_COMMAND";
            Packet responsePacket(
                CommandType::ERROR_RESPONSE,
                0,
                Packet::StringToPayload(responseText)
            );

            server.SendBytes(responsePacket.Serialize());
            logger.Log("TX", "ERROR_RESPONSE", responsePacket.payloadSize, "UNKNOWN_COMMAND");
            continue;
        }

        if (command == CommandType::REQUEST_SITUATION_REPORT)
        {
            std::vector<char> imageData;
            std::filesystem::path foundPath;

            if (!TryLoadReport(imageData, foundPath))
            {
                std::cout << "[Server] Failed to load report file from all candidate locations.\n";

                std::string responseText = "ERROR: REPORT_FILE_NOT_FOUND";
                Packet responsePacket(
                    CommandType::ERROR_RESPONSE,
                    0,
                    Packet::StringToPayload(responseText)
                );

                server.SendBytes(responsePacket.Serialize());
                logger.Log("TX", "ERROR_RESPONSE", responsePacket.payloadSize, "REPORT_NOT_FOUND");
                continue;
            }

            std::cout << "[Server] Using report file: " << foundPath.string() << "\n";

            Packet reportPacket(
                CommandType::REPORT_DATA,
                1,
                imageData
            );

            if (!server.SendBytes(reportPacket.Serialize()))
            {
                logger.Log("TX", "REPORT_DATA", reportPacket.payloadSize, "FAIL");
                continue;
            }

            logger.Log("TX", "REPORT_DATA", reportPacket.payloadSize, "OK");
            std::cout << "[Server] Sent report image (" << reportPacket.payloadSize << " bytes)\n";
            continue;
        }

        if (!stateMachine.CanExecute(command))
        {
            std::string responseText = "ERROR: INVALID_STATE_TRANSITION";
            Packet responsePacket(
                CommandType::ERROR_RESPONSE,
                0,
                Packet::StringToPayload(responseText)
            );

            server.SendBytes(responsePacket.Serialize());
            logger.Log("TX", "ERROR_RESPONSE", responsePacket.payloadSize, "INVALID_STATE");
            continue;
        }

        stateMachine.Apply(command);
        console.ShowState(stateMachine.GetCurrentState());

        std::string responseText = "STATE:" + stateMachine.GetStateAsString();
        Packet responsePacket(
            CommandType::STATE_UPDATE,
            1,
            Packet::StringToPayload(responseText)
        );

        if (!server.SendBytes(responsePacket.Serialize()))
        {
            logger.Log("TX", "STATE_UPDATE", responsePacket.payloadSize, "FAIL");
            continue;
        }

        logger.Log("TX", "STATE_UPDATE", responsePacket.payloadSize, "OK");
        std::cout << "[Server] Response sent: " << responseText << "\n";
    }

    return 0;
}