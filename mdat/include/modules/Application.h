#pragma once
#include <string>
#include <vector>
#include <optional>
#include <fstream>

class ProcessInfo {
public:
    ProcessInfo() = default;
    ProcessInfo(int pid, const std::string& name, const std::string& path)
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

class Application {
public:
    Application() = default;
    virtual ~Application() = default;

    // List running processes
    virtual std::vector<ProcessInfo> listApplication() = 0;

    // Start application: returns pid or nullopt
    virtual std::optional<int> startApplication(const std::string& application, const std::string& args = "") = 0;

    // Stop process by pid
    virtual bool stopApplication(const std::string& appNameOrPath) = 0;

	// Export application list to a text file
    virtual bool exportApplicationToFile(const std::string& filepath) = 0;
};