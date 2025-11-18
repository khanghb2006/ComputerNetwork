#pragma once
#include <string>
#include <vector>

struct ProcessInfo {
    unsigned long pid;
    std::string name;
};

class ProcessManager {
public:
    std::vector<ProcessInfo> listProcesses();
    bool stopProcess(unsigned long pid);
    bool startProcess(const std::string& path);        // m? m?i th? (process)
};



class ProcessModule {
public:
    // list running processes
    static std::vector<ProcessInfo> list();

    // kill/stop process by pid
    static bool kill(unsigned long pid);

    // open a path (startProcess in your code - opens files, folders, urls, lnk, or exe depending on impl)
    static bool start(const std::string& path);
};