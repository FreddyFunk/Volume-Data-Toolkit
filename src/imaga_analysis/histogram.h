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
    static const std::vector<uint16_t> getHistogramLinear(const VolumeData* const volume,
                                                          int32_t windowCenter, int32_t windowWidth,
                                                          int32_t windowOffset);
    static const std::vector<uint16_t> getHistogramLinearExact(const VolumeData* const volume,
                                                               int32_t windowCenter,
                                                               int32_t windowWidth,
                                                               int32_t windowOffset);
    // implemented as discribed here:
    // http://dicom.nema.org/medical/dicom/2017a/output/chtml/part03/sect_C.11.2.html
    static const std::vector<uint16_t> getHistogramSigmoid(const VolumeData* const volume,
                                                           int32_t windowCenter,
                                                           int32_t windowWidth,
                                                           int32_t windowOffset);
};

} // namespace VDTK