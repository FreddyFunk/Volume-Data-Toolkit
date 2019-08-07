#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class GridFilter {
public:
    GridFilter();
    ~GridFilter();

    static void applyFilter(VolumeData* const volume, const VDTK::FilterKernel& filter,
                            const uint32_t numberOfThreads);

private:
    static void applyFilterToSliceX(const VolumeData* const volume,
                                    VolumeData* const filteredVolume,
                                    const VDTK::FilterKernel& filter, const uint32_t positionX);
    static const uint16_t filterGridAverage(
        const std::vector<std::vector<std::vector<double>>>& filterGridValues);
    static const uint16_t getNewVoxelValue(const VolumeData* const volume, const uint32_t x,
                                           const uint32_t y, const uint32_t z,
                                           const VDTK::FilterKernel& filter);
};
} // namespace VDTK