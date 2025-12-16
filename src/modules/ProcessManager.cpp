#include "ProcessManager.h"

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
#endif

std::vector<ProcessInfo> ProcessModule::list() {
    ProcessManager pm;
    return pm.listProcesses();
}

bool ProcessModule::stop(unsigned long pid) {
    ProcessManager pm;
    return pm.stopProcess(pid);
}