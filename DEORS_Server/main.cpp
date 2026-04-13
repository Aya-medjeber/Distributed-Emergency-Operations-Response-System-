#include "ServerConsole.h"
#include "StateMachine.h"
#include "TcpServer.h"
#include "Packet.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <vector>

static CommandType ParseCommandFromText(const std::string& text)
{
    if (text == "DECLARE_ALERT") return CommandType::DECLARE_ALERT;
    if (text == "ESCALATE_ALERT") return CommandType::ESCALATE_ALERT;
    if (text == "RESOLVE_ALERT") return CommandType::RESOLVE_ALERT;
    if (text == "RESET_SYSTEM") return CommandType::RESET_SYSTEM;
    if (text == "REQUEST_SITUATION_REPORT") return CommandType::REQUEST_SITUATION_REPORT;
    return CommandType::ERROR_RESPONSE;
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

            bool loginSuccess = (credentials == "aya:1234");
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
        CommandType command = ParseCommandFromText(commandText);

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