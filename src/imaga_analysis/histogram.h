#pragma once

#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class HistogramGenerator {
public:
    static const std::vector<uint16_t> getHistogram(const VolumeData* const volume);
    static const std::vector<uint16_t> getHistogramWidthWindowing(const VolumeData* const volume,
                                                                  WindowingFunction func,
                                                                  int32_t windowCenter,
                                                                  int32_t windowWidth,
                                                                  int32_t windowOffset);

private:
};

} // namespace VDTK