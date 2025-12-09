#pragma once
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <set>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

struct ApplicationInfo {
public:
    ApplicationInfo() = default;
    ApplicationInfo(int pid, const std::string& name, const std::string& path)
        : pid_(pid), name_(name), path_(path) {} 

    // Get function
    int pid() const { return pid_; }
    const std::string& name() const { return name_; }
    const std::string& path() const { return path_; }

private:
    int pid_{ 0 };
    std::string name_;
    std::string path_; 
};

class ApplicationManager {
public:
    static std::vector<ApplicationInfo> listApplication(); 
    static std::optional<int> startApplication(const std::string& application, const std::string& args = "");
    static bool stopApplication(const std::string& appNameOrPath);

#ifdef _WIN32
public:
    static std::vector<ApplicationInfo> listApplicationWindows();
    static std::optional<int> startApplicationWindows(const std::wstring& applicationW, const std::wstring& argsW);
    static bool killAppicationsByID(int pid);
    static std::vector<ApplicationInfo> listAllInstalledApplicationsFromRegistry();
    static std::wstring utf8_to_wstring(const std::string& s);
    static std::string wstring_to_utf8(const std::wstring& w);
    static std::set<DWORD> GetWindowOwnerPIDs();
    static std::string GetLastErrorString(DWORD err = 0);
#endif // _WIN32
};