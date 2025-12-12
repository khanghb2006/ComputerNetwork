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

void EnumerateRegistryKey(HKEY hBaseKey, const std::wstring& subPath, std::vector<ApplicationInfo>& out) {
    HKEY hKey;
    if (RegOpenKeyExW(hBaseKey, subPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        WCHAR subkeyName[256];
        DWORD subkeyNameSize;

        while (true) {
            subkeyNameSize = sizeof(subkeyName) / sizeof(WCHAR);
            if (RegEnumKeyExW(hKey, index++, subkeyName, &subkeyNameSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
                break;
            }

            HKEY hSubKey;
            if (RegOpenKeyExW(hKey, subkeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                
                // Đọc DisplayName (Tên ứng dụng)
                WCHAR displayName[256];
                DWORD displayNameSize = sizeof(displayName);
                DWORD type = 0;
                
                if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &type, (LPBYTE)displayName, &displayNameSize) == ERROR_SUCCESS) {
                    
                    WCHAR path[MAX_PATH] = L"";
                    DWORD size = sizeof(path);
                    bool pathFound = false;

                    // 1. Ưu tiên đọc DisplayIcon (Thường là đường dẫn EXE. Ví dụ: C:\Chrome\chrome.exe,0)
                    if (RegQueryValueExW(hSubKey, L"DisplayIcon", NULL, &type, (LPBYTE)path, &size) == ERROR_SUCCESS) {
                        
                        // Cắt chuỗi nếu DisplayIcon chứa chỉ mục (ví dụ: "C:\App\app.exe,0")
                        std::wstring pathW = path;
                        size_t commaPos = pathW.find(L',');
                        if (commaPos != std::wstring::npos) {
                            pathW = pathW.substr(0, commaPos);
                            wcscpy(path, pathW.c_str());
                        }
                        pathFound = true;
                    }
                    
                    // 2. Nếu vẫn không có đường dẫn EXE, thử đọc InstallLocation 
                    //    (Thường là thư mục cài đặt, không phải file EXE)
                    if (!pathFound) {
                        size = sizeof(path);
                        RegQueryValueExW(hSubKey, L"InstallLocation", NULL, &type, (LPBYTE)path, &size);
                        // pathFound = true; // Không chắc chắn là EXE, nhưng vẫn ghi nhận
                    }

                    // 3. Nếu vẫn không có, thử đọc UninstallString 
                    if (path[0] == L'\0') {
                        size = sizeof(path);
                        RegQueryValueExW(hSubKey, L"UninstallString", NULL, &type, (LPBYTE)path, &size);
                        // Cắt bỏ phần uninstaller.exe
                        // Logic phức tạp, tạm thời ưu tiên DisplayIcon và InstallLocation
                    }

                    // Thêm vào kết quả
                    out.emplace_back(
                        0, // PID = 0
                        ApplicationManager::wstring_to_utf8(displayName),
                        ApplicationManager::wstring_to_utf8(path)
                    );
                }
                RegCloseKey(hSubKey);
            }
        }
        RegCloseKey(hKey);
    }
}


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

std::optional<int> ApplicationManager::startApplication(const std::string& application, const std::string& args) {
    std::wstring appW = utf8_to_wstring(application);
    std::wstring argsW = utf8_to_wstring(args);
    
    // Nếu path không chứa đường dẫn (ví dụ: chỉ là 'spotify'), Windows sẽ tìm trong PATH môi trường.
    // Lỗi xảy ra vì logic startApplicationWindows của bạn yêu cầu đường dẫn tuyệt đối hoặc lệnh hệ thống.
    
    // Tạo CMD line để Windows tự xử lý (tương tự ShellExecute)
    std::wstring cmdline = L"cmd /c start "; // Sử dụng CMD để Windows tìm file
    cmdline += appW;
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
    
    // Sử dụng nullptr cho lpApplicationName để Windows tự tìm lệnh 'cmd'
    BOOL ok = CreateProcessW(nullptr, cmdBuf.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);

    if (!ok) {
        return std::nullopt;
    }
    // ... (Trả về PID)
    CloseHandle(pi.hThread);
    int pid = static_cast<int>(pi.dwProcessId);
    CloseHandle(pi.hProcess);
    return pid;
}

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