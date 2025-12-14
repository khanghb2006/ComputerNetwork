// ApplicationManager.cpp
#include "../../include/modules/ApplicationManager.h"
#ifdef _WIN32

#include <tlhelp32.h> 
#include <sstream>
#include <iomanip>
#include <Psapi.h>  
#include <fstream>
#include <algorithm>
using namespace std;

// --- STATIC HELPER FUNCTIONS ---

// [Các hàm helper GetWindowOwnerPIDs, utf8_to_wstring, wstring_to_utf8]

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

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

// NOTE: kiểm tra path có phải file .exe tồn tại không
static bool IsValidExeW(const std::wstring& path) {
    if (path.length() < 4) return false;
    if (_wcsicmp(path.c_str() + path.length() - 4, L".exe") != 0)
        return false;
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

// NOTE: tách exe ra khỏi command line (UninstallString)
static std::wstring ExtractExeFromCommand(const std::wstring& cmd) {
    if (cmd.empty()) return L"";
    if (cmd[0] == L'"') {
        size_t end = cmd.find(L'"', 1);
        if (end != std::wstring::npos)
            return cmd.substr(1, end - 1);
    }
    size_t space = cmd.find(L' ');
    return cmd.substr(0, space);
}

// NOTE: scan thư mục để tìm exe đầu tiên
static std::wstring FindExeInFolder(const std::wstring& folder) {
    if (folder.empty()) return L"";
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((folder + L"\\*.exe").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        FindClose(h);
        return folder + L"\\" + fd.cFileName;
    }
    return L"";
}

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

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

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

void EnumerateRegistryKey(HKEY hBaseKey, const std::wstring& subPath, std::vector<ApplicationInfo>& out) {
    HKEY hKey;
    if (RegOpenKeyExW(hBaseKey, subPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;

    DWORD index = 0;
    WCHAR subkeyName[256];

    while (true) {
        DWORD subkeyNameSize = _countof(subkeyName);
        if (RegEnumKeyExW(hKey, index++, subkeyName, &subkeyNameSize,
            NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            break;

        HKEY hSubKey;
        if (RegOpenKeyExW(hKey, subkeyName, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
            continue;

        WCHAR displayName[256];
        DWORD size = sizeof(displayName);
        DWORD type = 0;

        // NOTE: bỏ entry không có DisplayName (tránh rác)
        if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &type,
            (LPBYTE)displayName, &size) != ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
            continue;
        }

        std::wstring exePath;
        WCHAR buf[MAX_PATH];

        // ===== 1️⃣ DisplayIcon (ưu tiên cao nhất) =====
        size = sizeof(buf);
        if (RegQueryValueExW(hSubKey, L"DisplayIcon", NULL, &type,
            (LPBYTE)buf, &size) == ERROR_SUCCESS) {

            std::wstring icon = buf;
            size_t comma = icon.find(L',');
            if (comma != std::wstring::npos)
                icon = icon.substr(0, comma);

            if (IsValidExeW(icon))
                exePath = icon;
        }

        // ===== 2️⃣ UninstallString =====
        if (exePath.empty()) {
            size = sizeof(buf);
            if (RegQueryValueExW(hSubKey, L"UninstallString", NULL, &type,
                (LPBYTE)buf, &size) == ERROR_SUCCESS) {

                std::wstring exe = ExtractExeFromCommand(buf);
                if (IsValidExeW(exe))
                    exePath = exe;
            }
        }

        // ===== 3️⃣ InstallLocation (scan folder) =====
        if (exePath.empty()) {
            size = sizeof(buf);
            if (RegQueryValueExW(hSubKey, L"InstallLocation", NULL, &type,
                (LPBYTE)buf, &size) == ERROR_SUCCESS) {

                std::wstring found = FindExeInFolder(buf);
                if (IsValidExeW(found))
                    exePath = found;
            }
        }

        // NOTE: nếu vẫn không tìm được exe → bỏ, vì start không được
        if (!exePath.empty()) {
            out.emplace_back(
                0,
                ApplicationManager::wstring_to_utf8(displayName),
                ApplicationManager::wstring_to_utf8(exePath)
            );
        }

        RegCloseKey(hSubKey);
    }

    RegCloseKey(hKey);
}

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

std::vector<ApplicationInfo> ApplicationManager::listAllInstalledApplicationsFromRegistry() {
    std::vector<ApplicationInfo> out;
    const std::wstring subPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

    // 1. HKEY_LOCAL_MACHINE (64-bit)
    EnumerateRegistryKey(HKEY_LOCAL_MACHINE, subPath, out);

    // 2. HKEY_LOCAL_MACHINE (32-bit trên 64-bit OS)
    EnumerateRegistryKey(HKEY_LOCAL_MACHINE, L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", out);

    // 3. HKEY_CURRENT_USER (Ứng dụng cá nhân)
    EnumerateRegistryKey(HKEY_CURRENT_USER, subPath, out);

    return out;
}

// --- PUBLIC INTERFACE IMPLEMENTATION ---

// Trả về vector<ApplicationInfo>
std::vector<ApplicationInfo> ApplicationManager::listApplication() {
    return listAllInstalledApplicationsFromRegistry();
}

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

std::optional<int> ApplicationManager::startApplication(
    const std::string& application,
    const std::string& args
) {
    std::wstring appW = utf8_to_wstring(application);
    std::wstring argsW = utf8_to_wstring(args);

    // NOTE: Chỉ cho phép start file .exe tồn tại thật
    if (appW.length() < 4 ||
        _wcsicmp(appW.c_str() + appW.length() - 4, L".exe") != 0 ||
        GetFileAttributesW(appW.c_str()) == INVALID_FILE_ATTRIBUTES) {
        return std::nullopt;
    }

    return startApplicationWindows(appW, argsW);
}

// ===================================================================
// ========================BAO KHANG==================================
// =================================================================== 

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool ApplicationManager::stopApplication(const std::string& appNameOrPath) {
    if (appNameOrPath.empty()) return false;

    // Chuẩn bị Input: Chuyển input của người dùng sang chữ thường
    std::string lowerInput = toLower(appNameOrPath); 
    bool processTerminated = false; // Biến theo dõi xem có process nào bị dừng chưa

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    
    if (Process32FirstW(snap, &pe)) {
        do {
            // Lấy Tên file và chuyển sang chữ thường
            std::string currentName = wstring_to_utf8(pe.szExeFile);
            std::string lowerCurrentName = toLower(currentName);

            // 1. Logic lấy Full Path
            std::string currentPath = "[unknown]"; 
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
            
            if (hProcess != NULL) {
                WCHAR szPath[MAX_PATH];
                DWORD dwSize = MAX_PATH;
                if (QueryFullProcessImageNameW(hProcess, 0, szPath, &dwSize)) {
                    currentPath = wstring_to_utf8(szPath);
                }
                CloseHandle(hProcess);
            }
            
            // Chuyển Path của hệ thống sang chữ thường
            std::string lowerCurrentPath = toLower(currentPath);
            
            // 2. So sánh KHÔNG PHÂN BIỆT CHỮ HOA/THƯỜNG
            // Kiểm tra: Tên file EXE hoặc Đường dẫn đầy đủ có khớp với input không
            if (lowerCurrentName == lowerInput || lowerCurrentPath == lowerInput) {
                
                if (killAppicationsByID(static_cast<int>(pe.th32ProcessID))) {
                    processTerminated = true; // Đánh dấu đã dừng thành công
                }
                // KHÔNG return ngay lập tức để tìm và dừng các process trùng tên/path khác
            }
        } while (Process32NextW(snap, &pe));
    }
    
    CloseHandle(snap);
    return processTerminated; // Trả về true nếu ít nhất một process bị dừng
}

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