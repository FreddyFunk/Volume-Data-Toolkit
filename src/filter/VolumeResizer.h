#pragma once
#include <array>

#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class VolumeResizer {
public:
    VolumeResizer();
    ~VolumeResizer();

    static void scaleNearestNeighbor(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                     const std::size_t numberOfThreads);
    static void scaleTrilinear(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                               const std::size_t numberOfThreads);
    static void scaleTricubic(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                              const std::size_t numberOfThreads);

private:
    enum class InterpolationMode { Nearest, Trilinear, Tricubic };

    static void scaleSliceX(const VolumeData* const volume, VolumeData* volumeScaled,
                            const VDTK::Vector3D<float> scale,
                            const InterpolationMode interpolationMode, const float scaledPositionX);

    static void scaleVolume(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                            const InterpolationMode interpolationMode,
                            const std::size_t numberOfThreads);

    // Nearest neighbor interpolation
    static float getNearestNeigborValue(const VolumeData* const volume,
                                        const VDTK::Vector3D<float>& originalPosition);

    // Linear interpolation
    static inline float interpolateLinear(const std::array<float, 2>& values, const float x);
    static inline float interpolateBilinear(const std::array<std::array<float, 2>, 2>& valuePlane,
                                            const float x, const float y);
    static inline float interpolateTrilinear(
        const std::array<std::array<std::array<float, 2>, 2>, 2>& valueGrid, const float x,
        const float y, const float z);
    static float getTrilinearInterpolatedValue(const VolumeData* const volume,
                                               const VDTK::Vector3D<float>& originalSize,
                                               const VDTK::Vector3D<float>& originalPosition);

    // Cubic interpolation
    static inline float interpolateCubic(const std::array<float, 4>& values, const float x);
    static inline float interpolateBicubic(const std::array<std::array<float, 4>, 4>& valuePlane,
                                           const float x, const float y);
    static inline float interpolateTricubic(
        const std::array<std::array<std::array<float, 4>, 4>, 4>& valueGrid, const float x,
        const float y, const float z);
    static float getTricubicInterpolatedValue(const VolumeData* const volume,
                                              const VDTK::Vector3D<float>& originalSize,
                                              const VDTK::Vector3D<float>& originalPosition);
};

} // namespace VDTK