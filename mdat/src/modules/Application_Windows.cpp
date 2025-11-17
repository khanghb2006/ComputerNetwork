// Application_Windows.cpp
#include "../../include/modules/Application_Windows.h"
#ifdef _WIN32

#include <windows.h>
#include <tlhelp32.h> // For process snapshots
#include <vector>
#include <sstream>
#include <iomanip>
#include <set>      // For GetWindowOwnerPIDs
#include <Psapi.h>  // For QueryFullProcessImageNameW
#include <fstream>

using namespace std;

// ---------- HELPER FUNCTIONS ----------
static std::wstring utf8_to_wstring(const std::string& s) {
    if (s.empty()) return {};
    int req = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (req <= 0) return {};
    std::wstring w;
    w.resize(req);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], req);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

static std::string wstring_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    int req = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (req <= 0) return {};
    std::string s;
    s.resize(req);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], req, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

static BOOL CALLBACK EnumWindowCallback(HWND hwnd, LPARAM lParam) {
    std::set<DWORD>* pPIDs = reinterpret_cast<std::set<DWORD>*>(lParam);

    // Get only visible windows
    if (IsWindowVisible(hwnd)) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid); // get PID
        if (pid != 0) {
            pPIDs->insert(pid); // Add to the set
        }
    }
    return TRUE; // Continue;
}

static std::set<DWORD> GetWindowOwnerPIDs() {
    std::set<DWORD> pids;
    EnumWindows(EnumWindowCallback, reinterpret_cast<LPARAM>(&pids));
    return pids;
}


// ---------- Public Interface Functions ----------

std::optional<int> Application_Windows::startApplication(const std::string& application, const std::string& args) {
    // Convert public-facing std::string to std::wstring for the internal Windows function
    std::wstring appW = utf8_to_wstring(application);
    std::wstring argsW = utf8_to_wstring(args);
    return startApplicationWindows(appW, argsW);
}

bool Application_Windows::stopApplication(int pid) {
    return stopApplicationWindows(pid);
}

std::vector<ProcessInfo> Application_Windows::listApplication() {
    return listApplicationWindows();
}


// ---------- Internal Windows Helper Functions ----------

std::vector<ProcessInfo> Application_Windows::listApplicationWindows() {
    std::vector<ProcessInfo> out;

    // Step 1: Get the list of PIDs for processes with visible windows
    std::set<DWORD> windowPIDs = GetWindowOwnerPIDs();

    // Step 2: Get a snapshot of all processes
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return out;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            DWORD currentPID = pe.th32ProcessID;

            // Step 3: CHECK - Only add if this PID is in the list of windowed processes
            if (windowPIDs.find(currentPID) == windowPIDs.end()) {
                continue; // Skip this process
            }

            // Step 4: Convert the name
            std::string name = wstring_to_utf8(pe.szExeFile);
            if (name.empty()) name = "[unknown]";

            // Step 5: Get the full path (Path)
            std::string path = "[Access Denied]";
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentPID);

            if (hProcess != NULL) {
                WCHAR szPath[MAX_PATH];
                DWORD dwSize = MAX_PATH;
                // Use QueryFullProcessImageNameW to get the path
                if (QueryFullProcessImageNameW(hProcess, 0, szPath, &dwSize)) {
                    path = wstring_to_utf8(szPath);
                }
                CloseHandle(hProcess);
            }

            // Step 6: FIX WAS HERE - Call emplace_back with 3 PARAMETERS
            out.emplace_back(static_cast<int>(currentPID), name, path);

        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return out;
}

std::optional<int> Application_Windows::startApplicationWindows(const std::wstring& applicationW, const std::wstring& argsW)
{
    std::wstring cmdline;
    if (!applicationW.empty()) {
        cmdline += L"\"";
        cmdline += applicationW;
        cmdline += L"\"";
    }
    if (!argsW.empty()) {
        cmdline += L" ";
        cmdline += argsW;
    }

    std::vector<wchar_t> cmdBuf(cmdline.begin(), cmdline.end());
    cmdBuf.push_back(L'\0');

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL ok = CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    if (!ok) {
        // DWORD err = GetLastError();
        return std::nullopt;
    }
    CloseHandle(pi.hThread);
    int pid = static_cast<int>(pi.dwProcessId);
    CloseHandle(pi.hProcess);
    return pid;
}

bool Application_Windows::stopApplicationWindows(int pid) {
    if (pid <= 0) return false;

    // Get a handle to the process with PROCESS_TERMINATE rights
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!h) return false; // Failed to get handle (e.g., process not found or access denied)

    // Terminate the process
    BOOL ok = TerminateProcess(h, 1); // 1 = exit code
    CloseHandle(h); // Clean up the handle
    return ok == TRUE;
}

bool Application_Windows::exportApplicationToFile(const std::string& filepath) {
	// 1. Open the file for writing
    std::ofstream outFile(filepath);
    if (!outFile.is_open()) {
        return false;
    }

	// 2. Get the list of applications
    auto list = this->listApplicationWindows();

	// 3.   Write header information
    outFile << "=== PROCESS LIST ===\n";
    outFile << "Found " << list.size() << " windowed processes:\n\n";

	// 4. Write each application's details
    for (const auto& p : list) {
        outFile << "PID: " << p.pid() << "\n";
        outFile << "  Name: " << p.name() << "\n";
        outFile << "  Path: " << p.path() << "\n";
        outFile << "---------------------------------\n";
    }

    outFile.close();
    return true;
}

std::string Application_Windows::GetLastErrorString(DWORD err) {
    if (err == 0) err = GetLastError();
    if (err == 0) return std::string("No error");
    LPWSTR buf = nullptr;
    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buf, 0, nullptr);

    std::string out;
    if (size && buf) {
        // Convert the WCHAR error message to UTF-8
        int needed = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            out.resize(needed);
            WideCharToMultiByte(CP_UTF8, 0, buf, -1, &out[0], needed, nullptr, nullptr);
            if (!out.empty() && out.back() == '\0') out.pop_back();
        }
        LocalFree(buf); // Free the buffer allocated by FormatMessage
    }
    else {
        out = "Unknown error";
    }

    // Append the error code for clarity
    std::ostringstream ss;
    ss << out << " (code " << err << ")";
    return ss.str();
}

#endif // _WIN32