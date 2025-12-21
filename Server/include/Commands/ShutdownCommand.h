#pragma once
#include "ICommand.h"

class ShutdownCommand : public ICommand {
	private:
		int delay;

	public:
		ShutdownCommand();
		void setDelay(int sec);
		std::string execute(const std::string &args) override;
};	