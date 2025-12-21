#pragma once
#include "../ICommand.h"

class RestartCommand : public ICommand {
	private:
		int delay;
	public:
		RestartCommand();
		void setDelay(int sec);
		std::string execute(const std::string &args) override;
};