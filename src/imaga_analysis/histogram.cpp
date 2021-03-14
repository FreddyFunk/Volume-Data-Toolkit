#include <algorithm>
#include <cmath>
#include <numeric>
#include "histogram.h"
#include "../filter/WindowFilter.h"

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
    std::vector<uint16_t> histo(UINT16_MAX + 1, 0);

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
        return std::vector<uint16_t>();
        break;
    }
    }

    for (const uint16_t& value : volume->getRawVolumeData()) {

        histo[apply(value, windowCenter, windowWidth, windowOffset)] += 1;
    }
    return histo;
}
