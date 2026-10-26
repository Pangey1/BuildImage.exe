#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
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
        if (gigaBytes > 1.0) return false;
        return true;
    }
    return false;
}

void ProcessDecompilation(const std::wstring& inputFile, const std::wstring& outputFile) {
    if (!CheckFileSize(inputFile)) {
        StartErrorTimeout("Decompiler Error: File size exceeds the 1 GB limit! Compilation aborted.");
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
        std::cout << "\rDecompiling progress: [" << percent << "%]... ";
        std::cout.flush();
    }

    txtOut.close();
    delete bitmap;
    std::cout << "\n\x1B[32m" << "SUCCESS! Code saved directly next to your image file." << "\x1B[0m\n\n";
    system("pause");
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
        std::cout << "DecompilerImageForCreationOfPictures (Universal Unicode Engine v2.5)\n";
        std::cout << "Usage: Drag and drop ANY image up to 1 GB onto this executable.\n";
        std::wcout << L"Allowed formats: .png, .jpg, .jpeg, .bmp, .webp, .ico, .gif, .tiff, .svg\n";
        std::cout << "\nPress any key to exit...";
        std::cin.get();
        LocalFree(argv);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 0;
    }

    std::wstring inputFile = L"";
    std::wstring outputFile = L"";

    if (argc == 2) {
        inputFile = argv[1];
        if (IsSupportedFormat(inputFile)) {
            size_t slashPos = inputFile.find_last_of(L"\\/");
            if (slashPos != std::wstring::npos) {
                outputFile = inputFile.substr(0, slashPos + 1) + L"Image.txt";
            }
            else {
                outputFile = L"Image.txt";
            }
            ProcessDecompilation(inputFile, outputFile);
        }
        else {
            LocalFree(argv);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            StartErrorTimeout("Error: Unsupported file format! Allowed: .png, .jpg, .jpeg, .bmp, .webp, .ico, .gif, .tiff, .svg");
            return 1;
        }
    }
    else {
        std::wstring flagInput = L"";
        std::wstring flagOutput = L"";
        for (int i = 1; i < argc; i++) {
            std::wstring arg = argv[i];
            if (arg == L"-i" && i + 1 < argc) flagInput = argv[++i];
            else if (arg == L"-o" && i + 1 < argc) flagOutput = argv[++i];
        }
        if (flagInput.empty() || flagOutput.empty() || !IsSupportedFormat(flagInput)) {
            LocalFree(argv);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            StartErrorTimeout("Compiler Error: Invalid CMD parameters or unsupported file format.");
            return 1;
        }
        ProcessDecompilation(flagInput, flagOutput);
    }

    LocalFree(argv);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
