#include "ServerConsole.h"
#include "StateMachine.h"
#include "TcpServer.h"
#include "Packet.h"
#include "Logger.h"
#include <iostream>
#include <string>
#include <vector>

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

    std::vector<char> rawData;
    if (!server.ReceiveBytes(rawData))
    {
        console.ShowMessage("Failed to receive data.");
        return 1;
    }

    Packet receivedPacket;
    if (!Packet::Deserialize(rawData, receivedPacket))
    {
        console.ShowMessage("Failed to deserialize packet.");
        logger.Log("RX", "UNKNOWN_PACKET", static_cast<int>(rawData.size()), "FAIL");
        return 1;
    }

    logger.Log("RX", "LOGIN_REQUEST", receivedPacket.payloadSize, "OK");

    std::string credentials = Packet::PayloadToString(receivedPacket.payload);
    std::cout << "[Server] Received credentials: " << credentials << "\n";

    bool loginSuccess = (credentials == "aya:1234");
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
        return 1;
    }

    logger.Log("TX", "LOGIN_RESPONSE", responsePacket.payloadSize, "OK");
    std::cout << "[Server] Login response sent: " << responseText << "\n";

    return 0;
}