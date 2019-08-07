
#include <threadpool/ThreadPool.h>

#include "GridFilter.h"

namespace VDTK {
GridFilter::GridFilter() {}

GridFilter::~GridFilter() {}

void GridFilter::applyFilter(VolumeData* const volume, const VDTK::FilterKernel& filter,
                             const uint32_t numberOfThreads) {
    VolumeData filteredVolume = *volume;

    {
        // create own scope to use destructor of thread pool (wait for all task to
        // finish)
        ThreadPool threadPool(numberOfThreads);

        // scale each x slice using seperate threads
        // we still use tri-XY and not bi-XY interpolation
        for (uint32_t x = 0; x < volume->getSize().getX(); x++) {
            threadPool.enqueue(&GridFilter::applyFilterToSliceX, volume, &filteredVolume, filter,
                               x);
        }
    }

    *volume = filteredVolume;
}

void GridFilter::applyFilterToSliceX(const VolumeData* const volume,
                                     VolumeData* const filteredVolume,
                                     const VDTK::FilterKernel& filter, const uint32_t positionX) {
    for (uint32_t y = 0; y < volume->getSize().getY(); y++) {
        for (uint32_t z = 0; z < volume->getSize().getZ(); z++) {
            filteredVolume->setVoxelValue(positionX, y, z,
                                          getNewVoxelValue(volume, positionX, y, z, filter));
        }
    }
}

const uint16_t GridFilter::filterGridAverage(
    const std::vector<std::vector<std::vector<double>>>& filterGridValues) {
    double average = 0;

    for (uint32_t x = 0; x < filterGridValues.size(); x++) {
        for (uint32_t y = 0; y < filterGridValues.data()->size(); y++) {
            for (uint32_t z = 0; z < filterGridValues.data()->data()->size(); z++) {
                average += filterGridValues[x][y][z];
            }
        }
    }

    if (average < 0.0) {
        return 0;
    } else if (average > UINT16_MAX) {
        return UINT16_MAX;
    } else {
        return static_cast<uint16_t>(average);
    }
}

const uint16_t GridFilter::getNewVoxelValue(const VolumeData* const volume, const uint32_t x,
                                            const uint32_t y, const uint32_t z,
                                            const VDTK::FilterKernel& filter) {
    // stores values of filter grid when applied to volume data
    std::vector<std::vector<std::vector<double>>> filterGridValues(
        filter.getKernelSize(),
        std::vector<std::vector<double>>(filter.getKernelSize(),
                                         std::vector<double>(filter.getKernelSize(), 0.0)));

    // iterate over whole volume grid
    for (uint32_t filterX = 0; filterX < filter.getKernelSize(); filterX++) {
        for (uint32_t filterY = 0; filterY < filter.getKernelSize(); filterY++) {
            for (uint32_t filterZ = 0; filterZ < filter.getKernelSize(); filterZ++) {
                const int32_t filterGridOffset = filter.getKernelSize() / 2;
                // calculate volume position from current filter grid position
                const int32_t volumePositionX = x + (filterX - filterGridOffset);
                const int32_t volumePositionY = y + (filterY - filterGridOffset);
                const int32_t volumePositionZ = z + (filterZ - filterGridOffset);

                // check if current filter position is a valid volume position
                const bool outsideBorderX =
                    (volumePositionX < 0 ||
                     volumePositionX >= static_cast<int32_t>(volume->getSize().getX()));
                const bool outsideBorderY =
                    (volumePositionY < 0 ||
                     volumePositionY >= static_cast<int32_t>(volume->getSize().getY()));
                const bool outsideBorderZ =
                    (volumePositionZ < 0 ||
                     volumePositionZ >= static_cast<int32_t>(volume->getSize().getZ()));

                // if current filter position is not a valid volume position extrapolate
                // with nearest neighbor
                if (outsideBorderX || outsideBorderY || outsideBorderZ) {
                    // Pixels outside the volume borders must be extrapolated
                    filterGridValues[filterX][filterY][filterZ] =
                        static_cast<double>(volume->getVoxelValue(x, y, z)) *
                        filter.getFilterGrid()[filterX][filterY][filterZ];
                } else {
                    filterGridValues[filterX][filterY][filterZ] =
                        static_cast<double>(volume->getVoxelValue(volumePositionX, volumePositionY,
                                                                  volumePositionZ)) *
                        filter.getFilterGrid()[filterX][filterY][filterZ];
                }
            }
        }
    }

    return filterGridAverage(filterGridValues);
}
} // namespace VDTK