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
#include <cstdio>
#include <cwctype>
#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

struct RGBColor {
    uint8_t r, g, b;
};

RGBColor GetColorFromPalette(int value) {
    RGBColor color = { 0, 0, 0 };
    if (value >= 0 && value <= 40) {
        uint8_t gray = (uint8_t)((value * 255) / 40);
        color = { gray, gray, gray };
    }
    else if (value >= 41 && value <= 80) {
        int t = value - 41;
        color.r = 255; color.g = (uint8_t)((t * 255) / 39); color.b = 0;
    }
    else if (value >= 81 && value <= 120) {
        int t = value - 81;
        color.r = (uint8_t)(255 - ((t * 255) / 39)); color.g = 255; color.b = 0;
    }
    else if (value >= 121 && value <= 160) {
        int t = value - 121;
        color.r = 0; color.g = (uint8_t)((t * 255) / 39); color.b = 255;
    }
    else if (value >= 161 && value <= 200) {
        int t = value - 161;
        color.r = (uint8_t)((t * 255) / 39); color.g = 0; color.b = (uint8_t)(255 - ((t * 100) / 39));
    }
    return color;
}

// Мгновенный 3D-кэш (32x32x32 = 32768 ячеек) для сверхбыстрого поиска цвета
int GetPaletteValueFromColor(RGBColor pixel) {
    static std::vector<uint8_t> lut3D(32768, 0);
    static bool initialized = false;

    if (!initialized) {
        std::vector<RGBColor> palette(201);
        for (int i = 0; i <= 200; i++) {
            palette.at(i) = GetColorFromPalette(i);
        }

        for (int r5 = 0; r5 < 32; r5++) {
            int r = (r5 * 255) / 31;
            for (int g5 = 0; g5 < 32; g5++) {
                int g = (g5 * 255) / 31;
                for (int b5 = 0; b5 < 32; b5++) {
                    int b = (b5 * 255) / 31;
                    int bestVal = 0;
                    int minDistSq = 1000000000;
                    for (int val = 0; val <= 200; val++) {
                        RGBColor cand = palette.at(val);
                        int dr = r - int(cand.r);
                        int dg = g - int(cand.g);
                        int db = b - int(cand.b);
                        int distSq = dr * dr + dg * dg + db * db;
                        if (distSq < minDistSq) {
                            minDistSq = distSq;
                            bestVal = val;
                        }
                    }
                    int idx = (r5 << 10) | (g5 << 5) | b5;
                    lut3D.at(idx) = (uint8_t)(bestVal);
                }
            }
        }
        initialized = true;
    }

    int rIdx = pixel.r >> 3;
    int gIdx = pixel.g >> 3;
    int bIdx = pixel.b >> 3;
    return int(lut3D.at((rIdx << 10) | (gIdx << 5) | bIdx));
}

std::wstring GetExtensionLower(const std::wstring& filename) {
    size_t dotPos = filename.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return L"";
    std::wstring ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return (wchar_t)(std::towlower(c)); });
    return ext;
}

bool IsSupportedImageFormat(const std::wstring& filename) {
    std::wstring ext = GetExtensionLower(filename);
    return (ext == L".bmp" || ext == L".png" || ext == L".jpg" || ext == L".jpeg" ||
        ext == L".webp" || ext == L".ico" || ext == L".tif" ||
        ext == L".tiff" || ext == L".svg");
}

bool IsGifFormat(const std::wstring& filename) {
    return (GetExtensionLower(filename) == L".gif");
}

bool IsSupportedVideoFormat(const std::wstring& filename) {
    std::wstring ext = GetExtensionLower(filename);
    return (ext == L".mp4" || ext == L".avi" || ext == L".mkv" || ext == L".mov" ||
        ext == L".webm" || ext == L".wmv" || ext == L".flv" || ext == L".m4v" ||
        ext == L".mpeg" || ext == L".mpg" || ext == L".3gp");
}

bool FileExistsW(const std::wstring& path) {
    DWORD attrib = GetFileAttributesW(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

void RemoveDirRecursiveW(const std::wstring& dirPath) {
    std::wstring searchPath = dirPath + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::wstring name = findData.cFileName;
            if (name != L"." && name != L"..") {
                std::wstring fullPath = dirPath + L"\\" + name;
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    RemoveDirRecursiveW(fullPath);
                }
                else {
                    DeleteFileW(fullPath.c_str());
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
    RemoveDirectoryW(dirPath.c_str());
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
        double gigaBytes = double(size.QuadPart) / (1024.0 * 1024.0 * 1024.0);
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

std::wstring GetExeDirectory() {
    std::vector<wchar_t> buffer(MAX_PATH, 0);
    GetModuleFileNameW(NULL, buffer.data(), MAX_PATH);
    std::wstring path(buffer.data());
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) return path.substr(0, pos);
    return L".";
}

std::wstring FindOrAskFFmpeg(int argc, LPWSTR* argv) {
    for (int i = 1; i < argc - 1; i++) {
        std::wstring arg = argv[i];
        if (arg == L"-f") {
            std::wstring candidate = argv[i + 1];
            if (FileExistsW(candidate)) return candidate;
        }
    }

    std::wstring localFfmpeg = GetExeDirectory() + L"\\ffmpeg.exe";
    if (FileExistsW(localFfmpeg)) return localFfmpeg;

    if (FileExistsW(L"ffmpeg.exe")) return L"ffmpeg.exe";

    std::cout << "\x1B[33m[FFmpeg Required] ffmpeg.exe was not found next to the decompiler.\x1B[0m\n";
    std::cout << "Enter full path to ffmpeg.exe (or drag and drop it here): ";
    std::wstring userPath;
    std::getline(std::wcin, userPath);

    if (!userPath.empty() && userPath.front() == L'"') userPath.erase(0, 1);
    if (!userPath.empty() && userPath.back() == L'"') userPath.pop_back();

    if (FileExistsW(userPath)) return userPath;
    return L"";
}

std::string WStringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), int(wstr.size()), NULL, 0, NULL, NULL);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), int(wstr.size()), &result.at(0), sizeNeeded, NULL, NULL);
    return result;
}

bool WriteBitmapFrameToStream(Gdiplus::Bitmap* bitmap, std::ofstream& txtOut, bool isLastFrame) {
    int widthX = bitmap->GetWidth();
    int heightY = bitmap->GetHeight();

    Gdiplus::Rect rect(0, 0, widthX, heightY);
    Gdiplus::BitmapData bmpData;
    if (bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bmpData) != Gdiplus::Ok) {
        return false;
    }

    uint8_t* scan0 = (uint8_t*)(bmpData.Scan0);
    int stride = bmpData.Stride;

    std::string frameBuffer;
    frameBuffer.reserve(size_t(widthX) * size_t(heightY) * 5 + size_t(heightY) * 2 + 16);

    for (int y = 0; y < heightY; y++) {
        uint8_t* rowPtr = scan0 + y * stride;
        for (int x = 0; x < widthX; x++) {
            RGBColor pixel;
            pixel.b = rowPtr[x * 3 + 0];
            pixel.g = rowPtr[x * 3 + 1];
            pixel.r = rowPtr[x * 3 + 2];

            int tokenValue = GetPaletteValueFromColor(pixel);
            frameBuffer.push_back(char('0' + (tokenValue / 100)));
            frameBuffer.push_back(char('0' + ((tokenValue / 10) % 10)));
            frameBuffer.push_back(char('0' + (tokenValue % 10)));

            if (y == heightY - 1 && x == widthX - 1) {
                if (isLastFrame) frameBuffer.push_back('.');
                else frameBuffer += "\n};";
            }
            else {
                frameBuffer += ", ";
            }
        }
        frameBuffer.push_back('\n');
    }

    txtOut.write(frameBuffer.data(), frameBuffer.size());
    bitmap->UnlockBits(&bmpData);
    return true;
}

void ProcessImageDecompilation(const std::wstring& inputFile, const std::wstring& outputFile) {
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
        int percent = int(((y + 1) * 100) / heightY);
        auto frameNow = std::chrono::high_resolution_clock::now();
        std::chrono::duration frameElapsed = frameNow - frameStart;
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
void ProcessAnimatedDecompilation(const std::wstring& inputFile, const std::wstring& outputFile, const std::wstring& wavOutputFile, const std::wstring& ffmpegPath, bool isGifMode) {
    if (!CheckFileSize(inputFile)) {
        StartErrorTimeout("Decompiler Error: File size exceeds the 2 GB limit! Aborted.");
        return;
    }
    if (ffmpegPath.empty()) {
        StartErrorTimeout(isGifMode
            ? "Decompiler Error: Valid ffmpeg.exe path is required for GIF decompilation!"
            : "Decompiler Error: Valid ffmpeg.exe path is required for video decompilation!");
        return;
    }
    std::wstring q = L"\x22";
    std::string savedAudioNameUtf8 = "";
    // Для обычного видео спрашиваем про звук, для GIF звук не извлекаем
    if (!isGifMode) {
        std::cout << "\n=========================================\n";
        std::cout << " [VIDEO DECOMPILER - AUDIO EXTRACTION]\n";
        std::cout << "=========================================\n";
        std::cout << "Extract audio track to a separate .wav file? (y/n): ";
        std::wstring answer;
        std::getline(std::wcin, answer);
        bool extractAudio = (!answer.empty() && (answer.front() == L'y' || answer.front() == L'Y' || answer.front() == L'1' || answer.front() == L'д' || answer.front() == L'Д'));
        if (extractAudio) {
            PrintCurrentTime();
            std::cout << "Extracting audio to WAV via FFmpeg...\n";
            std::wstring audioCmd = q + q + ffmpegPath + q + L" -y -loglevel error -i " + q + inputFile + q + L" -vn -acodec pcm_s16le " + q + wavOutputFile + q + q;
            int audioRet = _wsystem(audioCmd.c_str());
            if (audioRet == 0 && FileExistsW(wavOutputFile)) {
                size_t slashPos = wavOutputFile.find_last_of(L"\/");
                std::wstring justFileName = (slashPos != std::wstring::npos) ? wavOutputFile.substr(slashPos + 1) : wavOutputFile;
                savedAudioNameUtf8 = WStringToUtf8(justFileName);
                PrintCurrentTime();
                std::wcout << L"\x1B[32mAudio extracted successfully: " << justFileName << L"\x1B[0m\n";
            }
            else {
                PrintCurrentTime();
                std::cout << "\x1B[33mWarning: No audio stream found in video (or extraction failed). Writing sound(\x22\x22).\x1B[0m\n";
                if (FileExistsW(wavOutputFile)) DeleteFileW(wavOutputFile.c_str());
            }
        }
        else {
            PrintCurrentTime();
            std::cout << "Skipping audio extraction. Writing sound(\x22\x22) header.\n";
        }
    }
    std::vector<wchar_t> tempBase(MAX_PATH, 0);
    GetTempPathW(MAX_PATH, tempBase.data());
    std::wstring tempDir = std::wstring(tempBase.data()) + L"decompile_anim_" + std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(tempDir.c_str(), NULL);
    PrintCurrentTime();
    std::cout << (isGifMode ? "Splitting GIF into frames via FFmpeg...\n" : "Splitting video into frames via FFmpeg...\n");
    std::wstring framePattern = tempDir + L"/frame_%06d.bmp";
    std::wstring splitCmd = q + q + ffmpegPath + q + L" -y -loglevel error -i " + q + inputFile + q + L" " + q + framePattern + q + q;
    int splitRet = _wsystem(splitCmd.c_str());
    if (splitRet != 0) {
        RemoveDirRecursiveW(tempDir);
        StartErrorTimeout("Decompiler Error: FFmpeg failed to decode frames!");
        return;
    }
    typedef std::vector<std::wstring> WStringList;
    WStringList frameFiles;
    WIN32_FIND_DATAW findData;
    std::wstring searchMask = tempDir + L"/frame_*.bmp";
    HANDLE hFind = FindFirstFileW(searchMask.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                frameFiles.push_back(tempDir + L"/" + findData.cFileName);
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    std::sort(frameFiles.begin(), frameFiles.end());
    // Если GIF оказался однокадровым (статичным), перенаправляем в обычную декомпиляцию картинки
    if (isGifMode && frameFiles.size() == 1) {
        std::wstring singleFrame = frameFiles.front();
        ProcessImageDecompilation(singleFrame, outputFile);
        RemoveDirRecursiveW(tempDir);
        return;
    }
    if (frameFiles.size() < 2) {
        RemoveDirRecursiveW(tempDir);
        StartErrorTimeout("Decompiler Error: Animation/Video must contain at least 2 frames!");
        return;
    }
    Gdiplus::Bitmap* firstBmp = Gdiplus::Bitmap::FromFile(frameFiles.front().c_str());
    int widthX = firstBmp ? firstBmp->GetWidth() : 0;
    int heightY = firstBmp ? firstBmp->GetHeight() : 0;
    if (firstBmp) delete firstBmp;
    PrintCurrentTime();
    std::cout << "\n=========================================\n";
    std::cout << (isGifMode ? " [GIF DECOMPILER - STRUCTURE DETECTED]\n" : " [VIDEO DECOMPILER - STRUCTURE DETECTED]\n");
    std::cout << " Resolution:   " << widthX << "x" << heightY << "\n";
    std::cout << " Total Frames: " << frameFiles.size() << "\n";
    if (!isGifMode) {
        std::cout << " Audio Header: sound(\x22" << savedAudioNameUtf8 << "\x22)\n";
    }
    else {
        std::cout << " Mode:         GIF Animation (No sound header)\n";
    }
    std::cout << "=========================================\n\n";
    std::ofstream txtOut(outputFile, std::ios::binary);
    if (!txtOut.is_open()) {
        RemoveDirRecursiveW(tempDir);
        StartErrorTimeout("Write Error: Failed to create output text file.");
        return;
    }
    // В режиме GIF строка sound(...) не пишется
    if (!isGifMode) {
        txtOut << "sound(\x22" << savedAudioNameUtf8 << "\x22)\n\n";
    }
    auto animStart = std::chrono::high_resolution_clock::now();
    size_t totalFrames = frameFiles.size();
    for (size_t f = 0; f < totalFrames; f++) {
        Gdiplus::Bitmap* frameBmp = Gdiplus::Bitmap::FromFile(frameFiles.at(f).c_str());
        if (!frameBmp || frameBmp->GetLastStatus() != Gdiplus::Ok) {
            if (frameBmp) delete frameBmp;
            txtOut.close();
            RemoveDirRecursiveW(tempDir);
            StartErrorTimeout("Decompiler Error: Corrupted frame encountered during processing.");
            return;
        }
        bool isLast = (f == totalFrames - 1);
        WriteBitmapFrameToStream(frameBmp, txtOut, isLast);
        if (!isLast) txtOut << "\n";
        delete frameBmp;
        int percent = int(((f + 1) * 100) / totalFrames);
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration elapsed = now - animStart;
        std::cout << "\rDecompiling frame " << (f + 1) << "/" << totalFrames
            << " [" << percent << "%] | Elapsed: "
            << std::fixed << std::setprecision(1) << elapsed.count() << "s... ";
        std::cout.flush();
    }
    txtOut.close();
    RemoveDirRecursiveW(tempDir);
    std::cout << "\n\n";
    PrintCurrentTime();
    if (isGifMode) {
        std::wcout << L"\x1B[32mSUCCESS! GIF code saved directly as: " << outputFile << L"\x1B[0m\n\n";
    }
    else {
        std::wcout << L"\x1B[32mSUCCESS! Video code saved directly as: " << outputFile << L"\x1B[0m\n\n";
    }
    system("pause");
}
void ShowHelp() {
    std::cout << "DecompilerImageForCreationOfPictures (Universal Legendary Engine v4.0)\n";
    std::cout << "Usage image: Drag and drop ANY image onto this executable.\n";
    std::cout << "Usage GIF:   Drag and drop ANY .gif animation onto this executable (requires ffmpeg.exe).\n";
    std::cout << "Usage video: Drag and drop ANY video onto this executable (or use -f path\to\ffmpeg.exe).\n";
    std::cout << "Supported images: .png, .jpg, .jpeg, .bmp, .webp, .ico, .tiff, .svg\n";
    std::cout << "Supported GIF:    .gif (animated & static)\n";
    std::cout << "Supported videos: .mp4, .avi, .mkv, .mov, .webm, .wmv, .flv, .m4v, .mpeg, .mpg, .3gp\n";
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
    size_t dotPos = inputFile.find_last_of(L".");
    std::wstring baseName = (dotPos != std::wstring::npos) ? inputFile.substr(0, dotPos) : inputFile;
    std::wstring outputFile = baseName + L".txt";
    std::wstring wavOutputFile = baseName + L".wav";
    if (IsSupportedImageFormat(inputFile)) {
        ProcessImageDecompilation(inputFile, outputFile);
    }
    else if (IsGifFormat(inputFile)) {
        std::wstring ffmpegPath = FindOrAskFFmpeg(argc, argv);
        ProcessAnimatedDecompilation(inputFile, outputFile, wavOutputFile, ffmpegPath, true);
    }
    else if (IsSupportedVideoFormat(inputFile)) {
        std::wstring ffmpegPath = FindOrAskFFmpeg(argc, argv);
        ProcessAnimatedDecompilation(inputFile, outputFile, wavOutputFile, ffmpegPath, false);
    }
    else {
        LocalFree(argv);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        StartErrorTimeout("Error: Unsupported file format!");
        return 1;
    }
    auto globalEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration elapsed = globalEnd - globalStart;
    std::cout << "=========================================\n";
    std::cout << " Total execution time: " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds.\n";
    std::cout << "=========================================\n\n";
    LocalFree(argv);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}