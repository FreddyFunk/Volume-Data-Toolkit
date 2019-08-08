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
static std::size_t numberOfDigits(std::size_t number) {
    std::size_t digits = 0;
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
static bool isADigit(const std::string& s, const std::size_t index) {
    if (index >= s.size()) {
        return false;
    }
    const std::size_t number = static_cast<std::size_t>(s.at(index)) - 48;
    if (number >= 0 && number <= 9) {
        return true;
    }
    return false;
}

static void renameIndexedFilesInDirectory(const std::filesystem::path& directoryPath,
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

        std::size_t fileIndexStart = 0;
        while (!isADigit(fileName, fileIndexStart)) {
            fileIndexStart++;
        }

        std::size_t fileIndexEnd = fileIndexStart;
        while (isADigit(fileName, fileIndexEnd)) {
            fileIndexEnd++;
        }

        for (std::size_t i = fileIndexStart; i < fileIndexEnd; i++) {
            indexAsString.push_back(fileName.at(i));
        }

        const std::size_t indexAsInt = std::stoi(indexAsString);

        std::string indexWithPrefixZeros = {};
        const std::size_t numberOfPrefixZeros =
            numberOfDigits(filePaths.size()) - numberOfDigits(indexAsInt);
        for (std::size_t i = 0; i < numberOfPrefixZeros; i++) {
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
