#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class EndianConverter {
public:
    EndianConverter();
    ~EndianConverter();

    static const void flipEndianness(VolumeData* const volume) {
        for (std::size_t x = 0; x < volume->getSize().getX(); x++) {
            for (std::size_t y = 0; y < volume->getSize().getY(); y++) {
                for (std::size_t z = 0; z < volume->getSize().getZ(); z++) {
                    const uint16_t originalValue = volume->getVoxelValue(x, y, z);
                    const uint16_t convertedValue =
                        (originalValue & 0xff) << 8 | (originalValue & 0xff00) >> 8;
                    volume->setVoxelValue(x, y, z, convertedValue);
                }
            }
        }
    }

private:
};

EndianConverter::EndianConverter() {}

EndianConverter::~EndianConverter() {}
} // namespace VDTK