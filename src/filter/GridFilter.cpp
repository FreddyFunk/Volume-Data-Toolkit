
#include <threadpool/ThreadPool.h>

#include "GridFilter.h"

namespace VDTK {
GridFilter::GridFilter() {}

GridFilter::~GridFilter() {}

void GridFilter::applyFilter(VolumeData* const volume, const VDTK::FilterKernel& filter,
                             const std::size_t numberOfThreads) {
    VolumeData filteredVolume = *volume;

    {
        // create own scope to use destructor of thread pool (wait for all task to
        // finish)
        ThreadPool threadPool(numberOfThreads);

        // scale each x slice using seperate threads
        // we still use tri-XY and not bi-XY interpolation
        for (std::size_t x = 0; x < volume->getSize().getX(); x++) {
            threadPool.enqueue(&GridFilter::applyFilterToSliceX, volume, &filteredVolume, filter,
                               x);
        }
    }

    *volume = filteredVolume;
}

void GridFilter::applyFilterToSliceX(const VolumeData* const volume,
                                     VolumeData* const filteredVolume,
                                     const VDTK::FilterKernel& filter,
                                     const std::size_t positionX) {
    for (std::size_t y = 0; y < volume->getSize().getY(); y++) {
        for (std::size_t z = 0; z < volume->getSize().getZ(); z++) {
            filteredVolume->setVoxelValue(positionX, y, z,
                                          getNewVoxelValue(volume, positionX, y, z, filter));
        }
    }
}

const uint16_t GridFilter::filterGridAverage(
    const std::vector<std::vector<std::vector<double>>>& filterGridValues) {
    double average = 0;

    for (std::size_t x = 0; x < filterGridValues.size(); x++) {
        for (std::size_t y = 0; y < filterGridValues.data()->size(); y++) {
            for (std::size_t z = 0; z < filterGridValues.data()->data()->size(); z++) {
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

const uint16_t GridFilter::getNewVoxelValue(const VolumeData* const volume, const std::size_t x,
                                            const std::size_t y, const std::size_t z,
                                            const VDTK::FilterKernel& filter) {
    // stores values of filter grid when applied to volume data
    std::vector<std::vector<std::vector<double>>> filterGridValues(
        filter.getKernelSize(),
        std::vector<std::vector<double>>(filter.getKernelSize(),
                                         std::vector<double>(filter.getKernelSize(), 0.0)));

    // iterate over whole volume grid
    for (std::size_t filterX = 0; filterX < filter.getKernelSize(); filterX++) {
        for (std::size_t filterY = 0; filterY < filter.getKernelSize(); filterY++) {
            for (std::size_t filterZ = 0; filterZ < filter.getKernelSize(); filterZ++) {
                const int64_t filterGridOffset = static_cast<int64_t>(filter.getKernelSize() / 2);
                // calculate volume position from current filter grid position
                const int64_t volumePositionX =
                    static_cast<int64_t>(filterX) - filterGridOffset + x;
                const int64_t volumePositionY =
                    static_cast<int64_t>(filterY) - filterGridOffset + y;
                const int64_t volumePositionZ =
                    static_cast<int64_t>(filterZ) - filterGridOffset + z;

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
                        static_cast<double>(volume->getVoxelValue(static_cast<std::size_t>(x),
                                                                  static_cast<std::size_t>(y),
                                                                  static_cast<std::size_t>(z))) *
                        filter.getFilterGrid()[filterX][filterY][filterZ];
                } else {
                    filterGridValues[filterX][filterY][filterZ] =
                        static_cast<double>(
                            volume->getVoxelValue(static_cast<std::size_t>(volumePositionX),
                                                  static_cast<std::size_t>(volumePositionY),
                                                  static_cast<std::size_t>(volumePositionZ))) *
                        filter.getFilterGrid()[filterX][filterY][filterZ];
                }
            }
        }
    }

    return filterGridAverage(filterGridValues);
}
} // namespace VDTK