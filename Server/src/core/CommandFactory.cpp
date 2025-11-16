#include "CommandFactory.h"
#include "../../include/Commands/RecordCommand.h"
#include "../../include/Commands/ShutdownCommand.h"
#include "../../include/Commands/RestartCommand.h"

#include <iostream>

ICommand* CommandFactory::create(const std::string& cmdName) {
	// to lower case cdmName
	std::string cmd = cmdName;
	for (auto& c : cmd) c = tolower(c);

	if (cmd == "record")
		return new RecordCommand();
	
	if (cmd == "shutdown")
		return new ShutdownCommand();

	if (cmd == "restart") 
		return new RestartCommand();

	std::cout << "Invalid command name: " << cmdName << "\n";
	return nullptr;
}