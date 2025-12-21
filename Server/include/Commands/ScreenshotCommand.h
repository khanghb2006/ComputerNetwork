#pragma once
#include "ICommand.h"

class ScreenshotCommand : public ICommand {
public:
    ScreenshotCommand() = default;
    std::string execute(const std::string& args) override;
};