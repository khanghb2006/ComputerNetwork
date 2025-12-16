#pragma once
#include "ICommand.h"

class KeyCommand : public ICommand {
public:
    std::string execute(const std::string& args) override;
};