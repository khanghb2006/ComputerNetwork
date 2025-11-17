#pragma once
#include "../ICommand.h"
// Screenshot không cần Application, nó dùng class Screenshot riêng
// Nhưng cần include để biết class Screenshot nằm ở đâu (nếu dùng trong .cpp)

class ScreenshotCommand : public ICommand {
public:
    ScreenshotCommand() = default;
    std::string execute(const std::string& args) override;
};