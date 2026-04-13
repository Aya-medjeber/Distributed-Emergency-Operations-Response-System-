
#include "ServerConsole.h"
#include "StateMachine.h"
#include <iostream>

int main()
{
    ServerConsole console;
    StateMachine stateMachine;

    console.ShowStartup();
    console.ShowState(stateMachine.GetCurrentState());

    console.ShowMessage("Server started.");
    console.ShowMessage("Waiting for client connection...");
    console.ShowMessage("No connections yet.");

    return 0;
}