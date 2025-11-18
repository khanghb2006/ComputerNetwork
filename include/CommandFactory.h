#pragma once

#include <string>
#include "ICommand.h"

class CommandFactory {
	public :
		static ICommand* create(const std::string& cdmName);
};