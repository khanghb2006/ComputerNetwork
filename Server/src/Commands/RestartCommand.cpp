#include "RestartCommand.h"
#include <cstdlib>
#include <string>

RestartCommand::RestartCommand() : delay(0) {}

void RestartCommand::setDelay(int sec) {
	delay = sec;
}

std::string RestartCommand::execute(const std::string& args) {
	std::string cmd = "shutdown -r -t " + std::to_string(delay);
	int result = system(cmd.c_str());
	return "Restart";
}