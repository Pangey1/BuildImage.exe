#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

struct RGBColor {
    uint8_t r, g, b;
};
RGBColor GetColorFromPalette(int value) {
    RGBColor color = { 0, 0, 0 };
    if (value >= 0 && value <= 40) {
        uint8_t gray = static_cast<uint8_t>((value * 255) / 40);
        color = { gray, gray, gray };
    }
    else if (value >= 41 && value <= 80) {
        int t = value - 41;
        color.r = 255; color.g = static_cast<uint8_t>((t * 255) / 39); color.b = 0;
    }
    else if (value >= 81 && value <= 120) {
        int t = value - 81;
        color.r = static_cast<uint8_t>(255 - ((t * 255) / 39)); color.g = 255; color.b = 0;
    }
    else if (value >= 121 && value <= 160) {
        int t = value - 121;
        color.r = 0; color.g = static_cast<uint8_t>((t * 255) / 39); color.b = 255;
    }
    else if (value >= 161 && value <= 200) {
        int t = value - 161;
        color.r = static_cast<uint8_t>((t * 255) / 39); color.g = 0; color.b = static_cast<uint8_t>(255 - ((t * 100) / 39));
    }
    return color;
}

int GetPaletteValueFromColor(RGBColor pixel) {
    int bestValue = 0;
    double minDistance = 1e9;
    for (int val = 0; val <= 200; val++) {
        RGBColor candidate = GetColorFromPalette(val);
        double distance = std::sqrt(
            std::pow(pixel.r - candidate.r, 2) +
            std::pow(pixel.g - candidate.g, 2) +
            std::pow(pixel.b - candidate.b, 2)
        );
        if (distance < minDistance) {
            minDistance = distance;
            bestValue = val;
        }
    }
    return bestValue;
}
bool IsSupportedFormat(const std::wstring& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::wstring::npos) return false;
    std::wstring ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return std::tolower(c); });
    return (ext == L".bmp" || ext == L".png" || ext == L".jpg" || ext == L".jpeg" ||
        ext == L".webp" || ext == L".ico" || ext == L".gif" || ext == L".tif" ||
        ext == L".tiff" || ext == L".svg");
}

void StartErrorTimeout(const std::string& errorMsg) {
    std::cerr << "\x1B[31m" << errorMsg << "\x1B[0m\n";
    for (int i = 30; i > 0; i--) {
        std::cout << "\rThe program will automatically close in " << i << " seconds... ";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\n";
}

bool CheckFileSize(const std::wstring& filePath) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(filePath.c_str(), GetFileExInfoStandard, &fad)) {
        LARGE_INTEGER size;
        size.HighPart = fad.nFileSizeHigh;
        size.LowPart = fad.nFileSizeLow;
        double gigaBytes = static_cast<double>(size.QuadPart) / (1024.0 * 1024.0 * 1024.0);
        if (gigaBytes > 2.0) return false;
        return true;
    }
    return false;
}

void PrintCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    localtime_s(&buf, &in_time_t);
    std::cout << "\x1B[36m[" << std::put_time(&buf, "%H:%M:%S") << "]\x1B[0m ";
}

void ProcessDecompilation(const std::wstring& inputFile, const std::wstring& outputFile) {
    if (!CheckFileSize(inputFile)) {
        StartErrorTimeout("Decompiler Error: File size exceeds the 2 GB limit! Compilation aborted.");
        return;
    }

    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(inputFile.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        if (bitmap) delete bitmap;
        StartErrorTimeout("Decompiler Error: Failed to open or decode image file structure.");
        return;
    }

    int widthX = bitmap->GetWidth();
    int heightY = bitmap->GetHeight();

    PrintCurrentTime();
    std::cout << "\n=========================================\n";
    std::cout << " [IMAGE DECOMPILER - STRUCTURE DETECTED]\n";
    std::cout << " Width (Columns X): " << widthX << "\n";
    std::cout << " Height (Rows Y):   " << heightY << "\n";
    std::cout << "=========================================\n\n";

    std::ofstream txtOut(outputFile);
    if (!txtOut.is_open()) {
        delete bitmap;
        StartErrorTimeout("Write Error: Failed to create output text file next to the source image.");
        return;
    }

    auto frameStart = std::chrono::high_resolution_clock::now();

    for (int y = 0; y < heightY; y++) {
        for (int x = 0; x < widthX; x++) {
            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);

            RGBColor pixel;
            pixel.r = pixelColor.GetR();
            pixel.g = pixelColor.GetG();
            pixel.b = pixelColor.GetB();

            int tokenValue = GetPaletteValueFromColor(pixel);
            if (tokenValue < 10) txtOut << "00" << tokenValue;
            else if (tokenValue < 100) txtOut << "0" << tokenValue;
            else txtOut << tokenValue;

            if (y == heightY - 1 && x == widthX - 1) txtOut << ".";
            else txtOut << ", ";
        }
        txtOut << "\n";

        int percent = static_cast<int>(((y + 1) * 100) / heightY);
        auto frameNow = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frameElapsed = frameNow - frameStart;

        std::cout << "\rDecompiling progress: [" << percent << "%] | Elapsed: "
            << std::fixed << std::setprecision(1) << frameElapsed.count() << "s... ";
        std::cout.flush();
    }

    txtOut.close();
    delete bitmap;

    std::cout << "\n\n";
    PrintCurrentTime();
    std::wcout << L"\x1B[32mSUCCESS! Code saved directly as: " << outputFile << L"\x1B[0m\n\n";
    system("pause");
}

void ShowHelp() {
    std::cout << "DecompilerImageForCreationOfPictures (Universal Legendary Engine v2.7)\n";
    std::cout << "Usage: Drag and drop ANY image up to 2 GB onto this executable.\n";
    std::cout << "Supported formats: .png, .jpg, .jpeg, .bmp, .webp, .ico, .gif, .tiff, .svg\n";
}

int main() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc < 2) {
        ShowHelp();
        LocalFree(argv);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 0;
    }

    auto globalStart = std::chrono::high_resolution_clock::now();

    std::wstring inputFile = argv[1];
    std::wstring outputFile = L"";

    if (IsSupportedFormat(inputFile)) {
        size_t dotPos = inputFile.find_last_of(L".");
        if (dotPos != std::wstring::npos) {
            outputFile = inputFile.substr(0, dotPos) + L".txt";
        }
        else {
            outputFile = inputFile + L".txt";
        }
        ProcessDecompilation(inputFile, outputFile);
    }
    else {
        LocalFree(argv);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        StartErrorTimeout("Error: Unsupported file format! Allowed: .png, .jpg, .jpeg, .bmp, .webp, .ico, .gif, .tiff, .svg");
        return 1;
    }

    auto globalEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = globalEnd - globalStart;
    std::cout << "=========================================\n";
    std::cout << " Total execution time: " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds.\n";
    std::cout << "=========================================\n\n";

    LocalFree(argv);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
