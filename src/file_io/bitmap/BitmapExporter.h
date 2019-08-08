#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class BitmapExporter {
public:
    static const bool writeColor(const std::filesystem::path& directoryPath,
                                 const VolumeData& volume);
    static const bool writeMonochrom(const std::filesystem::path& directoryPath,
                                     const VolumeData& volume);

private:
    enum class PixelMode { RGBColor, RGBMonochrom };

    // returns colorful pixel. No information loss (16 bit into 24 bit RBG pixel)
    static inline const std::vector<char> convertToRGBColor(const uint16_t voxelValue);
    // returns monochrom pixel. INFORMATION LOSS (16 bit voxel value into 8 bit
    // RBG channel)
    static inline const std::vector<char> convertToRGBMonochrom(const uint16_t voxelValue);

    static const void writeAxis(const std::filesystem::path& directoryPath,
                                const VolumeData& volume, VolumeAxis axis,
                                const PixelMode pixelMode);
    static const void writeAxisAtIndex(const std::vector<char> (*convertToPixel)(uint16_t),
                                       const std::filesystem::path& directoryPath,
                                       const VolumeData& volume, VolumeAxis axis,
                                       const std::size_t sliceIndex);
};
} // namespace VDTK