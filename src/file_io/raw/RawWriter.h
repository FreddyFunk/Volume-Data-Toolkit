#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class RawWriter {
public:
    RawWriter();
    ~RawWriter();

    static const bool write(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                            const VolumeData& volume);

private:
    static const std::vector<uint8_t> convertTo8Bit(const std::vector<uint16_t>& data,
                                                    const uint64_t voxelCount);
};
} // namespace VDTK
