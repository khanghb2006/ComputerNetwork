#pragma once
#include "Application.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <optional>

class Application_Windows : public Application {
public:
    Application_Windows() = default;
    ~Application_Windows() override = default;

    // These functions override the pure virtual functions from the Application base class.
    std::vector<ProcessInfo> listApplication() override;
    std::optional<int> startApplication(const std::string& application, const std::string& args = "") override;
    bool stopApplication(int pid) override;
    bool exportApplicationToFile(const std::string& filepath) override;

private:
    // These functions contain the actual Win32 API calls.
    std::vector<ProcessInfo> listApplicationWindows();
    std::optional<int> startApplicationWindows(const std::wstring& applicationW, const std::wstring& argsW);
    bool stopApplicationWindows(int pid);

    static std::string GetLastErrorString(DWORD err = 0);
};

#endif // _WIN32