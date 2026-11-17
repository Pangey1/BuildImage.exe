#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <windows.h>

namespace fs = std::filesystem;

typedef std::vector<int> IntRow;
typedef std::vector<IntRow> IntMatrix;

struct RGB {
    uint8_t r, g, b;
};

RGB GetColorFromPalette(int value) {
    RGB color = { 0, 0, 0 };
    if (value < 0) value = 0;
    if (value > 200) value = 200;

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

std::string Trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

int ParseToken(std::string token) {
    token = Trim(token);
    if (token.empty()) return -1;

    if (token.length() >= 3 && token.at(0) == '0' && (token.at(1) == 'x' || token.at(1) == 'X')) {
        try {
            unsigned int hexVal;
            std::stringstream ss;
            ss << std::hex << token.substr(2);
            ss >> hexVal;
            return static_cast<int>(hexVal);
        }
        catch (...) { return -1; }
    }

    try {
        return std::stoi(token);
    }
    catch (...) { return -1; }
}

void ShowHelp() {
    std::cout << "====================================================================\n";
    std::cout << " Compiler 'Creation of Pictures, GIFs & Videos' (Engine Mode v4.0)\n";
    std::cout << "====================================================================\n\n";
    std::cout << "1. HELP:\n";
    std::cout << "   BuildImage.exe -h help\n\n";
    std::cout << "2. COMPILE STATIC IMAGE (.bmp):\n";
    std::cout << "   Command: BuildImage.exe -o image.txt -b picture.bmp\n";
    std::cout << "   Syntax:  Numbers 0..200 (dec or 0x hex) separated by commas, ending with '.'\n\n";
    std::cout << "3. COMPILE GIF ANIMATION (.gif):\n";
    std::cout << "   Command: BuildImage.exe -g anim.txt -f ffmpeg.exe -o anim.gif [-fps 60]\n";
    std::cout << "   Syntax:  No sound() header. Frames separated by '};', last frame ends with '.'\n\n";
    std::cout << "4. COMPILE VIDEO (.mp4, .avi, .mkv, .mov, etc.):\n";
    std::cout << "   Command: BuildImage.exe -o movie.txt -f ffmpeg.exe -b video.mp4 [-fps 60]\n";
    std::cout << "   Syntax:  Starts with sound(\"audio.mp3\") or sound(\"\"), frames separated by '};', ends with '.'\n";
    std::cout << "====================================================================\n";
}

auto SplitByComma(const std::string& s) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = Trim(item);
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

bool WriteBMP(const std::string& outputFile, const IntMatrix& imageMatrix) {
    int countRowsY = static_cast<int>(imageMatrix.size());
    int countColumnsX = static_cast<int>(imageMatrix.front().size());

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "\x1B[31mWrite Error: Failed to open output file: " << outputFile << "\x1B[0m\n";
        return false;
    }

    uint32_t rowSize = (countColumnsX * 3 + 3) & ~3;
    uint32_t fileSize = 54 + rowSize * countRowsY;

    uint8_t header[] = {
        'B', 'M', static_cast<uint8_t>(fileSize), static_cast<uint8_t>(fileSize >> 8), static_cast<uint8_t>(fileSize >> 16), static_cast<uint8_t>(fileSize >> 24),
        0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
        static_cast<uint8_t>(countColumnsX), static_cast<uint8_t>(countColumnsX >> 8), static_cast<uint8_t>(countColumnsX >> 16), static_cast<uint8_t>(countColumnsX >> 24),
        static_cast<uint8_t>(-countRowsY), static_cast<uint8_t>(-countRowsY >> 8), static_cast<uint8_t>(-countRowsY >> 16), static_cast<uint8_t>(-countRowsY >> 24),
        1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    out.write(reinterpret_cast<const char*>(header), 54);

    for (int y = 0; y < countRowsY; y++) {
        std::vector<uint8_t> rowBuffer(rowSize, 0);
        for (int x = 0; x < countColumnsX; x++) {
            RGB color = GetColorFromPalette(imageMatrix.at(y).at(x));
            rowBuffer.at(x * 3 + 0) = color.b;
            rowBuffer.at(x * 3 + 1) = color.g;
            rowBuffer.at(x * 3 + 2) = color.r;
        }
        out.write(reinterpret_cast<const char*>(rowBuffer.data()), rowSize);
    }
    out.close();
    return true;
}

bool ParseSoundHeader(const std::string& line, std::string& outSoundPath, std::string& errorMsg) {
    std::string lowerLine = ToLower(line);
    if (lowerLine.rfind("sound", 0) != 0) {
        errorMsg = "Video file must start with sound(\"file.ext\") or sound(\"\")!";
        return false;
    }

    size_t openParen = line.find('(');
    size_t closeParen = line.rfind(')');
    if (openParen == std::string::npos || closeParen == std::string::npos || closeParen < openParen) {
        errorMsg = "Invalid sound syntax! Expected sound(\x22...\x22) or sound('').";
        return false;
    }

    std::string afterParen = Trim(line.substr(closeParen + 1));
    if (!afterParen.empty() && afterParen != ";") {
        errorMsg = "Unexpected characters after sound(...) declaration!";
        return false;
    }

    std::string inner = Trim(line.substr(openParen + 1, closeParen - openParen - 1));
    if (inner.length() < 2) {
        errorMsg = "Sound argument must be enclosed in quotes \x22\x22 or ''!";
        return false;
    }

    char quoteChar = inner.front();
    if ((quoteChar != '"' && quoteChar != '\'') || inner.back() != quoteChar) {
        errorMsg = "Sound path must be wrapped in matching quotes (\x22...\x22 or '...')!";
        return false;
    }

    outSoundPath = inner.substr(1, inner.length() - 2);
    if (!outSoundPath.empty()) {
        fs::path p(outSoundPath);
        if (!p.has_extension()) {
            errorMsg = "Sound file must include an extension (e.g., .mp3, .wav)!";
            return false;
        }
        if (!fs::exists(p)) {
            errorMsg = "Sound file not found on disk: " + outSoundPath;
            return false;
        }
    }
    return true;
}

bool CompileAnimatedStream(const std::string& inputFile, const std::string& outputFile, const std::string& ffmpegPath, int fps, bool isGifMode) {
    if (ffmpegPath.empty()) {
        std::cerr << "\x1B[31mCompiler Error: FFmpeg path not specified! Use -f <path_to_ffmpeg.exe>\x1B[0m\n";
        return false;
    }
    if (!fs::exists(ffmpegPath)) {
        std::cerr << "\x1B[31mCompiler Error: FFmpeg executable not found at: " << ffmpegPath << "\x1B[0m\n";
        return false;
    }

    std::ifstream file(inputFile);
    if (!file.good()) {
        std::cerr << "\x1B[31mCompiler Error: Source file not found! (" << inputFile << ")\x1B[0m\n";
        return false;
    }

    fs::path tempDir = fs::temp_directory_path() / ("buildimg_stream_" + std::to_string(GetCurrentProcessId()));
    fs::create_directories(tempDir);

    IntMatrix currentFrame;
    std::string soundPath = "";
    bool soundParsed = isGifMode;
    bool endOfFileMarkerReached = false;
    std::string line;
    int lineNumber = 0;

    size_t frameCount = 0;
    size_t expectedHeight = 0;
    size_t expectedWidth = 0;
    constexpr size_t BUF_SIZE = 64;

    std::cout << "\n";

    while (std::getline(file, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        line = Trim(line);
        if (line.empty()) continue;

        if (endOfFileMarkerReached) {
            fs::remove_all(tempDir);
            std::cerr << "\n\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Code discovered after closing dot '.'!\x1B[0m\n";
            return false;
        }

        if (!soundParsed) {
            std::string err;
            if (!ParseSoundHeader(line, soundPath, err)) {
                fs::remove_all(tempDir);
                std::cerr << "\n\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: " << err << "\x1B[0m\n";
                return false;
            }
            soundParsed = true;
            continue;
        }

        bool isFrameEnd = false;
        if (line.size() >= 2 && line.substr(line.size() - 2) == "};") {
            isFrameEnd = true;
            line.erase(line.size() - 2);
            line = Trim(line);
        }
        else if (line.back() == '.') {
            endOfFileMarkerReached = true;
            line.pop_back();
            line = Trim(line);
        }

        if (!line.empty()) {
            for (char ch : line) {
                unsigned char uch = static_cast<unsigned char>(ch);
                if (!std::isdigit(uch) && ch != ',' && ch != ' ' && ch != '\t' && ch != '\r' &&
                    ch != 'x' && ch != 'X' && !std::isxdigit(uch)) {
                    fs::remove_all(tempDir);
                    std::cerr << "\n\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Illegal character '" << ch << "'!\x1B[0m\n";
                    return false;
                }
            }
            IntRow currentRow;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                int val = ParseToken(token);
                if (val != -1) {
                    if (val < 0 || val > 200) {
                        fs::remove_all(tempDir);
                        std::cerr << "\n\x1B[31mValue Error [" << inputFile << " -> Line " << lineNumber << "]: Value " << val << " out of range (0..200)!\x1B[0m\n";
                        return false;
                    }
                    currentRow.push_back(val);
                }
            }
            if (!currentRow.empty()) {
                currentFrame.push_back(currentRow);
            }
        }
        if (isFrameEnd || endOfFileMarkerReached) {
            if (currentFrame.empty()) {
                fs::remove_all(tempDir);
                std::cerr << "\n\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Empty frame detected!\x1B[0m\n";
                return false;
            }
            if (frameCount == 0) {
                expectedHeight = currentFrame.size();
                expectedWidth = currentFrame.front().size();
            }
            if (currentFrame.size() != expectedHeight) {
                fs::remove_all(tempDir);
                std::cerr << "\n\x1B[31mLinker Error [" << inputFile << "]: Frame " << frameCount + 1 << " height mismatch (Got: " << currentFrame.size() << ", Expected: " << expectedHeight << ")!\x1B[0m\n";
                return false;
            }
            for (size_t r = 0; r < currentFrame.size(); r++) {
                if (currentFrame.at(r).size() != expectedWidth) {
                    fs::remove_all(tempDir);
                    std::cerr << "\n\x1B[31mLinker Error [" << inputFile << "]: Frame " << frameCount + 1 << ", Row " << r + 1 << " width mismatch (Got: " << currentFrame.at(r).size() << ", Expected: " << expectedWidth << ")!\x1B[0m\n";
                    return false;
                }
            }
            char frameName[BUF_SIZE];
            snprintf(frameName, BUF_SIZE, "frame_%06zu.bmp", frameCount);
            fs::path framePath = tempDir / frameName;
            if (!WriteBMP(framePath.string(), currentFrame)) {
                fs::remove_all(tempDir);
                return false;
            }
            frameCount++;
            currentFrame.clear();
            if (frameCount % 10 == 0 || endOfFileMarkerReached) {
                std::cout << "\rRendered frames: " << frameCount << "... ";
                std::cout.flush();
            }
        }
    }
    file.close();
    std::cout << "\n";
    if (!soundParsed) {
        fs::remove_all(tempDir);
        std::cerr << "\x1B[31mSyntax Error [" << inputFile << "]: Empty source file!\x1B[0m\n";
        return false;
    }
    if (!endOfFileMarkerReached) {
        fs::remove_all(tempDir);
        std::cerr << "\x1B[31mSyntax Error [" << inputFile << "]: Missing final dot '.' marker at the end of file!\x1B[0m\n";
        return false;
    }
    if (frameCount < 2) {
        fs::remove_all(tempDir);
        std::cerr << "\x1B[31mAnimation Error [" << inputFile << "]: At least 2 frames separated by '};' are required (Got: " << frameCount << ")!\x1B[0m\n";
        return false;
    }
    std::cout << (isGifMode ? "Encoding GIF via FFmpeg...\n" : "Encoding video via FFmpeg...\n");
    std::string inputPattern = (tempDir / "frame_%06d.bmp").string();
    std::string absFfmpeg = fs::absolute(ffmpegPath).string();
    std::string q = "\x22";
    std::stringstream cmd;
    cmd << q << q << absFfmpeg << q << " -y -loglevel error -framerate " << fps << " -i " << q << inputPattern << q << " ";
    if (isGifMode) {
        cmd << "-vf " << q << "split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" << q << " -loop 0 ";
    }
    else {
        if (!soundPath.empty()) {
            cmd << "-i " << q << soundPath << q << " ";
        }
        cmd << "-vf " << q << "pad=ceil(iw/2)*2:ceil(ih/2)*2" << q << " -c:v libx264 -pix_fmt yuv420p ";
        if (!soundPath.empty()) {
            cmd << "-c:a aac -shortest ";
        }
    }
    cmd << q << outputFile << q << q;
    int ret = std::system(cmd.str().c_str());
    fs::remove_all(tempDir);
    if (ret != 0) {
        std::cerr << "\x1B[31mEncoder Error: FFmpeg failed with exit code " << ret << "!\x1B[0m\n";
        return false;
    }
    double durationSec = double(frameCount) / fps;
    std::cout << " -> " << inputFile << " compiled into " << (isGifMode ? "GIF " : "video ") << outputFile
        << " (" << expectedWidth << "x" << expectedHeight << ", "
        << frameCount << " frames @ " << fps << " FPS, " << durationSec << "s";
    if (!isGifMode) {
        std::cout << (soundPath.empty() ? ", no audio" : ", audio: " + soundPath);
    }
    std::cout << ")\n";
    return true;
}
bool CompileSingleImage(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream file(inputFile);
    if (!file.good()) {
        std::cerr << "\x1B[31mCompiler Error: Source file not found! (" << inputFile << ")\x1B[0m\n";
        return false;
    }

    IntMatrix imageMatrix;
    std::string line;
    int lineNumber = 0;
    bool endOfFileMarkerReached = false;
    int countRowsY = 0;
    int countColumnsX = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        while (!line.empty() && std::isspace((unsigned char)(line.back()))) line.pop_back();
        if (line.empty()) continue;

        if (endOfFileMarkerReached) {
            std::cerr << "\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Code discovered after closing dot '.'!\x1B[0m\n";
            return false;
        }

        if (line.back() == '.') {
            endOfFileMarkerReached = true;
            line.pop_back();
        }

        for (char ch : line) {
            unsigned char uch = (unsigned char)(ch);
            if (!std::isdigit(uch) && ch != ',' && ch != ' ' && ch != '\t' && ch != '\r' &&
                ch != 'x' && ch != 'X' && !std::isxdigit(uch)) {
                std::cerr << "\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Illegal character '" << ch << "'!\x1B[0m\n";
                return false;
            }
        }

        IntRow currentRow;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            int val = ParseToken(token);
            if (val != -1) {
                currentRow.push_back(val);
            }
        }

        if (!currentRow.empty()) {
            imageMatrix.push_back(currentRow);
            countRowsY++;
            if (int(currentRow.size()) > countColumnsX) countColumnsX = int(currentRow.size());
        }
    }
    file.close();

    if (!endOfFileMarkerReached) {
        std::cerr << "\x1B[31mSyntax Error [" << inputFile << "]: Missing final dot '.' marker!\x1B[0m\n";
        return false;
    }

    for (int i = 0; i < countRowsY; i++) {
        if (int(imageMatrix.at(i).size()) != countColumnsX) {
            std::cerr << "\x1B[31mLinker Error [" << inputFile << "]: Row " << i + 1 << " columns mismatch (Got: " << imageMatrix.at(i).size() << ", Expected: " << countColumnsX << ")!\x1B[0m\n";
            return false;
        }
    }

    if (!WriteBMP(outputFile, imageMatrix)) return false;

    std::cout << " -> " << inputFile << " successfully compiled into " << outputFile << " (" << countColumnsX << "x" << countRowsY << ")\n";
    return true;
}
int main(int argc, char* argv[]) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    if (argc < 2) {
        ShowHelp();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help" || arg == "help") {
            ShowHelp();
            return 0;
        }
    }

    if (argc < 5) {
        std::cerr << "\x1B[31mCompiler Error: Not enough arguments! Run 'BuildImage.exe -h help' for instructions.\x1B[0m\n";
        return 1;
    }

    std::string gifInputArg = "";
    std::string originInputArg = "";
    std::string buildOutputArg = "";
    std::string ffmpegArg = "";
    int fps = 60;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-g" && i + 1 < argc) gifInputArg = argv[++i];
        else if (arg == "-o" && i + 1 < argc) originInputArg = argv[++i];
        else if (arg == "-b" && i + 1 < argc) buildOutputArg = argv[++i];
        else if (arg == "-f" && i + 1 < argc) ffmpegArg = argv[++i];
        else if (arg == "-fps" && i + 1 < argc) {
            try {
                fps = std::stoi(argv[++i]);
                if (fps <= 0) fps = 60;
            }
            catch (...) { fps = 60; }
        }
    }

    if (!gifInputArg.empty()) {
        if (originInputArg.empty()) {
            std::cerr << "\x1B[31mCompiler Error: Output GIF file (-o) missing! Usage: BuildImage.exe -g input.txt -f ffmpeg.exe -o output.gif\x1B[0m\n";
            return 1;
        }

        auto inputFiles = SplitByComma(gifInputArg);
        auto outputFiles = SplitByComma(originInputArg);

        if (inputFiles.size() != outputFiles.size()) {
            std::cerr << "\x1B[31mBatch Error: Files count mismatch!\x1B[0m\n";
            return 1;
        }

        std::cout << "[GIF Batch Mode] Starting compilation for " << inputFiles.size() << " files...\n\n";
        int successCount = 0;
        for (size_t i = 0; i < inputFiles.size(); i++) {
            int percent = int(((i + 1) * 100) / inputFiles.size());
            std::cout << "Processing: [" << percent << "%] ";
            if (CompileAnimatedStream(inputFiles.at(i), outputFiles.at(i), ffmpegArg, fps, true)) {
                successCount++;
            }
        }
        std::cout << "\n\x1B[32m" << "BATCH COMPLETE: " << successCount << " of " << inputFiles.size() << " GIFs compiled successfully." << "\x1B[0m\n";
        return 0;
    }

    if (originInputArg.empty() || buildOutputArg.empty()) {
        std::cerr << "\x1B[31mCompiler Error: Input (-o) or output (-b) specifications missing.\x1B[0m\n";
        return 1;
    }

    auto inputFiles = SplitByComma(originInputArg);
    auto outputFiles = SplitByComma(buildOutputArg);

    if (inputFiles.size() != outputFiles.size()) {
        std::cerr << "\x1B[31mBatch Error: Files count mismatch!\x1B[0m\n";
        std::cerr << "\x1B[33mYou specified " << inputFiles.size() << " input files (-o) but " << outputFiles.size() << " output names (-b).\x1B[0m\n";
        return 1;
    }

    std::cout << "[Batch Mode] Starting compilation for " << inputFiles.size() << " files...\n\n";

    int successCount = 0;
    for (size_t i = 0; i < inputFiles.size(); i++) {
        int percent = int(((i + 1) * 100) / inputFiles.size());
        std::cout << "Processing: [" << percent << "%] ";

        std::string ext = ToLower(fs::path(outputFiles.at(i)).extension().string());
        bool ok = false;
        if (ext == ".bmp") {
            ok = CompileSingleImage(inputFiles.at(i), outputFiles.at(i));
        }
        else {
            ok = CompileAnimatedStream(inputFiles.at(i), outputFiles.at(i), ffmpegArg, fps, false);
        }

        if (ok) successCount++;
    }

    std::cout << "\n\x1B[32m" << "BATCH COMPLETE: " << successCount << " of " << inputFiles.size() << " targets compiled successfully." << "\x1B[0m\n";
    return 0;
}