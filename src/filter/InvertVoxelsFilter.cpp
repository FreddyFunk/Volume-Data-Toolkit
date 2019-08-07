
#include <numeric>
#include "InvertVoxelsFilter.h"

namespace VDTK {
InvertVoxelFilter::InvertVoxelFilter() {}

InvertVoxelFilter::~InvertVoxelFilter() {}

const void InvertVoxelFilter::invertVoxelData(VolumeData& volume) {
    for (uint32_t x = 0; x < volume.getSize().getX(); x++) {
        for (uint32_t y = 0; y < volume.getSize().getY(); y++) {
            for (uint32_t z = 0; z < volume.getSize().getZ(); z++) {
                volume.setVoxelValue(x, y, z, UINT16_MAX - volume.getVoxelValue(x, y, z));
            }
        }
    }
}
} // namespace VDTK
