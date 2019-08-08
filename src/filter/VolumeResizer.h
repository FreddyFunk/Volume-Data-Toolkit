#pragma once
#include <array>

#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class VolumeResizer {
public:
    VolumeResizer();
    ~VolumeResizer();

    static const void scaleNearestNeighbor(VolumeData* const volume,
                                           const VDTK::Vector3D<float>& scale,
                                           const std::size_t numberOfThreads);
    static const void scaleTrilinear(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                     const std::size_t numberOfThreads);
    static const void scaleTricubic(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                    const std::size_t numberOfThreads);

private:
    enum class InterpolationMode { Nearest, Trilinear, Tricubic };

    static const void scaleSliceX(const VolumeData* const volume, VolumeData* volumeScaled,
                                  const VDTK::Vector3D<float> scale,
                                  const InterpolationMode interpolationMode,
                                  const float scaledPositionX);

    static const void scaleVolume(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                  const InterpolationMode interpolationMode,
                                  const std::size_t numberOfThreads);

    // Nearest neighbor interpolation
    static const float getNearestNeigborValue(const VolumeData* const volume,
                                              const VDTK::Vector3D<float>& originalPosition);

    // Linear interpolation
    static const inline float interpolateLinear(const std::array<float, 2>& values, const float x);
    static const inline float interpolateBilinear(
        const std::array<std::array<float, 2>, 2>& valuePlane, const float x, const float y);
    static const inline float interpolateTrilinear(
        const std::array<std::array<std::array<float, 2>, 2>, 2>& valueGrid, const float x,
        const float y, const float z);
    static const float getTrilinearInterpolatedValue(const VolumeData* const volume,
                                                     const VDTK::Vector3D<float>& originalSize,
                                                     const VDTK::Vector3D<float>& originalPosition);

    // Cubic interpolation
    static const inline float interpolateCubic(const std::array<float, 4>& values, const float x);
    static const inline float interpolateBicubic(
        const std::array<std::array<float, 4>, 4>& valuePlane, const float x, const float y);
    static const inline float interpolateTricubic(
        const std::array<std::array<std::array<float, 4>, 4>, 4>& valueGrid, const float x,
        const float y, const float z);
    static const float getTricubicInterpolatedValue(const VolumeData* const volume,
                                                    const VDTK::Vector3D<float>& originalSize,
                                                    const VDTK::Vector3D<float>& originalPosition);
};

} // namespace VDTK