#include <iostream>
#include "CommandFactory.h"
#include "modules/Webcam.h"
#include "SystemControl.h"

int main() {
	// Test command record
	std::cout << "How to use this program: " << "\n";
	std::cout << "Enter /\"record/\" if you want to open Webcam" << "\n";
	std::cout << "Enter /\"restart/\" if you wnt to restart system" << "\n";
	std::cout << "Enter /\"shutdown/\" if you want to shutdown system" << "\n";
	
	std::string commandName;
	std::cin >> commandName;

	ICommand* cmd = CommandFactory::create(commandName);
	if (!cmd) {
		std::cout << "Command not found" << "\n";
		return 0;
	}

	//Webcam cam(0);
	//cam.showPreview();

	std::string res = cmd->execute("");
	std::cout << "[RESULT]" << res << "\n";
	
	delete cmd;
	cmd = nullptr;
	return 0;
}