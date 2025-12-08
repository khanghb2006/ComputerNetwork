#pragma once

#include <string>
#include "ICommand.h"
// Bỏ include "modules/Application.h" và con trỏ Application*

class CommandFactory {
	public :
		static ICommand* create(const std::string& cdmName); 
};