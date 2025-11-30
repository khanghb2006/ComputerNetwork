#pragma once

#include <string>
#include "ICommand.h"
#include "modules/Application.h" 

class CommandFactory {
	public :
		static ICommand* create(const std::string& cdmName, Application* app);
};