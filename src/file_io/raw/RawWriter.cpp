
#include "RawWriter.h"

namespace VDTK {
RawWriter::RawWriter() {}

RawWriter::~RawWriter() {}

const std::vector<uint8_t> RawWriter::convertTo8Bit(const std::vector<uint16_t>& data,
                                                    const uint64_t voxelCount) {
    std::vector<uint8_t> convertedData(voxelCount);

    // volume data in VDTK is always stored as 16 bit
    const uint16_t* ptr = data.data();
    for (uint64_t index = 0; index < voxelCount; index++) {
        convertedData[index] = static_cast<uint8_t>(ptr[index] / UINT8_MAX);
    }

    return convertedData;
}

bool RawWriter::write(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel,
                      const VolumeData& volume) {
    std::ofstream file = std::ofstream(filePath, std::ios::out | std::ios::binary);

    if (file.fail()) {
        // unable to create file
        return false;
    }

    switch (bitsPerVoxel) {
    case 8: {
        const std::vector<uint8_t> volume8Bit =
            convertTo8Bit(volume.getRawVolumeData(), volume.getVoxelCount());
        const char* rawVolumeData = reinterpret_cast<const char*>(volume8Bit.data());
        file.write(rawVolumeData, volume.getVoxelCount() * sizeof(uint8_t));
        break;
    }
    case 16: {
        const char* rawVolumeData = reinterpret_cast<const char*>(volume.getRawVolumeData().data());
        file.write(rawVolumeData, volume.getVoxelCount() * sizeof(uint16_t));
        break;
    }
    default:
        break;
    }

    file.close();
    return true;
}
} // namespace VDTK