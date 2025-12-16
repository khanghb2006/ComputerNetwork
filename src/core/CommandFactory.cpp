#include "CommandFactory.h"
#include "RecordCommand.h"
#include "ShutdownCommand.h"
#include "RestartCommand.h"
#include "ApplicationCommand.h"
#include "ScreenshotCommand.h"
#include "KeyCommand.h"
#include "ProcessCommand.h"

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

	if (cmd == "key")
		return new KeyCommand();

	if (cmd == "process")
		return new ProcessCommand();

	if (cmd == "application")
		return new ApplicationCommand();

	if(cmd == "screenshot")
		return new ScreenshotCommand();

	std::cout << "Invalid command name: " << cmdName << "\n";
	return nullptr;
}