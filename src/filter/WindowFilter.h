#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class WindowFilter {
public:
    WindowFilter();
    ~WindowFilter();

    static const void applyWindow(VolumeData* const volume, const int32_t windowCenter,
                                  const int32_t windowWidth, const int32_t windowOffset,
                                  const uint32_t numberOfThreads);

private:
    static const void applyWindowSliceX(VolumeData* const volume, const int32_t windowCenter,
                                        const int32_t windowWidth, const int32_t windowOffset,
                                        const uint32_t positionX);
};

} // namespace VDTK