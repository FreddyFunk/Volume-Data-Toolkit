#include <algorithm>
#include <cmath>
#include <numeric>
#include "histogram.h"

const std::vector<uint16_t> VDTK::HistogramGenerator::getHistogram(const VolumeData* const volume) {
    std::vector<uint16_t> histo(UINT16_MAX + 1, 0);

    for (const uint16_t& value : volume->getRawVolumeData()) {
        histo[value] = histo[value] + 1;
    }

    return histo;
}

const std::vector<uint16_t> VDTK::HistogramGenerator::getHistogramWidthWindowing(
    const VolumeData* const volume, WindowingFunction func, int32_t windowCenter,
    int32_t windowWidth, int32_t windowOffset) {
    switch (func) {
    case VDTK::WindowingFunction::Linear: {
        return getHistogramLinear(volume, windowCenter, windowWidth, windowOffset);
        break;
    }
    case VDTK::WindowingFunction::LinearExact: {
        return getHistogramLinearExact(volume, windowCenter, windowWidth, windowOffset);
        break;
    }
    case VDTK::WindowingFunction::Sigmoid: {
        return getHistogramSigmoid(volume, windowCenter, windowWidth, windowOffset);
        break;
    }
    default: {
        return std::vector<uint16_t>();
        break;
    }
    }
}

const std::vector<uint16_t> VDTK::HistogramGenerator::getHistogramLinear(
    const VolumeData* const volume, int32_t windowCenter, int32_t windowWidth,
    int32_t windowOffset) {
    const float windowCenterShifted = static_cast<float>(windowCenter) - 0.5f;
    const float windowWidthShifted = static_cast<float>(windowWidth) - 1.0f;
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    const float lowerBorder = windowCenterShifted - windowWidthShifted / 2.0f;
    const float upperBorder = windowCenterShifted + windowWidthShifted / 2.0f;

    constexpr float max = static_cast<float>(UINT16_MAX);

    std::vector<uint16_t> histo(UINT16_MAX + 1, 0);

    for (const uint16_t& value : volume->getRawVolumeData()) {
        const float valueAsFloat = static_cast<float>(value);

        uint16_t mappedValue = 0;

        if (valueAsFloat + windowOffsetAsFloat <= lowerBorder) {
            mappedValue = 0;
        } else if (valueAsFloat + windowOffsetAsFloat > upperBorder) {
            mappedValue = UINT16_MAX;
        } else {
            mappedValue = static_cast<uint16_t>(
                ((valueAsFloat + windowOffsetAsFloat - windowCenterShifted) / windowWidthShifted +
                 0.5f) *
                max);
        }

        histo[mappedValue] = histo[mappedValue] + 1;
    }

    return histo;
}

const std::vector<uint16_t> VDTK::HistogramGenerator::getHistogramLinearExact(
    const VolumeData* const volume, int32_t windowCenter, int32_t windowWidth,
    int32_t windowOffset) {
    const float windowCenterAsFloat = static_cast<float>(windowCenter);
    const float windowWidthAsFloat = static_cast<float>(windowWidth);
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    const float lowerBorder = windowCenterAsFloat - windowWidthAsFloat / 2.0f;
    const float upperBorder = windowCenterAsFloat + windowWidthAsFloat / 2.0f;

    constexpr float max = static_cast<float>(UINT16_MAX);

    std::vector<uint16_t> histo(UINT16_MAX + 1, 0);

    for (const uint16_t& value : volume->getRawVolumeData()) {
        uint16_t mappedValue = 0;
        const float valueAsFloat = static_cast<float>(value);

        if (valueAsFloat + windowOffset <= lowerBorder) {
            mappedValue = 0;
        } else if (valueAsFloat + windowOffset > upperBorder) {
            mappedValue = UINT16_MAX;
        } else {
            mappedValue = static_cast<uint16_t>(
                ((valueAsFloat + windowOffsetAsFloat - windowCenterAsFloat) / windowWidthAsFloat +
                 0.5f) *
                max);
        }

        histo[mappedValue] = histo[mappedValue] + 1;
    }

    return histo;
}

const std::vector<uint16_t> VDTK::HistogramGenerator::getHistogramSigmoid(
    const VolumeData* const volume, int32_t windowCenter, int32_t windowWidth,
    int32_t windowOffset) {
    constexpr float outputRange = static_cast<float>(UINT16_MAX);
    const float windowCenterAsFloat = static_cast<float>(windowCenter);
    const float windowWidthAsFloat = static_cast<float>(windowWidth);
    const float windowOffsetAsFloat = static_cast<float>(windowOffset);

    std::vector<uint16_t> histo(UINT16_MAX + 1, 0);

    for (const uint16_t& value : volume->getRawVolumeData()) {
        const float valueAsFloat = static_cast<float>(value);

        const uint16_t mappedValue = static_cast<uint16_t>(
            outputRange /
            (1.0f + std::exp(-4.0f * ((valueAsFloat + windowOffsetAsFloat - windowCenterAsFloat) /
                                      windowWidthAsFloat))));

        histo[mappedValue] = histo[mappedValue] + 1;
    }

    return histo;
}
