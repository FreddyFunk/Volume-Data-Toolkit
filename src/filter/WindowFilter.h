#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class WindowFilter {
public:
    WindowFilter();
    ~WindowFilter();

    static void applyWindow(VolumeData* const volume, const int32_t windowCenter,
                            const int32_t windowWidth, const int32_t windowOffset,
                            const std::size_t numberOfThreads);

private:
    static void applyWindowSliceX(VolumeData* const volume, const int32_t windowCenter,
                                  const int32_t windowWidth, const int32_t windowOffset,
                                  const std::size_t positionX);
};

} // namespace VDTK