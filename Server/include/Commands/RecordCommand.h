#pragma once
#include "ICommand.h"

class RecordCommand : public ICommand {
	public :
		std::string execute(const std::string& args) override;
};