#include "../../include/Commands/ShutdownCommand.h"
#include <cstdlib>
#include <iostream>
#include <string>

ShutdownCommand::ShutdownCommand() : delay(0) {}

void ShutdownCommand::setDelay(int sec) {
	delay = sec;
}

std::string ShutdownCommand::execute(const std::string& args) {
	std::string cmd = "shutdown -s -t " + std::to_string(delay);
	std::cout << "[Shutdown] " << cmd << "\n";
	int result = system(cmd.c_str());
	return "Shutdown";
}