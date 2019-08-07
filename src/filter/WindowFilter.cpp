
#include <threadpool/ThreadPool.h>

#include "WindowFilter.h"

namespace VDTK {
WindowFilter::WindowFilter() {}

WindowFilter::~WindowFilter() {}

const void WindowFilter::applyWindow(VolumeData* const volume, const int32_t windowCenter,
                                     const int32_t windowWidth, const int32_t windowOffset,
                                     const uint32_t numberOfThreads) {
    {
        // create own scope to use destructor of thread pool (wait for all task to
        // finish)
        ThreadPool threadPool(numberOfThreads);

        // scale each x slice using seperate threads
        // we still use tri-XY and not bi-XY interpolation
        for (uint32_t x = 0; x < volume->getSize().getX(); x++) {
            threadPool.enqueue(WindowFilter::applyWindowSliceX, volume, windowCenter, windowWidth,
                               windowOffset, x);
        }
    }
}

const void WindowFilter::applyWindowSliceX(VolumeData* const volume, const int32_t windowCenter,
                                           const int32_t windowWidth, const int32_t windowOffset,
                                           const uint32_t positionX) {
    double level = windowCenter;
    double width = windowWidth;
    double offset = windowOffset;
    double minimum = 0;
    double maximum = UINT16_MAX;

    for (uint32_t y = 0; y < volume->getSize().getY(); y++) {
        for (uint32_t z = 0; z < volume->getSize().getZ(); z++) {
            if (volume->getVoxelValue(positionX, y, z) + offset <=
                level - 0.5 - ((width - 1.0) / 2.0)) {
                volume->setVoxelValue(positionX, y, z, static_cast<uint16_t>(minimum));

            } else if (volume->getVoxelValue(positionX, y, z) + offset >
                       level - 0.5 + ((width - 1.0) / 2.0)) {
                volume->setVoxelValue(positionX, y, z, static_cast<uint16_t>(maximum));
            } else {
                volume->setVoxelValue(
                    positionX, y, z,
                    static_cast<uint16_t>(
                        (((volume->getVoxelValue(positionX, y, z) + offset) - (level - 0.5)) /
                             (width - 1.0) +
                         0.5) *
                            (maximum - minimum) +
                        minimum));
            }
        }
    }
}
} // namespace VDTK