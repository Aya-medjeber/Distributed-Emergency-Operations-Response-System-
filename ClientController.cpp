#include "ClientController.h"
#include "ClientUI.h"
#include "TcpClient.h"
#include "Packet.h"
#include "Logger.h"
#include "ClientHelpers.h"
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <fstream>

static bool SaveBinaryFile(const std::string& path, const std::vector<char>& data)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
        return false;

    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    return true;
}

static std::string ExtractStateValue(const std::string& responseText)
{
    const std::string prefix = "STATE:";
    if (responseText.rfind(prefix, 0) == 0)
    {
        return responseText.substr(prefix.length());
    }
    return "";
}

bool ClientController::IsValidMenuOption(int choice) const
{
    return IsValidMenuOptionHelper(choice);
}

void ClientController::Run()
{
    ClientUI ui;
    TcpClient client;
    Logger logger("client_log.txt");

    ui.ShowWelcome();

    if (!client.Initialize())
    {
        std::cout << "[Client] Network initialization failed.\n";
        return;
    }

    int choice = -1;
    bool connected = false;
    bool loggedIn = false;
    std::string currentState = "NOT_CONNECTED";

    std::cout << "[Client] Current State: " << currentState << "\n";

    while (choice != 0)
    {
        ui.ShowMenu();

        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            std::cout << "[Client] Invalid input. Please enter a number.\n";
            continue;
        }

        if (!IsValidMenuOption(choice))
        {
            ui.ShowSelectedOption(-1);
            continue;
        }

        ui.ShowSelectedOption(choice);

        if (choice == 1)
        {
            if (!connected)
            {
                connected = client.Connect("127.0.0.1", 54000);
                if (!connected)
                    continue;

                currentState = "CONNECTED_NOT_AUTHENTICATED";
                std::cout << "[Client] Current State: " << currentState << "\n";
            }

            std::string username;
            std::string password;

            std::cout << "Username: ";
            std::cin >> username;
            std::cout << "Password: ";
            std::cin >> password;

            std::string credentials = username + ":" + password;

            Packet loginPacket(
                CommandType::LOGIN_REQUEST,
                0,
                Packet::StringToPayload(credentials)
            );

            if (!client.SendBytes(loginPacket.Serialize()))
            {
                std::cout << "[Client] Failed to send login packet.\n";
                logger.Log("TX", "LOGIN_REQUEST", loginPacket.payloadSize, "FAIL");
                continue;
            }

            logger.Log("TX", "LOGIN_REQUEST", loginPacket.payloadSize, "OK");

            std::vector<char> rawResponse;
            if (!client.ReceiveBytes(rawResponse))
            {
                std::cout << "[Client] Failed to receive login response.\n";
                continue;
            }

            Packet responsePacket;
            if (!Packet::Deserialize(rawResponse, responsePacket))
            {
                std::cout << "[Client] Failed to parse login response.\n";
                logger.Log("RX", "LOGIN_RESPONSE", static_cast<int>(rawResponse.size()), "FAIL");
                continue;
            }

            logger.Log("RX", "LOGIN_RESPONSE", responsePacket.payloadSize, "OK");

            std::string responseText = Packet::PayloadToString(responsePacket.payload);
            std::cout << "[Client] Server Response: " << responseText << "\n";

            loggedIn = (responseText == "LOGIN_SUCCESS");

            if (loggedIn)
            {
                currentState = "NORMAL";
            }
            else
            {
                currentState = "CONNECTED_NOT_AUTHENTICATED";
            }

            std::cout << "[Client] Current State: " << currentState << "\n";
        }
        else if (choice >= 2 && choice <= 6)
        {
            if (!connected)
            {
                std::cout << "[Client] Connect first using Login.\n";
                continue;
            }

            if (!loggedIn)
            {
                std::cout << "[Client] You must log in first.\n";
                continue;
            }

            std::string commandText = GetCommandTextFromChoiceHelper(choice);

            Packet commandPacket(
                CommandType::STATE_UPDATE,
                1,
                Packet::StringToPayload(commandText)
            );

            if (!client.SendBytes(commandPacket.Serialize()))
            {
                std::cout << "[Client] Failed to send command packet.\n";
                logger.Log("TX", commandText, commandPacket.payloadSize, "FAIL");
                continue;
            }

            logger.Log("TX", commandText, commandPacket.payloadSize, "OK");

            std::vector<char> rawResponse;
            if (!client.ReceiveBytes(rawResponse))
            {
                std::cout << "[Client] Failed to receive command response.\n";
                continue;
            }

            Packet responsePacket;
            if (!Packet::Deserialize(rawResponse, responsePacket))
            {
                std::cout << "[Client] Failed to parse command response.\n";
                logger.Log("RX", "COMMAND_RESPONSE", static_cast<int>(rawResponse.size()), "FAIL");
                continue;
            }

            if (responsePacket.commandType == CommandType::REPORT_DATA)
            {
                if (SaveBinaryFile("received_situation_report.jpg", responsePacket.payload))
                {
                    std::cout << "[Client] Situation report saved as received_situation_report.jpg\n";
                    logger.Log("RX", "REPORT_DATA", responsePacket.payloadSize, "OK");
                }
                else
                {
                    std::cout << "[Client] Failed to save received report.\n";
                    logger.Log("RX", "REPORT_DATA", responsePacket.payloadSize, "FAIL");
                }

                std::cout << "[Client] Current State: " << currentState << "\n";
                continue;
            }

            std::string responseText = Packet::PayloadToString(responsePacket.payload);
            std::cout << "[Client] Server Response: " << responseText << "\n";

            std::string extractedState = ExtractStateValue(responseText);
            if (!extractedState.empty())
            {
                currentState = extractedState;
            }

            std::cout << "[Client] Current State: " << currentState << "\n";

            logger.Log("RX", "COMMAND_RESPONSE", responsePacket.payloadSize, "OK");
        }
    }

    client.Disconnect();
}