#pragma once

class ClientController
{
public:
    void Run();

private:
    bool IsValidMenuOption(int choice) const;
};