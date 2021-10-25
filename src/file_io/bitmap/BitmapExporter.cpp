
#include <string>
#include <vector>

#include <bitmap/bitmap_image.hpp>
#include "../include/VDTK/common/CommonIO.h"

#include "BitmapExporter.h"

namespace VDTK {
bool BitmapExporter::writeColor(const std::filesystem::path& directoryPath,
                                const VolumeData& volume) {
    writeAxis(directoryPath, volume, VolumeAxis::YZAxis, PixelMode::RGBColor);
    writeAxis(directoryPath, volume, VolumeAxis::XZAxis, PixelMode::RGBColor);
    writeAxis(directoryPath, volume, VolumeAxis::XYAxis, PixelMode::RGBColor);

    // TODO: better error handling
    return true;
}

bool BitmapExporter::writeMonochrom(const std::filesystem::path& directoryPath,
                                    const VolumeData& volume) {
    writeAxis(directoryPath, volume, VolumeAxis::YZAxis, PixelMode::RGBMonochrom);
    writeAxis(directoryPath, volume, VolumeAxis::XZAxis, PixelMode::RGBMonochrom);
    writeAxis(directoryPath, volume, VolumeAxis::XYAxis, PixelMode::RGBMonochrom);

    // TODO: better error handling
    return true;
}

inline const std::vector<char> BitmapExporter::convertToRGBColor(const uint16_t voxelValue) {
    // convert 16 bit to 24 bit
    const uint32_t tmpRawPixelData = (static_cast<uint32_t>(voxelValue) * 3) / 2;
    const char* rawPixelData = reinterpret_cast<const char*>(&tmpRawPixelData);

    // parse 24 bit ISO value into an RGB 555 pixel
    std::vector<char> pixel(3);
    pixel[0] = rawPixelData[0];
    pixel[1] = rawPixelData[1];
    pixel[2] = rawPixelData[2];

    return pixel;
}

inline const std::vector<char> BitmapExporter::convertToRGBMonochrom(const uint16_t voxelValue) {
    // convert 16 bit to 8 bit
    const uint8_t rawPixelData = static_cast<uint8_t>(voxelValue / UINT8_MAX);

    // each channel gets same value (monochrom)
    std::vector<char> pixel(3);
    pixel[0] = rawPixelData;
    pixel[1] = rawPixelData;
    pixel[2] = rawPixelData;

    return pixel;
}

void BitmapExporter::writeAxis(const std::filesystem::path& directoryPath, const VolumeData& volume,
                               VolumeAxis axis, const PixelMode pixelMode) {
    // Function pointer for the selected pixel mode
    const std::vector<char> (*convertToPixel)(uint16_t) = nullptr;
    switch (pixelMode) {
    case PixelMode::RGBColor: {
        convertToPixel = &BitmapExporter::convertToRGBColor;
        break;
    }
    case PixelMode::RGBMonochrom: {
        convertToPixel = &BitmapExporter::convertToRGBMonochrom;
        break;
    }
    default: {
        convertToPixel = &BitmapExporter::convertToRGBMonochrom;
        break;
    }
    }

    std::size_t numberOfSlices = 0;
    switch (axis) {
    case VolumeAxis::YZAxis: {
        numberOfSlices = volume.getSize().getX();
        break;
    }
    case VolumeAxis::XZAxis: {
        numberOfSlices = volume.getSize().getY();
        break;
    }
    case VolumeAxis::XYAxis: {
        numberOfSlices = volume.getSize().getZ();
        break;
    }
    default: { break; }
    }

    // proccess each slice of the current axis
    for (std::size_t sliceIndex = 0; sliceIndex < numberOfSlices; sliceIndex++) {
        writeAxisAtIndex(convertToPixel, directoryPath, volume, axis, sliceIndex);
    }
}

void BitmapExporter::writeAxisAtIndex(const std::vector<char> (*convertToPixel)(uint16_t),
                                      const std::filesystem::path& directoryPath,
                                      const VolumeData& volume, VolumeAxis axis,
                                      const std::size_t sliceIndex) {
    std::string fileName = {};

    // TODO: move to own function
    switch (axis) {
    case VolumeAxis::YZAxis: {
        fileName = "X_";
        break;
    }
    case VolumeAxis::XZAxis: {
        fileName = "Y_";
        break;
    }
    case VolumeAxis::XYAxis: {
        fileName = "Z_";
        break;
    }
    default: { break; }
    }

    VolumeSlice slice = volume.getSlice(axis, sliceIndex);

    bitmap_image image(static_cast<int>(slice.getWidth()), static_cast<int>(slice.getHeigth()));

    // convert the slice into an bitmap with selected pixel representation
    for (std::size_t x = 0; x < slice.getWidth(); x++) {
        for (std::size_t y = 0; y < slice.getHeigth(); y++) {
            // parse 24 bit ISO value into an RGB 555 pixel
            const std::vector<char> rawPixelData = convertToPixel(slice.getPixel(x, y));
            rgb_t pixel;
            pixel.red = rawPixelData[0];
            pixel.green = rawPixelData[1];
            pixel.blue = rawPixelData[2];

            image.set_pixel(static_cast<unsigned int>(x), static_cast<unsigned int>(y), pixel);
        }
    }

    // save image to selected directory
    // create dictionary if not exists
    if (!std::filesystem::exists(directoryPath)) {
        std::filesystem::create_directories(directoryPath);
    }
    // generate file name
    std::filesystem::path imageFilePath = directoryPath;
    // if path is a directory path, generic file name gets generated
    if (std::filesystem::is_directory(imageFilePath)) {
        const std::size_t numberOfNeededZeros =
            VDTK::FileIOCommon::numberOfDigits(slice.getWidth()) -
            VDTK::FileIOCommon::numberOfDigits(sliceIndex);

        fileName.append(std::string(numberOfNeededZeros, '0'));
        fileName.append(std::to_string(sliceIndex));

        imageFilePath = imageFilePath / fileName;
        imageFilePath.replace_extension("bmp");
    }

    image.save_image(imageFilePath.string());
}
} // namespace VDTK