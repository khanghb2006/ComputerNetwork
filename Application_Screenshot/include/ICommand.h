#pragma once
#include <string>

class ICommand {
	public:
		virtual ~ICommand() = default;

		// Execute the command
		virtual std::string execute(const std::string& args) = 0;
};