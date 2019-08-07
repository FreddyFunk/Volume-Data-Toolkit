#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class BitmapImporter {
public:
    BitmapImporter();
    ~BitmapImporter();

    static const bool importMonochrom(VolumeData* const volumeData,
                                      const std::filesystem::path& directoryPath,
                                      const VolumeAxis axis, const VDTK::VolumeSpacing spacing);
    static const bool importColor(VolumeData* const volumeData,
                                  const std::filesystem::path& directoryPath, const VolumeAxis axis,
                                  const VDTK::VolumeSpacing spacing);

private:
    enum class PixelMode { RGBColor, RGBMonochrom };

    // POSSIBLE INFORMATION LOSS (24 bit RGB pixel to 16 bit voxel value)
    static inline const uint16_t pixelRGBColorToVoxel(const std::vector<char> pixel);
    // if bitmap is monochrom pixel only got 8 bit. Gets converted to 16 bit voxel
    // value
    static inline const uint16_t pixelRGBMonochromToVoxel(const std::vector<char> pixel);

    static const uint32_t getNumberOfBitmapsInDirectory(const std::filesystem::path& directoryPath);
    static const VDTK::VolumeSize calculateVolumeSize(const std::filesystem::path& directoryPath,
                                                      const VolumeAxis axis);
    static const bool import(VolumeData* const volumeData,
                             const std::filesystem::path& directoryPath, const VolumeAxis axis,
                             const VDTK::VolumeSpacing spacing, const PixelMode pixelMode);

    static const std::vector<std::string> m_validExtenions;
};
} // namespace VDTK
