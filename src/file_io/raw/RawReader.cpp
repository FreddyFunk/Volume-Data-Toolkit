
#include <algorithm>
#include "RawReader.h"

namespace VDTK {
RawReader::RawReader() {}

RawReader::~RawReader() {}

bool RawReader::read(VolumeData* const volumeData, const std::filesystem::path& filePath,
                     const uint8_t bitsPerVoxel, const VDTK::VolumeSize volumeSize,
                     const VDTK::VolumeSpacing volumeSpacing) {
    if (!std::filesystem::exists(filePath)) {
        // Input Raw file does not exist
        return false;
    }

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    std::size_t fileSize = 0;

    try {
        fileSize = std::filesystem::file_size(filePath);
    } catch (std::filesystem::filesystem_error& e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    if (!file.is_open()) {
        // unable to open file
        return false;
    }

    VolumeData volume(volumeSize, volumeSpacing);

    // fileSize is given in bytes, but voxel data can be 12 bit for example.
    // Therefore we have to calculate the total bits of the file before and then
    // convert then to bytes by dividing with 8
    if (fileSize != (volume.getVoxelCount() * bitsPerVoxel) / 8) {
        // Volume dimensions and filesize do not fit together
        return false;
    }

    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        // Unable to read file
        return false;
    }

    volume.setRawVolumeData(convertTo16Bit(buffer, bitsPerVoxel, volume.getVoxelCount()));
    *volumeData = volume;
    return true;
}

const std::vector<uint16_t> RawReader::convertTo16Bit(std::vector<char>& data,
                                                      const uint8_t bitsPerVoxel,
                                                      const uint64_t voxelCount) {
    std::vector<uint16_t> convertedData(voxelCount);

    switch (bitsPerVoxel) {
    case 8: {
        std::transform(
            data.begin(), data.end(), convertedData.begin(),
            [](char in) -> const uint16_t { return static_cast<uint16_t>(in) * UINT8_MAX; });
        break;
    }
    case 16: {
        std::copy(reinterpret_cast<const uint16_t* const>(&data[0]),
                  reinterpret_cast<const uint16_t* const>(&data[0]) + voxelCount,
                  convertedData.begin());
        break;
    }
    default:
        break;
    }

    return convertedData;
}

} // namespace VDTK