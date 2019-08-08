
#include "BinarySliceImporter.h"

namespace VDTK {

BinarySliceImporter::BinarySliceImporter() {}

BinarySliceImporter::~BinarySliceImporter() {}

const bool BinarySliceImporter::import(VolumeData* const volumeData,
                                       const std::filesystem::path& directoryPath,
                                       const uint8_t bitsPerVoxel, const VolumeAxis axis,
                                       const VDTK::VolumeSize size,
                                       const VDTK::VolumeSpacing spacing) {
    if (!std::filesystem::exists(directoryPath)) {
        // Directory does not exist
        return false;
    }

    VolumeData volume(size, spacing);

    std::size_t sliceWidth = 0;
    std::size_t sliceHeight = 0;

    switch (axis) {
    case VolumeAxis::XZAxis: {
        sliceWidth = size.getX();
        sliceHeight = size.getZ();
        break;
    }
    case VolumeAxis::XYAxis: {
        sliceWidth = size.getX();
        sliceHeight = size.getY();
        break;
    }
    case VolumeAxis::YZAxis:
    default: {
        sliceWidth = size.getY();
        sliceHeight = size.getZ();
        break;
    }
    }

    std::size_t sliceIndex = 0;
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
        // skip subdirectories
        if (std::filesystem::is_regular_file(directoryEntry)) {
            VolumeSlice slice(VDTK::VolumeAxis::XYAxis, 0, 0);

            if (!loadSlice(&slice, directoryEntry, bitsPerVoxel, axis, sliceWidth, sliceHeight)) {
                return false;
            }
            volume.setSlice(slice, sliceIndex);
            sliceIndex++;
        }
    }

    *volumeData = volume;
    return true;
}

const bool BinarySliceImporter::loadSlice(VolumeSlice* const volumeSlice,
                                          const std::filesystem::path& filePath,
                                          const uint8_t bitsPerVoxel, const VolumeAxis axis,
                                          const std::size_t width, const std::size_t height) {
    std::size_t fileSize = 0;
    try {
        fileSize = std::filesystem::file_size(filePath);
    } catch (std::filesystem::filesystem_error& e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        // unable to open file
        return false;
    }

    // fileSize is given in bytes, but voxel data can be 12 bit for example.
    // Therefore we have to calculate the total bits of the file before and then
    // convert then to bytes by dividing with 8
    if (fileSize != ((width * height) * bitsPerVoxel) / 8) {
        // slice dimensions and filesize do not fit together
        return false;
    }

    std::vector<char> buffer(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), fileSize);

    *volumeSlice =
        VolumeSlice(axis, convertTo16Bit(&buffer, bitsPerVoxel, width * height), width, height);
    return true;
}

const std::vector<uint16_t> BinarySliceImporter::convertTo16Bit(std::vector<char>* buffer,
                                                                const uint8_t bitsPerPixel,
                                                                const uint64_t pixelCount) {
    std::vector<uint16_t> convertedData(pixelCount);

    switch (bitsPerPixel) {
    case 8: {
        const uint8_t* const ptr = reinterpret_cast<uint8_t*>(buffer->data());
        for (uint64_t index = 0; index < pixelCount; index++) {
            convertedData[index] = static_cast<uint16_t>(ptr[index]) * UINT8_MAX;
        }
        break;
    }
    case 16: {
        const uint16_t* const ptr = reinterpret_cast<uint16_t*>(buffer->data());
        for (uint64_t index = 0; index < pixelCount; index++) {
            convertedData[index] = ptr[index];
        }
        break;
    }
    default:
        break;
    }

    return convertedData;
}
} // namespace VDTK