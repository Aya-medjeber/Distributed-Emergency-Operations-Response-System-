#include "ClientUI.h"
#include <iostream>

void ClientUI::ShowWelcome() const
{
    std::cout << "=========================================\n";
    std::cout << " DEORS Client - Operator Dashboard\n";
    std::cout << "=========================================\n";
}

void ClientUI::ShowMenu() const
{
    std::cout << "\nPlease choose an option:\n";
    std::cout << "1. Login\n";
    std::cout << "2. Declare Alert\n";
    std::cout << "3. Escalate Alert\n";
    std::cout << "4. Resolve Alert\n";
    std::cout << "5. Reset System\n";
    std::cout << "6. Request Situation Report\n";
    std::cout << "0. Exit\n";
    std::cout << "Choice: ";
}

void ClientUI::ShowSelectedOption(int choice) const
{
    switch (choice)
    {
    case 1:
        std::cout << "[Client] Login selected.\n";
        break;
    case 2:
        std::cout << "[Client] Declare Alert selected.\n";
        break;
    case 3:
        std::cout << "[Client] Escalate Alert selected.\n";
        break;
    case 4:
        std::cout << "[Client] Resolve Alert selected.\n";
        break;
    case 5:
        std::cout << "[Client] Reset System selected.\n";
        break;
    case 6:
        std::cout << "[Client] Request Situation Report selected.\n";
        break;
    case 0:
        std::cout << "[Client] Exiting application.\n";
        break;
    default:
        std::cout << "[Client] Invalid option.\n";
        break;
    }
}