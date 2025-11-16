#include "SystemControl.h"
#include <iostream>

bool SystemControl::runCommand(ICommand* cmd) {
	if (!cmd) {
		std::cout << "Invalid command" << "\n";
		return false;
	}
	bool ok = !cmd->execute("").empty();
	delete cmd;
	return ok;
}