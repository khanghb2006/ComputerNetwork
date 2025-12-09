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
};



class ProcessModule {
public:
    // list running processes
    static std::vector<ProcessInfo> list();

    // kill/stop process by pid
    static bool kill(unsigned long pid);

};