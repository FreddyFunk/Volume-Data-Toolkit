
#include <threadpool/ThreadPool.h>

#include "WindowFilter.h"

namespace VDTK {
WindowFilter::WindowFilter() {}

WindowFilter::~WindowFilter() {}

void WindowFilter::applyWindow(VolumeData* const volume, const WindowingFunction func,
                               const int32_t windowCenter, const int32_t windowWidth,
                               const int32_t windowOffset, const std::size_t numberOfThreads) {
    {
        // create own scope to use destructor of thread pool (wait for all task to
        // finish)
        ThreadPool threadPool(numberOfThreads);

        // scale each x slice using seperate threads
        // we still use tri-XY and not bi-XY interpolation
        for (std::size_t x = 0; x < volume->getSize().getX(); x++) {
            threadPool.enqueue(WindowFilter::applyWindowSliceX, volume, func, windowCenter,
                               windowWidth, windowOffset, x);
        }
    }
}

uint16_t WindowFilter::getValueWithWindowingFunctionLinear(const uint16_t value,
                                                           const int32_t windowCenter,
                                                           const int32_t windowWidth,
                                                           const int32_t windowOffset) {
    const float windowCenterShifted = static_cast<float>(windowCenter) - 0.5f;
    const float windowWidthShifted = static_cast<float>(windowWidth) - 1.0f;
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    const float lowerBorder = windowCenterShifted - windowWidthShifted / 2.0f;
    const float upperBorder = windowCenterShifted + windowWidthShifted / 2.0f;

    constexpr float max = static_cast<float>(UINT16_MAX);

    const float valueAsFloat = static_cast<float>(value);

    if (valueAsFloat + windowOffsetAsFloat <= lowerBorder) {
        return 0;
    } else if (valueAsFloat + windowOffsetAsFloat > upperBorder) {
        return UINT16_MAX;
    } else {
        return static_cast<uint16_t>(
            ((valueAsFloat + windowOffsetAsFloat - windowCenterShifted) / windowWidthShifted +
             0.5f) *
            max);
    }
}

uint16_t WindowFilter::getValueWithWindowingFunctionLinearExact(const uint16_t value,
                                                                const int32_t windowCenter,
                                                                const int32_t windowWidth,
                                                                const int32_t windowOffset) {
    const float windowCenterAsFloat = static_cast<float>(windowCenter);
    const float windowWidthAsFloat = static_cast<float>(windowWidth);
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    const float lowerBorder = windowCenterAsFloat - windowWidthAsFloat / 2.0f;
    const float upperBorder = windowCenterAsFloat + windowWidthAsFloat / 2.0f;

    constexpr float max = static_cast<float>(UINT16_MAX);

    const float valueAsFloat = static_cast<float>(value);

    if (valueAsFloat + windowOffset <= lowerBorder) {
        return 0;
    } else if (valueAsFloat + windowOffset > upperBorder) {
        return UINT16_MAX;
    } else {
        return static_cast<uint16_t>(
            ((valueAsFloat + windowOffsetAsFloat - windowCenterAsFloat) / windowWidthAsFloat +
             0.5f) *
            max);
    }
}

uint16_t WindowFilter::getValueWithWindowingFunctionSigmoid(const uint16_t value,
                                                            const int32_t windowCenter,
                                                            const int32_t windowWidth,
                                                            const int32_t windowOffset) {
    constexpr float outputRange = static_cast<float>(UINT16_MAX);
    const float windowCenterAsFloat = static_cast<float>(windowCenter);
    const float windowWidthAsFloat = static_cast<float>(windowWidth);
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    const float valueAsFloat = static_cast<float>(value);

    return static_cast<uint16_t>(
        outputRange /
        (1.0f + std::exp(-4.0f * ((valueAsFloat + windowOffsetAsFloat - windowCenterAsFloat) /
                                  windowWidthAsFloat))));
}

void WindowFilter::applyWindowSliceX(VolumeData* const volume, const WindowingFunction func,
                                     const int32_t windowCenter, const int32_t windowWidth,
                                     const int32_t windowOffset, const std::size_t positionX) {
    const auto functionLinear = &WindowFilter::getValueWithWindowingFunctionLinear;
    const auto functionLinearExact = &WindowFilter::getValueWithWindowingFunctionLinearExact;
    const auto functionSigmoid = &WindowFilter::getValueWithWindowingFunctionSigmoid;

    auto apply = functionLinear;

    switch (func) {
    case VDTK::WindowingFunction::Linear: {
        apply = functionLinear;
        break;
    }
    case VDTK::WindowingFunction::LinearExact: {
        apply = functionLinearExact;
        break;
    }
    case VDTK::WindowingFunction::Sigmoid: {
        apply = functionSigmoid;
        break;
    }
    default: {
        break;
    }
    }

    for (std::size_t y = 0; y < volume->getSize().getY(); y++) {
        for (std::size_t z = 0; z < volume->getSize().getZ(); z++) {
            volume->setVoxelValue(positionX, y, z,
                                  apply(volume->getVoxelValue(positionX, y, z), windowCenter,
                                        windowWidth, windowOffset));
        }
    }
}
} // namespace VDTK