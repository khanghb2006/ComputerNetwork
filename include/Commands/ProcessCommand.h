#pragma once
#include "../ICommand.h"

class ProcessCommand : public ICommand {
public:
    std::string execute(const std::string& args) override;
};
