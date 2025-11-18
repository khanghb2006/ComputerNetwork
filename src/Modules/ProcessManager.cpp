#include "../../include/modules/ProcessManager.h"

// #ifdef _WIN32
// #include <windows.h>
// #include <tlhelp32.h>
// #include <codecvt>
// #include <locale>
// #include <shellapi.h>

// static std::string ws2s(const WCHAR* wstr) {
//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
//     return conv.to_bytes(wstr);
// }

// std::vector<ProcessInfo> ProcessManager::listProcesses() {
//     std::vector<ProcessInfo> list;

//     HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//     if (snap == INVALID_HANDLE_VALUE) return list;

//     PROCESSENTRY32 pe;
//     pe.dwSize = sizeof(pe);

//     if (Process32First(snap, &pe)) {
//         do {
//             list.push_back({ pe.th32ProcessID, ws2s(pe.szExeFile) });
//          } while (Process32Next(snap, &pe));
//     }

//     CloseHandle(snap);
//     return list;
// }

// bool ProcessManager::stopProcess(unsigned long pid) {
//     HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
//     if (!h) return false;
//     bool ok = TerminateProcess(h, 0);
//     CloseHandle(h);
//     return ok;
// }


// bool ProcessManager::startProcess(const std::string& path) {
//     HINSTANCE h = ShellExecuteA(
//         NULL,
//         "open",
//         path.c_str(),
//         NULL,
//         NULL,
//         SW_SHOWNORMAL
//     );

//     return (int)h > 32;
// }


// #elif __linux__

// #include <dirent.h>
// #include <unistd.h>
// #include <signal.h>
// #include <fstream>

// std::vector<ProcessInfo> ProcessManager::listProcesses() {
//     std::vector<ProcessInfo> list;
//     DIR* dir = opendir("/proc");
//     if (!dir) return list;

//     struct dirent* entry;
//     while ((entry = readdir(dir))) {
//         if (!isdigit(entry->d_name[0])) continue;

//         std::string pidStr = entry->d_name;
//         std::ifstream f("/proc/" + pidStr + "/comm");
//         std::string name;
//         if (f.is_open()) getline(f, name);

//         list.push_back({ std::stoul(pidStr), name });
//     }

//     closedir(dir);
//     return list;
// }

// bool ProcessManager::stopProcess(unsigned long pid) {
//     return kill(pid, SIGTERM) == 0;
// }

// bool ProcessManager::startProcess(const std::string& path) {
//     pid_t pid = fork();
//     if (pid == 0) {
//         execlp("xdg-open", "xdg-open", path.c_str(), NULL);
//         _exit(1);
//     }
//     return pid > 0;
// }


// #elif __APPLE__

// #include <libproc.h>
// #include <unistd.h>
// #include <signal.h>

// std::vector<ProcessInfo> ProcessManager::listProcesses() {
//     std::vector<ProcessInfo> list;

//     int count = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
//     std::vector<pid_t> pids(count);
//     proc_listpids(PROC_ALL_PIDS, 0, pids.data(), count * sizeof(pid_t));

//     char name[PROC_PIDPATHINFO_MAXSIZE];

//     for (pid_t pid : pids) {
//         if (pid <= 0) continue;
//         if (proc_name(pid, name, sizeof(name)) > 0) {
//             list.push_back({ (unsigned long)pid, std::string(name) });
//         }
//     }

//     return list;
// }

// bool ProcessManager::stopProcess(unsigned long pid) {
//     return kill(pid, SIGTERM) == 0;
// }

// bool ProcessManager::startProcess(const std::string& path) {
//     pid_t pid = fork();
//     if (pid == 0) {
//         execl("/usr/bin/open", "open", path.c_str(), NULL);
//         _exit(1);
//     }
//     return pid > 0;
// }

// #endif

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

bool ProcessManager::startProcess(const std::string& path) {
    // dùng wide API cho an toàn
    int wlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
    std::wstring wpath(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], wlen);

    HINSTANCE h = ShellExecuteW(
        NULL,
        L"open",
        wpath.c_str(),
        NULL,
        NULL,
        SW_SHOWNORMAL
    );

    // ShellExecuteW trả về >32 nếu thành công
    return reinterpret_cast<intptr_t>(h) > 32;
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

bool ProcessManager::startProcess(const std::string& path) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("xdg-open", "xdg-open", path.c_str(), NULL);
        _exit(1);
    }
    return pid > 0;
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

bool ProcessManager::startProcess(const std::string& path) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/usr/bin/open", "open", path.c_str(), NULL);
        _exit(1);
    }
    return pid > 0;
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

bool ProcessModule::start(const std::string& path) {
    ProcessManager pm;
    return pm.startProcess(path);
}