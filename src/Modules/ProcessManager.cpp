#include "../../include/modules/ProcessManager.h"


#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>

// convert wide char → UTF-8
static std::string ws2s(const WCHAR* w) {
    int size = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    std::string s(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], size, NULL, NULL);
    if (!s.empty() && s.back() == 0) s.pop_back();
    return s;
}

std::vector<ProcessInfo> ProcessManager::listProcesses() {
    std::vector<ProcessInfo> list;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return list;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            info.name = ws2s(pe.szExeFile);

            list.push_back(info);

        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return list;
}

bool ProcessManager::stopProcess(unsigned long pid) {
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) return false;

    BOOL ok = TerminateProcess(h, 0);
    CloseHandle(h);
    return ok == TRUE;
}


#elif __linux__

#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <cctype>

std::vector<ProcessInfo> ProcessManager::listProcesses() {
    std::vector<ProcessInfo> list;

    DIR* dir = opendir("/proc");
    if (!dir) return list;

    struct dirent* ent;
    while ((ent = readdir(dir))) {
        if (!isdigit(ent->d_name[0])) continue;

        std::string pidStr = ent->d_name;
        std::ifstream f("/proc/" + pidStr + "/comm");

        std::string name;
        if (f.is_open()) getline(f, name);

        ProcessInfo info;
        info.pid = std::stoul(pidStr);
        info.name = name;

        list.push_back(info);
    }

    closedir(dir);
    return list;
}

bool ProcessManager::stopProcess(unsigned long pid) {
    return kill(pid, SIGTERM) == 0;
}

#elif __APPLE__

#include <libproc.h>
#include <unistd.h>
#include <signal.h>

std::vector<ProcessInfo> ProcessManager::listProcesses() {
    std::vector<ProcessInfo> list;

    int count = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    std::vector<pid_t> pids(count);

    proc_listpids(PROC_ALL_PIDS, 0, pids.data(), count * sizeof(pid_t));

    char name[PROC_PIDPATHINFO_MAXSIZE];

    for (pid_t pid : pids) {
        if (pid <= 0) continue;

        if (proc_name(pid, name, sizeof(name)) > 0) {
            ProcessInfo info;
            info.pid = pid;
            info.name = std::string(name);

            list.push_back(info);
        }
    }

    return list;
}

bool ProcessManager::stopProcess(unsigned long pid) {
    return kill(pid, SIGTERM) == 0;
}

#endif




std::vector<ProcessInfo> ProcessModule::list() {
    ProcessManager pm;
    return pm.listProcesses();
}

bool ProcessModule::kill(unsigned long pid) {
    ProcessManager pm;
    return pm.stopProcess(pid);
}