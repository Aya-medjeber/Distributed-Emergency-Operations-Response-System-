#include "ClientController.h"
#include "ClientUI.h"
#include "TcpClient.h"
#include "Packet.h"
#include "Logger.h"
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static std::string GetCommandTextFromChoice(int choice)
{
    switch (choice)
    {
    case 2: return "DECLARE_ALERT";
    case 3: return "ESCALATE_ALERT";
    case 4: return "RESOLVE_ALERT";
    case 5: return "RESET_SYSTEM";
    case 6: return "REQUEST_SITUATION_REPORT";
    default: return "";
    }
}

bool ClientController::IsValidMenuOption(int choice) const
{
    return choice >= 0 && choice <= 6;
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

            std::string commandText = GetCommandTextFromChoice(choice);

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

            std::string responseText = Packet::PayloadToString(responsePacket.payload);
            std::cout << "[Client] Server Response: " << responseText << "\n";

            logger.Log("RX", "COMMAND_RESPONSE", responsePacket.payloadSize, "OK");
        }
    }

    client.Disconnect();
}