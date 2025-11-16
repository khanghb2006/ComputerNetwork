#pragma once

#include "ICommand.h"

class SystemControl : public ICommand {
	public:
		bool runCommand(ICommand* cmd);
};