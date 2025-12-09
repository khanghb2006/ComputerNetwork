#include "../../include/modules/Screenshot.h"
#ifdef _WIN32

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

// Make sure to link against gdi32.lib if you get linker errors.
// (Usually handled automatically by Visual Studio)

bool Screenshot::take(const std::string& filename) {
    // Get metrics for the entire "virtual screen" (all monitors combined)
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vw <= 0 || vh <= 0) return false;

    // Get a handle to the screen's Device Context (DC)
    HDC hScreen = GetDC(nullptr);
    if (!hScreen) return false;

    // Create a compatible "memory" DC to draw on
    HDC hMemDC = CreateCompatibleDC(hScreen);
    if (!hMemDC) {
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    // Create a compatible bitmap to hold the screenshot
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, vw, vh);
    if (!hBitmap) {
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    // Select the new bitmap into the memory DC
    HBITMAP oldBmp = (HBITMAP)SelectObject(hMemDC, hBitmap);

    // Perform the screen capture using BitBlt (Bit-Block Transfer)
    // Copy from the screen DC to the memory DC.
    if (!BitBlt(hMemDC, 0, 0, vw, vh, hScreen, vx, vy, SRCCOPY | CAPTUREBLT)) {
        // If BitBlt fails, clean up and return
        SelectObject(hMemDC, oldBmp);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

    // Save the HBITMAP from memory to a file
    bool ok = SaveBitmapToBMPFile(hBitmap, filename);

    // --- Cleanup ---
    SelectObject(hMemDC, oldBmp); // Select the old bitmap back
    DeleteObject(hBitmap);      // Delete the screenshot bitmap
    DeleteDC(hMemDC);           // Delete the memory DC
    ReleaseDC(nullptr, hScreen);  // Release the screen DC
    return ok;
}

std::string Screenshot::captureToBuffer() {
    // 1. Lấy kích thước màn hình
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // 2. Lấy Device Context (DC) của màn hình
    HDC hScreenDC = GetDC(NULL);
    if (hScreenDC == NULL) return ""; // Lỗi 1
    
    // 3. Tạo DC Tương thích
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (hMemoryDC == NULL) {
        ReleaseDC(NULL, hScreenDC);
        return ""; // Lỗi 2
    }

    // 4. Tạo Bitmap Tương thích
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    if (hBitmap == NULL) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return ""; // Lỗi 3
    }
    
    // 5. Chọn Bitmap vào Memory DC
    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

    // 6. Chụp màn hình (Sao chép Bit)
    if (!BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY)) {
        // Lỗi 4: Xóa đối tượng và thoát nếu BitBlt thất bại
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return ""; 
    }

    // 7. TRÍCH XUẤT DỮ LIỆU BMP RA BUFFER
    
    // Chuẩn bị header BMP
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    
    // Lấy thông tin Bitmap (infoHeader)
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = screen_width;
    infoHeader.biHeight = screen_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32; // Màu 32-bit (Phổ biến)
    infoHeader.biCompression = BI_RGB;

    DWORD dwBmpSize = ((screen_width * infoHeader.biBitCount + 31) / 32) * 4 * screen_height;
    
    // Tạo Buffer chứa dữ liệu pixel
    std::vector<char> bmpBuffer(dwBmpSize);

    // Lấy dữ liệu pixel
    if (GetDIBits(hScreenDC, hBitmap, 0, screen_height, bmpBuffer.data(), (BITMAPINFO*)&infoHeader, DIB_RGB_COLORS) == 0) {
        // Lỗi 5: Xóa đối tượng và thoát nếu GetDIBits thất bại
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return "";
    }
    
    // Điền thông tin File Header
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    // Ghép File Header, Info Header và Dữ liệu Pixel vào std::string
    std::string finalBuffer;
    finalBuffer.reserve(fileHeader.bfSize);
    
    // Thêm File Header
    finalBuffer.append((char*)&fileHeader, sizeof(BITMAPFILEHEADER));
    // Thêm Info Header
    finalBuffer.append((char*)&infoHeader, sizeof(BITMAPINFOHEADER));
    // Thêm Dữ liệu Pixel
    finalBuffer.append(bmpBuffer.data(), dwBmpSize);
    
    // 8. Dọn dẹp
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return finalBuffer; // Trả về Buffer BMP hoàn chỉnh
}

// Helper to save HBITMAP to a BMP file
bool Screenshot::SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename) {
    BITMAP bmp;
    // Get information about the bitmap
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) return false;

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int bpp = 32; // We'll save as 32-bits per pixel (BGRA)
    // Calculate row size (must be padded to a multiple of 4 bytes)
    int rowBytes = ((width * bpp + 31) / 32) * 4;
    int imageSize = rowBytes * height;

    // --- BMP File Header ---
    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 0x4D42; // 'BM'
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + imageSize;

    // --- BMP Info Header ---
    BITMAPINFOHEADER bih;
    ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height; // Positive value means bottom-up DIB
    bih.biPlanes = 1;
    bih.biBitCount = (WORD)bpp;
    bih.biCompression = BI_RGB; // No compression
    bih.biSizeImage = imageSize;

    HDC hdc = GetDC(nullptr);
    if (!hdc) return false;

    // We need a DC to use GetDIBits
    HDC memDC = CreateCompatibleDC(hdc);
    if (!memDC) {
        ReleaseDC(nullptr, hdc);
        return false;
    }

    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBitmap);
    std::vector<char> pixels(imageSize);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.bmiHeader = bih;

    // Extract the raw pixel data (bits) from the HBITMAP
    if (!GetDIBits(memDC, hBitmap, 0, height, pixels.data(), &bi, DIB_RGB_COLORS)) {
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    // --- Write to file ---
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // Failed to open file (e.g., bad path, permissions)
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    // Write the headers and the pixel data
    ofs.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
    ofs.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
    ofs.write(pixels.data(), imageSize);
    ofs.close();

    // Cleanup
    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);
    return true;
}

#endif // _WIN32