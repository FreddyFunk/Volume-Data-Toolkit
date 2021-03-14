#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class WindowFilter {
public:
    WindowFilter();
    ~WindowFilter();

    static void applyWindow(VolumeData* const volume, const WindowingFunction func,
                            const int32_t windowCenter, const int32_t windowWidth,
                            const int32_t windowOffset, const std::size_t numberOfThreads);

    static uint16_t getValueWithWindowingFunctionLinear(const uint16_t value,
                                                        const int32_t windowCenter,
                                                        const int32_t windowWidth,
                                                        const int32_t windowOffset);

    static uint16_t getValueWithWindowingFunctionLinearExact(const uint16_t value,
                                                        const int32_t windowCenter,
                                                        const int32_t windowWidth,
                                                        const int32_t windowOffset);

    // implemented as discribed here:
    // http://dicom.nema.org/medical/dicom/2017a/output/chtml/part03/sect_C.11.2.html
    static uint16_t getValueWithWindowingFunctionSigmoid(const uint16_t value,
                                                        const int32_t windowCenter,
                                                        const int32_t windowWidth,
                                                        const int32_t windowOffset);

private:
    static void applyWindowSliceX(VolumeData* const volume, WindowingFunction func,
                                  const int32_t windowCenter, const int32_t windowWidth,
                                  const int32_t windowOffset, const std::size_t positionX);
};

} // namespace VDTK