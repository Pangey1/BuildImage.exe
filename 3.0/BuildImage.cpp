#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <windows.h>

struct RGB {
    uint8_t r, g, b;
};

RGB GetColorFromPalette(int value) {
    RGB color = { 0, 0, 0 };
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

int ParseToken(std::string token) {
    token.erase(0, token.find_first_not_of(" \t\r\n"));
    token.erase(token.find_last_not_of(" \t\r\n") + 1);
    if (token.empty()) return -1;

    if (token.length() >= 3 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
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
    std::cout << "Compiler 'Creation of Pictures' (Batch Engine Mode v2.6)\n";
    std::cout << "Usage single: BuildImage.exe -i image.txt -o picture.bmp\n";
    std::cout << "Usage batch:  BuildImage.exe -i img1.txt,img2.txt -o pic1.bmp,pic2.bmp\n";
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

bool CompileSingleFile(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream fileCheck(inputFile);
    if (!fileCheck.good()) {
        std::cerr << "\x1B[31mCompiler Error: Source file not found! (" << inputFile << ")\x1B[0m\n";
        return false;
    }
    fileCheck.close();

    std::ifstream file(inputFile);
    std::vector<std::vector<int>> imageMatrix;
    std::string line;
    int lineNumber = 0;
    bool hasErrors = false;
    bool endOfFileMarkerReached = false;
    int countRowsY = 0;
    int countColumnsX = 0;

    while (std::getline(file, line)) {
        lineNumber++;
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        while (!line.empty() && std::isspace(line.back())) line.pop_back();
        if (line.empty()) continue;

        if (endOfFileMarkerReached) {
            std::cerr << "\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Code discovered after closing dot '.'!\x1B[0m\n";
            hasErrors = true;
            break;
        }

        if (line.back() == '.') {
            endOfFileMarkerReached = true;
            line.pop_back();
        }

        for (char ch : line) {
            if (!std::isdigit(ch) && ch != ',' && ch != ' ' && ch != '\t' && ch != '\r' &&
                ch != 'x' && ch != 'X' && !std::isxdigit(ch)) {
                std::cerr << "\x1B[31mSyntax Error [" << inputFile << " -> Line " << lineNumber << "]: Illegal character '" << ch << "'!\x1B[0m\n";
                hasErrors = true;
                break;
            }
        }

        if (hasErrors) break;

        std::vector<int> currentRow;
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
            if ((int)currentRow.size() > countColumnsX) countColumnsX = (int)currentRow.size();
        }
    }
    file.close();

    if (hasErrors) return false;

    if (!endOfFileMarkerReached) {
        std::cerr << "\x1B[31mSyntax Error [" << inputFile << "]: Missing final dot '.' marker!\x1B[0m\n";
        return false;
    }

    for (int i = 0; i < countRowsY; i++) {
        if ((int)imageMatrix[i].size() != countColumnsX) {
            std::cerr << "\x1B[31mLinker Error [" << inputFile << "]: Row " << i + 1 << " columns mismatch (Got: " << imageMatrix[i].size() << ", Expected: " << countColumnsX << ")!\x1B[0m\n";
            return false;
        }
    }

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
            RGB color = GetColorFromPalette(imageMatrix[y][x]);
            rowBuffer[x * 3 + 0] = color.b;
            rowBuffer[x * 3 + 1] = color.g;
            rowBuffer[x * 3 + 2] = color.r;
        }
        out.write(reinterpret_cast<const char*>(rowBuffer.data()), rowSize);
    }
    out.close();

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

    if (argc < 5) {
        ShowHelp();
        return 1;
    }

    std::string inputArg = "";
    std::string outputArg = "";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) inputArg = argv[++i];
        else if (arg == "-o" && i + 1 < argc) outputArg = argv[++i];
    }

    if (inputArg.empty() || outputArg.empty()) {
        std::cerr << "\x1B[31mCompiler Error: Input or output specifications missing.\x1B[0m\n";
        return 1;
    }

    std::vector<std::string> inputFiles = SplitByComma(inputArg);
    std::vector<std::string> outputFiles = SplitByComma(outputArg);

    if (inputFiles.size() != outputFiles.size()) {
        std::cerr << "\x1B[31mBatch Error: Files count mismatch!\x1B[0m\n";
        std::cerr << "\x1B[33mYou specified " << inputFiles.size() << " input files but " << outputFiles.size() << " output names.\x1B[0m\n";
        return 1;
    }

    std::cout << "[Batch Mode] Starting compilation for " << inputFiles.size() << " files...\n\n";

    int successCount = 0;
    for (size_t i = 0; i < inputFiles.size(); i++) {
        int percent = static_cast<int>(((i + 1) * 100) / inputFiles.size());
        std::cout << "Processing: [" << percent << "%] ";
        if (CompileSingleFile(inputFiles[i], outputFiles[i])) {
            successCount++;
        }
    }

    std::cout << "\n\x1B[32m" << "BATCH COMPLETE: " << successCount << " of " << inputFiles.size() << " pictures compiled successfully." << "\x1B[0m\n";
    return 0;
}
