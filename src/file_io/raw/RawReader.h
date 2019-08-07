#pragma once

#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class RawReader {
public:
    RawReader();
    ~RawReader();

    static bool read(VolumeData* const volumeData, const std::filesystem::path& filePath,
                     const uint8_t bitsPerVoxel, const VDTK::VolumeSize volumeSize,
                     const VDTK::VolumeSpacing volumeSpacing);

private:
    static const std::vector<uint16_t> convertTo16Bit(std::vector<char>& data,
                                                      const uint8_t bitsPerVoxel,
                                                      const uint64_t voxelCount);
};
} // namespace VDTK
