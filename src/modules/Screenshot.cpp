#include "Screenshot.h"
#ifdef _WIN32

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

// Ensure that gdi32.lib is linked in case of linker errors.
bool Screenshot::take(const std::string& filename) {

	// get  virtual screen dimensions
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (vw <= 0 || vh <= 0) return false;

	// get device context of the screen
    HDC hScreen = GetDC(nullptr);
    if (!hScreen) return false;

	// create a memory device context compatible with the screen
    HDC hMemDC = CreateCompatibleDC(hScreen);
    if (!hMemDC) {
        ReleaseDC(nullptr, hScreen);
        return false;
    }

	// create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, vw, vh);
    if (!hBitmap) {
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

	// choose the new bitmap into the memory DC
    HBITMAP oldBmp = (HBITMAP)SelectObject(hMemDC, hBitmap);

	// capture the screen into the bitmap
    if (!BitBlt(hMemDC, 0, 0, vw, vh, hScreen, vx, vy, SRCCOPY | CAPTUREBLT)) {
        // clear if failed
        SelectObject(hMemDC, oldBmp);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(nullptr, hScreen);
        return false;
    }

	//save HBITMAP to file
    bool ok = SaveBitmapToBMPFile(hBitmap, filename);

    // clear up
    SelectObject(hMemDC, oldBmp);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreen);
    return ok;
}

std::string Screenshot::captureToBuffer() {
	// get the screen dimensions
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// get the device context of the screen
    HDC hScreenDC = GetDC(NULL);
    if (hScreenDC == NULL) return ""; 

	// create a memory device context compatible with the screen
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (hMemoryDC == NULL) {
        ReleaseDC(NULL, hScreenDC);
        return ""; 
    }

	// create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screen_width, screen_height);
    if (hBitmap == NULL) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return "";
    }

	// choose the new bitmap into the memory DC
    HGDIOBJ hOldBitmap = SelectObject(hMemoryDC, hBitmap);

	// capture the screen into the bitmap
    if (!BitBlt(hMemoryDC, 0, 0, screen_width, screen_height, hScreenDC, 0, 0, SRCCOPY)) {
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return "";
    }

	// transfer bitmap data to buffer

	// BMP file header and info header
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    // FILL infoHeader
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = screen_width;
    infoHeader.biHeight = screen_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32; // 32-bit
    infoHeader.biCompression = BI_RGB;

    DWORD dwBmpSize = ((screen_width * infoHeader.biBitCount + 31) / 32) * 4 * screen_height;

	// buffer contain pixel data
    std::vector<char> bmpBuffer(dwBmpSize);

    // Lấy pixel từ bitmap
    if (GetDIBits(hScreenDC, hBitmap, 0, screen_height, bmpBuffer.data(), (BITMAPINFO*)&infoHeader, DIB_RGB_COLORS) == 0) {
        // Lỗi 5
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return "";
    }

    // Điền fileHeader
    fileHeader.bfType = 0x4D42; // 'BM'
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Nối các phần lại thành buffer hoàn chỉnh
    std::string finalBuffer;
    finalBuffer.reserve(fileHeader.bfSize);

    finalBuffer.append((char*)&fileHeader, sizeof(BITMAPFILEHEADER));
    finalBuffer.append((char*)&infoHeader, sizeof(BITMAPINFOHEADER));
    finalBuffer.append(bmpBuffer.data(), dwBmpSize);

    // 8. Dọn dẹp
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return finalBuffer;
}

// Hàm phụ: Lưu HBITMAP thành file BMP
bool Screenshot::SaveBitmapToBMPFile(HBITMAP hBitmap, const std::string& filename) {
    BITMAP bmp;
    // Lấy thông tin bitmap
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) return false;

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int bpp = 32; // Lưu 32-bit (BGRA)
    int rowBytes = ((width * bpp + 31) / 32) * 4;
    int imageSize = rowBytes * height;

    // --- BMP File Header ---
    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 0x4D42;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + imageSize;

    // --- BMP Info Header ---
    BITMAPINFOHEADER bih;
    ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = (WORD)bpp;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;

    HDC hdc = GetDC(nullptr);
    if (!hdc) return false;

    // Cần DC để dùng GetDIBits
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

    // Lấy pixel raw từ HBITMAP
    if (!GetDIBits(memDC, hBitmap, 0, height, pixels.data(), &bi, DIB_RGB_COLORS)) {
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

    // --- Ghi ra file ---
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // Lỗi khi mở file
        SelectObject(memDC, oldBmp);
        DeleteDC(memDC);
        ReleaseDC(nullptr, hdc);
        return false;
    }

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