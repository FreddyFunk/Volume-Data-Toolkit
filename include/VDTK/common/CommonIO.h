#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdint.h>

namespace VDTK::FileIOCommon {
// needed generating file names (bitmap or dicom for example)
// could be more optimized with a look up table
static const uint32_t numberOfDigits(uint32_t number) {
    uint32_t digits = 0;
    if (!number) {
        digits++; // to write a zero one digit is still needed
    }
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

// checks if a character in a string is a digit
static const bool isADigit(const std::string& s, const size_t index) {
    if (index >= s.size()) {
        return false;
    }
    const size_t number = static_cast<size_t>(s.at(index)) - 48;
    if (number >= 0 && number <= 9) {
        return true;
    }
    return false;
}

static const void renameIndexedFilesInDirectory(const std::filesystem::path& directoryPath,
                                                const std::string& sliceName = "_IMG",
                                                const std::string& fileExtension = {}) {
    std::vector<std::filesystem::path> filePaths;
    // get all file names in directory
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
        if (std::filesystem::is_regular_file(directoryEntry)) {
            filePaths.push_back(directoryEntry);
        }
    }
    // rename each file in diretory
    for (const auto& filePath : filePaths) {
        const std::string fileName = filePath.filename().string();
        std::string indexAsString = {};

        size_t fileIndexStart = 0;
        while (!isADigit(fileName, fileIndexStart)) {
            fileIndexStart++;
        }

        size_t fileIndexEnd = fileIndexStart;
        while (isADigit(fileName, fileIndexEnd)) {
            fileIndexEnd++;
        }

        for (size_t i = fileIndexStart; i < fileIndexEnd; i++) {
            indexAsString.push_back(fileName.at(i));
        }

        const uint32_t indexAsInt = std::stoi(indexAsString);

        std::string indexWithPrefixZeros = {};
        const uint32_t numberOfPrefixZeros =
            numberOfDigits(static_cast<uint32_t>(filePaths.size())) - numberOfDigits(indexAsInt);
        for (size_t i = 0; i < numberOfPrefixZeros; i++) {
            indexWithPrefixZeros.append("0");
        }
        indexWithPrefixZeros.append(indexAsString);

        std::filesystem::path newFilePath = filePath;
        newFilePath.replace_extension(fileExtension);
        newFilePath.replace_filename(indexWithPrefixZeros + sliceName);

        std::filesystem::rename(filePath, newFilePath);
    }
}
} // namespace VDTK::FileIOCommon
