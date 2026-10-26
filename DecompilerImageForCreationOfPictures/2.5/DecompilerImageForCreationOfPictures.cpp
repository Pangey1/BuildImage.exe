#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
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

std::wstring StringToWString(const std::string& s) {
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::vector<wchar_t> buf(len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buf.data(), len);
    return std::wstring(buf.data());
}

std::vector<std::string> SplitByComma(const std::string& s) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t\r\n"));
        item.erase(item.find_last_not_of(" \t\r\n") + 1);
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

bool ProcessSingleDecompilation(const std::string& inputFile, const std::string& outputFile) {
    std::wstring wInputFile = StringToWString(inputFile);
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wInputFile.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        std::cerr << "\x1B[31mDecompiler Error: Failed to open or decode structure of " << inputFile << "\x1B[0m\n";
        if (bitmap) delete bitmap;
        return false;
    }

    int widthX = bitmap->GetWidth();
    int heightY = bitmap->GetHeight();

    std::ofstream txtOut(outputFile);
    if (!txtOut.is_open()) {
        std::cerr << "\x1B[31mWrite Error: Failed to create output text file: " << outputFile << "\x1B[0m\n";
        delete bitmap;
        return false;
    }

    for (int y = 0; y < heightY; y++) {
        for (int x = 0; x < widthX; x++) {
            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);

            RGBColor pixel = { pixelColor.GetR(), pixelColor.GetG(), pixelColor.GetB() };
            int tokenValue = GetPaletteValueFromColor(pixel);

            if (tokenValue < 10) txtOut << "00" << tokenValue;
            else if (tokenValue < 100) txtOut << "0" << tokenValue;
            else txtOut << tokenValue;

            if (y == heightY - 1 && x == widthX - 1) txtOut << ".";
            else txtOut << ", ";
        }
        txtOut << "\n";
    }

    txtOut.close();
    delete bitmap;
    std::cout << " -> " << inputFile << " successfully decompiled into " << outputFile << " (" << widthX << "x" << heightY << ")\n";
    return true;
}

void ShowHelp() {
    std::cout << "DecompilerImageForCreationOfPictures (Batch Mode v2.6 - No Size Limits)\n";
    std::cout << "Usage single: DecompilerImageForCreationOfPictures.exe -i input.png -o output.txt\n";
    std::cout << "Usage batch:  DecompilerImageForCreationOfPictures.exe -i 1.png,2.png -o 1.txt,2.txt\n";
}

int main(int argc, char* argv[]) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int wArgc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &wArgc);

    if (wArgc < 2) {
        ShowHelp();
        LocalFree(argvW);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 0;
    }

    if (wArgc == 2) {
        std::wstring inputFileW = argvW[1];
        if (IsSupportedFormat(inputFileW)) {
            std::wstring outputFileW = L"";
            size_t dotPos = inputFileW.find_last_of(L".");
            if (dotPos != std::wstring::npos) {
                outputFileW = inputFileW.substr(0, dotPos) + L".txt";
            }
            else {
                outputFileW = inputFileW + L".txt";
            }

            std::string inputFileA(inputFileW.begin(), inputFileW.end());
            std::string outputFileA(outputFileW.begin(), outputFileW.end());

            std::cout << "[Drag-and-Drop Mode] Processing single image asset...\n";
            ProcessSingleDecompilation(inputFileA, outputFileA);
        }
        else {
            std::cerr << "\x1B[31mError: Unsupported file extension descriptor!\x1B[0m\n";
        }
        LocalFree(argvW);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        system("pause");
        return 0;
    }

    std::string inputArg = "";
    std::string outputArg = "";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) inputArg = argv[++i];
        else if (arg == "-o" && i + 1 < argc) outputArg = argv[++i];
    }

    if (inputArg.empty() || outputArg.empty()) {
        std::cerr << "\x1B[31mDecompiler Error: Input or output specifications missing.\x1B[0m\n";
        LocalFree(argvW);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 1;
    }

    std::vector<std::string> inputFiles = SplitByComma(inputArg);
    std::vector<std::string> outputFiles = SplitByComma(outputArg);

    if (inputFiles.size() != outputFiles.size()) {
        std::cerr << "\x1B[31mBatch Error: Files count mismatch!\x1B[0m\n";
        std::cerr << "\x1B[33mYou specified " << inputFiles.size() << " input images but " << outputFiles.size() << " output text names.\x1B[0m\n";
        LocalFree(argvW);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return 1;
    }

    std::cout << "[Batch Mode] Starting decompilation for " << inputFiles.size() << " images...\n\n";

    int successCount = 0;
    for (size_t i = 0; i < inputFiles.size(); i++) {
        if (ProcessSingleDecompilation(inputFiles[i], outputFiles[i])) {
            successCount++;
        }
    }

    std::cout << "\n\x1B[32mBATCH COMPLETE: " << successCount << " of " << inputFiles.size() << " images decompiled successfully.\x1B[0m\n";

    LocalFree(argvW);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}
