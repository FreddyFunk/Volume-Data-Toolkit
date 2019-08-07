
#include <algorithm>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

#include <libbmpread/bmpread.h>

#ifdef __cplusplus
}
#endif

#include "BitmapImporter.h"

namespace VDTK {
const std::vector<std::string> BitmapImporter::m_validExtenions = {".bmp", ".BMP"};

BitmapImporter::BitmapImporter() {}

BitmapImporter::~BitmapImporter() {}

const bool BitmapImporter::importMonochrom(VolumeData* const volumeData,
                                           const std::filesystem::path& directoryPath,
                                           const VolumeAxis axis,
                                           const VDTK::VolumeSpacing spacing) {
    return import(volumeData, directoryPath, axis, spacing, PixelMode::RGBMonochrom);
}

const bool BitmapImporter::importColor(VolumeData* const volumeData,
                                       const std::filesystem::path& directoryPath,
                                       const VolumeAxis axis, const VDTK::VolumeSpacing spacing) {
    return import(volumeData, directoryPath, axis, spacing, PixelMode::RGBColor);
}

inline const uint16_t BitmapImporter::pixelRGBColorToVoxel(const std::vector<char> pixel) {
    // convert pixel to unsigned int (technically 32 bit, but interpreted as 24
    // bit)
    std::vector<char> rawPixel24Bit(4);
    rawPixel24Bit[0] = pixel[0];
    rawPixel24Bit[0] = pixel[1];
    rawPixel24Bit[0] = pixel[2];
    rawPixel24Bit[0] = 0;
    uint32_t pixel24Bit = 0;
    std::memcpy(rawPixel24Bit.data(), &pixel24Bit, rawPixel24Bit.size());

    // convert 24 bit to 16 bit
    pixel24Bit /= 3;
    pixel24Bit *= 2;

    return static_cast<uint16_t>(pixel24Bit);
}

inline const uint16_t BitmapImporter::pixelRGBMonochromToVoxel(const std::vector<char> pixel) {
    // calculate averange of 3 8-bit channels (RGB)
    uint32_t rawPixel = 0;
    rawPixel += static_cast<uint32_t>(pixel[0]);
    rawPixel += static_cast<uint32_t>(pixel[1]);
    rawPixel += static_cast<uint32_t>(pixel[2]);
    rawPixel /= 3;

    // upscale 8 bit values to 16 bit
    rawPixel *= UINT8_MAX;

    return static_cast<uint16_t>(rawPixel);
}

const uint32_t BitmapImporter::getNumberOfBitmapsInDirectory(
    const std::filesystem::path& directoryPath) {
    uint32_t numberOfBitmaps = 0;
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
        const bool isFile = std::filesystem::is_regular_file(directoryEntry);
        const bool isBitmap =
            std::find(m_validExtenions.begin(), m_validExtenions.end(),
                      directoryEntry.path().extension()) != m_validExtenions.end();
        if (isFile && isBitmap) {
            numberOfBitmaps++;
        }
    }

    return numberOfBitmaps;
}

const VDTK::VolumeSize BitmapImporter::calculateVolumeSize(
    const std::filesystem::path& directoryPath, const VolumeAxis axis) {
    VDTK::VolumeSize size = VDTK::VolumeSize(0, 0, 0);

    // get the path of the first found bitmap file in the directory
    std::filesystem::path firstBitmapInDirectory;
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
        const bool isFile = std::filesystem::is_regular_file(directoryEntry);
        const bool isBitmap =
            std::find(m_validExtenions.begin(), m_validExtenions.end(),
                      directoryEntry.path().extension()) != m_validExtenions.end();
        if (isFile && isBitmap) {
            firstBitmapInDirectory = directoryEntry.path();
            break;
        }
    }

    // load bitmap from path of the first found bitmap file in directory
    bmpread_t bitmap;
    bmpread(firstBitmapInDirectory.string().c_str(), 0, &bitmap);

    // interpret bitmap width and hight depending on the axis
    switch (axis) {
    case VolumeAxis::YZAxis: {
        size.setX(getNumberOfBitmapsInDirectory(directoryPath));
        size.setY(bitmap.width);
        size.setZ(bitmap.height);
        break;
    }
    case VolumeAxis::XZAxis: {
        size.setX(bitmap.width);
        size.setY(getNumberOfBitmapsInDirectory(directoryPath));
        size.setZ(bitmap.height);
        break;
    }
    case VolumeAxis::XYAxis: {
        size.setX(bitmap.height);
        size.setY(bitmap.width);
        size.setZ(getNumberOfBitmapsInDirectory(directoryPath));
        break;
    }
    default: { break; }
    }

    // empty ram from bitmap
    bmpread_free(&bitmap);

    return size;
}

// TODO: needs some refactoring
const bool BitmapImporter::import(VolumeData* const volumeData,
                                  const std::filesystem::path& directoryPath, const VolumeAxis axis,
                                  const VDTK::VolumeSpacing spacing, const PixelMode pixelMode) {
    if (!std::filesystem::exists(directoryPath)) {
        // Directory does not exist
        return false;
    }

    if (getNumberOfBitmapsInDirectory(directoryPath) == 0) {
        // No Bitmap files in directory
        return false;
    }

    VolumeData volume(calculateVolumeSize(directoryPath, axis), spacing);

    uint32_t sliceIndex = 0;
    for (const auto& directoryEntry : std::filesystem::directory_iterator(directoryPath)) {
        const bool isFile = std::filesystem::is_regular_file(directoryEntry);
        const bool isBitmap =
            std::find(m_validExtenions.begin(), m_validExtenions.end(),
                      directoryEntry.path().extension()) != m_validExtenions.end();
        if (isFile && isBitmap) {
            bmpread_t bitmap;
            bmpread(directoryEntry.path().string().c_str(), 0, &bitmap);
            VolumeSlice slice(axis, bitmap.width, bitmap.height);

            // fill slice with bitmap pixel
            for (uint32_t width = 0; width < static_cast<uint32_t>(bitmap.width); width++) {
                for (uint32_t height = 0; height < static_cast<uint32_t>(bitmap.height); height++) {
                    // By default, each pixel spans three bytes: the red, green, and blue
                    // color components in that order

                    std::vector<char> rawPixelData(3);
                    rawPixelData[0] = bitmap.data[3 * ((height + (bitmap.height * width)) + 0)];
                    rawPixelData[1] = bitmap.data[3 * ((height + (bitmap.height * width)) + 1)];
                    rawPixelData[2] = bitmap.data[3 * ((height + (bitmap.height * width)) + 2)];

                    uint16_t voxelValue = 0;
                    switch (pixelMode) {
                    case PixelMode::RGBColor: {
                        voxelValue = pixelRGBColorToVoxel(rawPixelData);
                        break;
                    }
                    case PixelMode::RGBMonochrom:
                    default: {
                        voxelValue = pixelRGBMonochromToVoxel(rawPixelData);
                        break;
                    }
                    }
                    slice.setPixel(width, height, voxelValue);
                }
            }
            // empty ram from bitmap
            bmpread_free(&bitmap);

            volume.setSlice(slice, sliceIndex);

            sliceIndex++;
        }
    }

    *volumeData = volume;
    return true;
}
} // namespace VDTK