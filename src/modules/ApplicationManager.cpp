// ApplicationManager.cpp
#include "../../include/modules/ApplicationManager.h"
#ifdef _WIN32

#include <tlhelp32.h> 
#include <sstream>
#include <iomanip>
#include <Psapi.h>  
#include <fstream>

using namespace std;

// --- STATIC HELPER FUNCTIONS ---

// [Các hàm helper GetWindowOwnerPIDs, utf8_to_wstring, wstring_to_utf8 không cần thay đổi tên]

static BOOL CALLBACK EnumWindowCallback(HWND hwnd, LPARAM lParam) {
    std::set<DWORD>* pPIDs = reinterpret_cast<std::set<DWORD>*>(lParam);
    if (IsWindowVisible(hwnd)) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid); 
        if (pid != 0) {
            pPIDs->insert(pid); 
        }
    }
    return TRUE; 
}

std::set<DWORD> ApplicationManager::GetWindowOwnerPIDs() {
    std::set<DWORD> pids;
    EnumWindows(EnumWindowCallback, reinterpret_cast<LPARAM>(&pids));
    return pids;
}

std::wstring ApplicationManager::utf8_to_wstring(const std::string& s) {
    if (s.empty()) return {};
    int req = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (req <= 0) return {};
    std::wstring w;
    w.resize(req);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], req);
    if (!w.empty() && w.back() == L'\0') w.pop_back();
    return w;
}

std::string ApplicationManager::wstring_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    int req = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (req <= 0) return {};
    std::string s;
    s.resize(req);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], req, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

// --- INTERNAL WINDOWS FUNCTIONS ---

// Trả về vector<ApplicationInfo>
std::vector<ApplicationInfo> ApplicationManager::listApplicationWindows() {
    std::vector<ApplicationInfo> out;

    // 1. Lấy danh sách PID của các application có cửa sổ hiển thị
    std::set<DWORD> windowPIDs = GetWindowOwnerPIDs();

    // 2. Lấy snapshot của tất cả application/process
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return out;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            DWORD currentPID = pe.th32ProcessID;

            // 3. CHECK - Chỉ thêm nếu PID này có trong danh sách windowPIDs
            if (windowPIDs.find(currentPID) == windowPIDs.end()) {
                continue; // Bỏ qua application này
            }

            // 4. Chuyển đổi tên
            std::string name = wstring_to_utf8(pe.szExeFile);
            if (name.empty()) name = "[unknown]";

            // 5. Lấy full path
            std::string path = "[Access Denied]";
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentPID);

            if (hProcess != NULL) {
                WCHAR szPath[MAX_PATH];
                DWORD dwSize = MAX_PATH;
                if (QueryFullProcessImageNameW(hProcess, 0, szPath, &dwSize)) {
                    path = wstring_to_utf8(szPath);
                }
                CloseHandle(hProcess);
            }

            // 6. Thêm vào kết quả (sử dụng ApplicationInfo)
            out.emplace_back(static_cast<int>(currentPID), name, path);

        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return out;
}

std::optional<int> ApplicationManager::startApplicationWindows(const std::wstring& applicationW, const std::wstring& argsW)
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
    DWORD creationFlags = CREATE_NEW_CONSOLE;
    BOOL ok = CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, FALSE, creationFlags, nullptr, nullptr, &si, &pi);

    if (!ok) {
        return std::nullopt;
    }
    CloseHandle(pi.hThread);
    int pid = static_cast<int>(pi.dwProcessId);
    CloseHandle(pi.hProcess);
    return pid;
}

bool ApplicationManager::killAppicationsByID(int pid) {
    if (pid <= 0) return false;
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!h) return false; 
    BOOL ok = TerminateProcess(h, 1); 
    CloseHandle(h); 
    return ok == TRUE;
}

// --- PUBLIC INTERFACE IMPLEMENTATION ---

// Trả về vector<ApplicationInfo>
std::vector<ApplicationInfo> ApplicationManager::listApplication() {
    return listApplicationWindows();
}

std::optional<int> ApplicationManager::startApplication(const std::string& application, const std::string& args) {
    std::wstring appW = utf8_to_wstring(application);
    std::wstring argsW = utf8_to_wstring(args);
    return startApplicationWindows(appW, argsW);
}

bool ApplicationManager::stopApplication(const std::string& appNameOrPath) {
    if(appNameOrPath.empty()) return false;

    // 1. Lấy danh sách ứng dụng đang chạy
    std::vector<ApplicationInfo> runningApps = listApplicationWindows();
    
    // 2. Tìm và đóng ứng dụng
    for(const auto& app : runningApps) {
        // So sánh theo tên hoặc đường dẫn
        if(app.name() == appNameOrPath || app.path() == appNameOrPath) {
            return killAppicationsByID(app.pid());
        } 
    }

    return false;
}

bool ApplicationManager::exportApplicationToFile(const std::string& filepath) {
	// 1. Mở file để ghi
    std::ofstream outFile(filepath);
    if (!outFile.is_open()) {
        return false;
    }

	// 2. Lấy danh sách ứng dụng
    auto list = ApplicationManager::listApplicationWindows();

	// 3. Ghi thông tin header
    outFile << "=== APPLICATION LIST ===\n";
    outFile << "Found " << list.size() << " windowed applications:\n\n";

	// 4. Ghi chi tiết từng ứng dụng
    for (const auto& app : list) {
        outFile << "PID: " << app.pid() << "\n";
        outFile << "  Name: " << app.name() << "\n";
        outFile << "  Path: " << app.path() << "\n";
        outFile << "---------------------------------\n";
    }

    outFile.close();
    return true;
}

// [GetLastErrorString không cần thay đổi tên]

std::string ApplicationManager::GetLastErrorString(DWORD err) {
    if (err == 0) err = GetLastError();
    if (err == 0) return std::string("No error");
    LPWSTR buf = nullptr;
    DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buf, 0, nullptr);

    std::string out;
    if (size && buf) {
        int needed = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        if (needed > 0) {
            out.resize(needed);
            WideCharToMultiByte(CP_UTF8, 0, buf, -1, &out[0], needed, nullptr, nullptr);
            if (!out.empty() && out.back() == '\0') out.pop_back();
        }
        LocalFree(buf); 
    }
    else {
        out = "Unknown error";
    }

    std::ostringstream ss;
    ss << out << " (code " << err << ")";
    return ss.str();
}

#endif // _WIN32