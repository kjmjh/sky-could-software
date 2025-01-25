#include <windows.h>
#include <stdio.h>
#include <strsafe.h> // For StringCchPrintfW
#include <pathcch.h>  // For PathCchPrintfW

#pragma comment(lib, "Pathcch.lib")

// Function to save a HBITMAP to a BMP file
BOOL SaveBitmapToFile(HBITMAP hBitmap, const wchar_t* filename) {
    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
        wprintf(L"GetObject failed with error %lu\n", GetLastError());
        return FALSE;
    }

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // 24-bit bitmap
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

    // Allocate memory for the bitmap data
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB) {
        wprintf(L"GlobalAlloc failed with error %lu\n", GetLastError());
        return FALSE;
    }

    char* pDIB = (char*)GlobalLock(hDIB);
    if (!pDIB) {
        wprintf(L"GlobalLock failed with error %lu\n", GetLastError());
        GlobalFree(hDIB);
        return FALSE;
    }

    HDC hDC = GetDC(NULL);
    if (!GetDIBits(hDC, hBitmap, 0, bmp.bmHeight, pDIB, (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        wprintf(L"GetDIBits failed with error %lu\n", GetLastError());
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        ReleaseDC(NULL, hDC);
        return FALSE;
    }
    ReleaseDC(NULL, hDC);

    // Create the file
    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wprintf(L"CreateFileW failed with error %lu\n", GetLastError());
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    // Prepare the BITMAPFILEHEADER
    bmfHeader.bfType = 0x4D42; // "BM"
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    DWORD dwWritten;
    if (!WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL)) {
        wprintf(L"WriteFile (BITMAPFILEHEADER) failed with error %lu\n", GetLastError());
        CloseHandle(hFile);
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    if (!WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWritten, NULL)) {
        wprintf(L"WriteFile (BITMAPINFOHEADER) failed with error %lu\n", GetLastError());
        CloseHandle(hFile);
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    if (!WriteFile(hFile, pDIB, dwBmpSize, &dwWritten, NULL)) {
        wprintf(L"WriteFile (DIB data) failed with error %lu\n", GetLastError());
        CloseHandle(hFile);
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return FALSE;
    }

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    CloseHandle(hFile);
    return TRUE;
}

// Function to capture the screen and save it to a file
BOOL CaptureScreen(const wchar_t* folderPath) {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        wprintf(L"CreateCompatibleBitmap failed with error %lu\n", GetLastError());
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return FALSE;
    }

    if (!SelectObject(hMemoryDC, hBitmap)) {
        wprintf(L"SelectObject failed with error %lu\n", GetLastError());
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return FALSE;
    }

    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY)) {
        wprintf(L"BitBlt failed with error %lu\n", GetLastError());
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return FALSE;
    }

    // Ensure the folder exists
    if (!CreateDirectoryW(folderPath, NULL)) {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS) {
            wprintf(L"CreateDirectoryW failed with error %lu\n", dwError);
            DeleteObject(hBitmap);
            DeleteDC(hMemoryDC);
            ReleaseDC(NULL, hScreenDC);
            return FALSE;
        }
    }

    // Prepare the filename
    wchar_t filename[MAX_PATH];
    if (FAILED(PathCchPrintfW(filename, MAX_PATH, L"%s\\screenshot.bmp", folderPath))) {
        wprintf(L"PathCchPrintfW failed\n");
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return FALSE;
    }

    // Save the bitmap to file
    if (!SaveBitmapToFile(hBitmap, filename)) {
        wprintf(L"SaveBitmapToFile failed\n");
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return FALSE;
    }

    // Clean up
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return TRUE;
}

int wmain() {
    const wchar_t* folderPath = L"skj";
    if (CaptureScreen(folderPath)) {
        wprintf(L"Screenshot saved to %s\\screenshot.bmp\n", folderPath);
    } else {
        wprintf(L"Failed to save screenshot\n");
    }
    return 0;
}