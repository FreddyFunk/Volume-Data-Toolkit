
#include <cmath>
#include <threadpool/ThreadPool.h>

#include "VolumeResizer.h"

namespace VDTK {
VolumeResizer::VolumeResizer() {}

VolumeResizer::~VolumeResizer() {}

void VolumeResizer::scaleSliceX(const VolumeData* const volume, VolumeData* volumeScaled,
                                const VDTK::Vector3D<float> scale,
                                const InterpolationMode interpolationMode,
                                const float scaledPositionX) {
    const float originalSizeX = static_cast<float>(volume->getSize().getX());
    const float originalSizeY = static_cast<float>(volume->getSize().getY());
    const float originalSizeZ = static_cast<float>(volume->getSize().getZ());
    const VDTK::Vector3D<float> originalSize(originalSizeX, originalSizeY, originalSizeZ);

    for (float scaledPositionY = 0.0f; scaledPositionY < volumeScaled->getSize().getY();
         scaledPositionY++) {
        for (float scaledPositionZ = 0.0f; scaledPositionZ < volumeScaled->getSize().getZ();
             scaledPositionZ++) {
            const float originalPositionX = scaledPositionX / scale.getX();
            const float originalPositionY = scaledPositionY / scale.getY();
            const float originalPositionZ = scaledPositionZ / scale.getZ();
            const VDTK::Vector3D<float> originalPosition(originalPositionX, originalPositionY,
                                                         originalPositionZ);

            float valueInterpolated = 0.0f;

            switch (interpolationMode) {
            case InterpolationMode::Nearest: {
                valueInterpolated = getNearestNeigborValue(volume, originalPosition);
                break;
            }
            case InterpolationMode::Trilinear: {
                valueInterpolated =
                    getTrilinearInterpolatedValue(volume, originalSize, originalPosition);
                break;
            }
            case InterpolationMode::Tricubic: {
                valueInterpolated =
                    getTricubicInterpolatedValue(volume, originalSize, originalPosition);
                break;
            }
            default: { break; }
            }

            volumeScaled->setVoxelValue(scaledPositionX, scaledPositionY, scaledPositionZ,
                                        valueInterpolated);
        }
    }
}

void VolumeResizer::scaleVolume(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                const InterpolationMode interpolationMode,
                                const std::size_t numberOfThreads) {
    const float originalSizeX = static_cast<float>(volume->getSize().getX());
    const float originalSizeY = static_cast<float>(volume->getSize().getY());
    const float originalSizeZ = static_cast<float>(volume->getSize().getZ());
    const VDTK::Vector3D<float> originalSize(originalSizeX, originalSizeY, originalSizeZ);

    const std::size_t scaledSizeX =
        static_cast<std::size_t>(std::round(originalSizeX * scale.getX()));
    const std::size_t scaledSizeY =
        static_cast<std::size_t>(std::round(originalSizeY * scale.getY()));
    const std::size_t scaledSizeZ =
        static_cast<std::size_t>(std::round(originalSizeZ * scale.getZ()));
    const VDTK::VolumeSize scaledSize(scaledSizeX, scaledSizeY, scaledSizeZ);

    const float scaledSpacingX = volume->getSpacing().getX() / scale.getX();
    const float scaledSpacingY = volume->getSpacing().getY() / scale.getY();
    const float scaledSpacingZ = volume->getSpacing().getZ() / scale.getZ();
    const VDTK::VolumeSpacing scaledSpacing(scaledSpacingX, scaledSpacingY, scaledSpacingZ);

    VolumeData volumeScaled(scaledSize, scaledSpacing);

    {
        // create own scope to use destructor of thread pool (wait for all task to
        // finish)
        ThreadPool threadPool(numberOfThreads);

        // scale each x slice using seperate threads
        // we still use tri-XY and not bi-XY interpolation
        for (float scaledPositionX = 0.0f; scaledPositionX < scaledSizeX; scaledPositionX++) {
            threadPool.enqueue(VolumeResizer::scaleSliceX, volume, &volumeScaled, scale,
                               interpolationMode, scaledPositionX);
        }
    }

    *volume = volumeScaled;
}

void VolumeResizer::scaleNearestNeighbor(VolumeData* const volume,
                                         const VDTK::Vector3D<float>& scale,
                                         const std::size_t numberOfThreads) {
    scaleVolume(volume, scale, InterpolationMode::Nearest, numberOfThreads);
}

void VolumeResizer::scaleTrilinear(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                   const std::size_t numberOfThreads) {
    scaleVolume(volume, scale, InterpolationMode::Trilinear, numberOfThreads);
}

void VolumeResizer::scaleTricubic(VolumeData* const volume, const VDTK::Vector3D<float>& scale,
                                  const std::size_t numberOfThreads) {
    scaleVolume(volume, scale, InterpolationMode::Tricubic, numberOfThreads);
}

/*----------------TRILINEAR------------------------
 *
                          z           y  v6____________v7
                          ^          /  / |            /|
                          |         /  /  |           / |
                          |        /  /   |          /  |
                          |       /  v2___|_________v3  |
                          |      /   |    |         |   |
                          |     /    |   v4_________|__v5
                          |    /     |   /          |  /
                          |   /      |  /           | /
                          |  /       | /            |/
                          | /        v0_____________v1
                          |/
                          |-------------------------> x
p(x,y,z) is inside of this cube
-------------------------------------------------*/

float VolumeResizer::getNearestNeigborValue(const VolumeData* const volume,
                                            const VDTK::Vector3D<float>& originalPosition) {
    return volume->getVoxelValue(std::round(originalPosition.getX()),
                                 std::round(originalPosition.getY()),
                                 std::round(originalPosition.getZ()));
}

inline float VolumeResizer::interpolateLinear(const std::array<float, 2>& values, const float x) {
    return values[0] * (1.0f - x) + values[1] * x;
}

inline float VolumeResizer::interpolateBilinear(
    const std::array<std::array<float, 2>, 2>& valuePlane, const float x, const float y) {
    const std::array<float, 2> values = {interpolateLinear(valuePlane[0], y),
                                         interpolateLinear(valuePlane[1], y)};
    return interpolateLinear(values, x);
}

inline float VolumeResizer::interpolateTrilinear(
    const std::array<std::array<std::array<float, 2>, 2>, 2>& valueGrid, const float x,
    const float y, const float z) {
    const std::array<float, 2> values = {interpolateBilinear(valueGrid[0], y, z),
                                         interpolateBilinear(valueGrid[1], y, z)};
    return interpolateLinear(values, x);
}

float VolumeResizer::getTrilinearInterpolatedValue(const VolumeData* const volume,
                                                   const VDTK::Vector3D<float>& originalSize,
                                                   const VDTK::Vector3D<float>& originalPosition) {
    const float x0 = std::floor(originalPosition.getX());
    const float y0 = std::floor(originalPosition.getY());
    const float z0 = std::floor(originalPosition.getZ());
    // prevent access to a pixel out of border
    const float x1 = (std::ceil(originalPosition.getX()) <= originalSize.getX() - 1.0f)
                         ? std::ceil(originalPosition.getX())
                         : originalSize.getX() - 1.0f;
    const float y1 = (std::ceil(originalPosition.getY()) <= originalSize.getY() - 1.0f)
                         ? std::ceil(originalPosition.getY())
                         : originalSize.getY() - 1.0f;
    const float z1 = (std::ceil(originalPosition.getZ()) <= originalSize.getZ() - 1.0f)
                         ? std::ceil(originalPosition.getZ())
                         : originalSize.getZ() - 1.0f;

    std::array<std::array<std::array<float, 2>, 2>, 2> valueGrid;
    valueGrid[0][0][0] = volume->getVoxelValue(x0, y0, z0);
    valueGrid[0][0][1] = volume->getVoxelValue(x0, y0, z1);
    valueGrid[0][1][0] = volume->getVoxelValue(x0, y1, z0);
    valueGrid[0][1][1] = volume->getVoxelValue(x0, y1, z1);
    valueGrid[1][0][0] = volume->getVoxelValue(x1, y0, z0);
    valueGrid[1][0][1] = volume->getVoxelValue(x1, y0, z1);
    valueGrid[1][1][0] = volume->getVoxelValue(x1, y1, z0);
    valueGrid[1][1][1] = volume->getVoxelValue(x1, y1, z1);

    return interpolateTrilinear(valueGrid, originalPosition.getX() - x0,
                                originalPosition.getY() - y0, originalPosition.getZ() - z0);
}

inline float VolumeResizer::interpolateCubic(const std::array<float, 4>& values, const float x) {
    return values[1] +
           0.5f * x *
               (values[2] - values[0] +
                x * (2.0f * values[0] - 5.0f * values[1] + 4.0f * values[2] - values[3] +
                     x * (3.0f * (values[1] - values[2]) + values[3] - values[0])));
}

inline float VolumeResizer::interpolateBicubic(
    const std::array<std::array<float, 4>, 4>& valuePlane, const float x, const float y) {
    const std::array<float, 4> values = {
        interpolateCubic(valuePlane[0], y), interpolateCubic(valuePlane[1], y),
        interpolateCubic(valuePlane[2], y), interpolateCubic(valuePlane[3], y)};
    return interpolateCubic(values, x);
}

inline float VolumeResizer::interpolateTricubic(
    const std::array<std::array<std::array<float, 4>, 4>, 4>& valueGrid, const float x,
    const float y, const float z) {
    const std::array<float, 4> values = {
        interpolateBicubic(valueGrid[0], y, z), interpolateBicubic(valueGrid[1], y, z),
        interpolateBicubic(valueGrid[2], y, z), interpolateBicubic(valueGrid[3], y, z)};
    return interpolateCubic(values, x);
}

float VolumeResizer::getTricubicInterpolatedValue(const VolumeData* const volume,
                                                  const VDTK::Vector3D<float>& originalSize,
                                                  const VDTK::Vector3D<float>& originalPosition) {
    const float x0 = std::floor(originalPosition.getX());
    const float y0 = std::floor(originalPosition.getY());
    const float z0 = std::floor(originalPosition.getZ());
    // prevent access to a pixel out of border
    const float x1 = (std::ceil(originalPosition.getX()) <= originalSize.getX() - 1.0f)
                         ? std::ceil(originalPosition.getX())
                         : originalSize.getX() - 1.0f;
    const float y1 = (std::ceil(originalPosition.getY()) <= originalSize.getY() - 1.0f)
                         ? std::ceil(originalPosition.getY())
                         : originalSize.getY() - 1.0f;
    const float z1 = (std::ceil(originalPosition.getZ()) <= originalSize.getZ() - 1.0f)
                         ? std::ceil(originalPosition.getZ())
                         : originalSize.getZ() - 1.0f;

    // prevent access to a pixel out of border
    const float negativOffsetX = (x0 < 1.0f) ? 0.0f : 1.0f;
    const float negativOffsetY = (y0 < 1.0f) ? 0.0f : 1.0f;
    const float negativOffsetZ = (z0 < 1.0f) ? 0.0f : 1.0f;

    // prevent access to a pixel out of border
    const float positivOffsetX = (x1 >= originalSize.getX() - 1.0f) ? 0.0f : 1.0f;
    const float positivOffsetY = (y1 >= originalSize.getY() - 1.0f) ? 0.0f : 1.0f;
    const float positivOffsetZ = (z1 >= originalSize.getZ() - 1.0f) ? 0.0f : 1.0f;

    std::array<std::array<std::array<float, 4>, 4>, 4> valueGrid;
    valueGrid[0][0][0] =
        volume->getVoxelValue(x0 - negativOffsetX, y0 - negativOffsetY, z0 - negativOffsetZ);
    valueGrid[0][0][1] = volume->getVoxelValue(x0 - negativOffsetX, y0 - negativOffsetY, z0);
    valueGrid[0][0][2] = volume->getVoxelValue(x0 - negativOffsetX, y0 - negativOffsetY, z1);
    valueGrid[0][0][3] =
        volume->getVoxelValue(x0 - negativOffsetX, y0 - negativOffsetY, z1 + positivOffsetZ);

    valueGrid[0][1][0] = volume->getVoxelValue(x0 - negativOffsetX, y0, z0 - negativOffsetZ);
    valueGrid[0][1][1] = volume->getVoxelValue(x0 - negativOffsetX, y0, z0);
    valueGrid[0][1][2] = volume->getVoxelValue(x0 - negativOffsetX, y0, z1);
    valueGrid[0][1][3] = volume->getVoxelValue(x0 - negativOffsetX, y0, z1 + positivOffsetZ);

    valueGrid[0][2][0] = volume->getVoxelValue(x0 - negativOffsetX, y1, z0 - negativOffsetZ);
    valueGrid[0][2][1] = volume->getVoxelValue(x0 - negativOffsetX, y1, z0);
    valueGrid[0][2][2] = volume->getVoxelValue(x0 - negativOffsetX, y1, z1);
    valueGrid[0][2][3] = volume->getVoxelValue(x0 - negativOffsetX, y1, z1 + positivOffsetZ);

    valueGrid[0][3][0] =
        volume->getVoxelValue(x0 - negativOffsetX, y1 + positivOffsetY, z0 - negativOffsetZ);
    valueGrid[0][3][1] = volume->getVoxelValue(x0 - negativOffsetX, y1 + positivOffsetY, z0);
    valueGrid[0][3][2] = volume->getVoxelValue(x0 - negativOffsetX, y1 + positivOffsetY, z1);
    valueGrid[0][3][3] =
        volume->getVoxelValue(x0 - negativOffsetX, y1 + positivOffsetY, z1 + positivOffsetZ);

    valueGrid[1][0][0] = volume->getVoxelValue(x0, y0 - negativOffsetY, z0 - negativOffsetZ);
    valueGrid[1][0][1] = volume->getVoxelValue(x0, y0 - negativOffsetY, z0);
    valueGrid[1][0][2] = volume->getVoxelValue(x0, y0 - negativOffsetY, z1);
    valueGrid[1][0][3] = volume->getVoxelValue(x0, y0 - negativOffsetY, z1 + positivOffsetZ);

    valueGrid[1][1][0] = volume->getVoxelValue(x0, y0, z0 - negativOffsetZ);
    valueGrid[1][1][1] = volume->getVoxelValue(x0, y0, z0);
    valueGrid[1][1][2] = volume->getVoxelValue(x0, y0, z1);
    valueGrid[1][1][3] = volume->getVoxelValue(x0, y0, z1 + positivOffsetZ);

    valueGrid[1][2][0] = volume->getVoxelValue(x0, y1, z0 - negativOffsetZ);
    valueGrid[1][2][1] = volume->getVoxelValue(x0, y1, z0);
    valueGrid[1][2][2] = volume->getVoxelValue(x0, y1, z1);
    valueGrid[1][2][3] = volume->getVoxelValue(x0, y1, z1 + positivOffsetZ);

    valueGrid[1][3][0] = volume->getVoxelValue(x0, y1 + positivOffsetY, z0 - negativOffsetZ);
    valueGrid[1][3][1] = volume->getVoxelValue(x0, y1 + positivOffsetY, z0);
    valueGrid[1][3][2] = volume->getVoxelValue(x0, y1 + positivOffsetY, z1);
    valueGrid[1][3][3] = volume->getVoxelValue(x0, y1 + positivOffsetY, z1 + positivOffsetZ);

    valueGrid[2][0][0] = volume->getVoxelValue(x1, y0 - negativOffsetY, z0 - negativOffsetZ);
    valueGrid[2][0][1] = volume->getVoxelValue(x1, y0 - negativOffsetY, z0);
    valueGrid[2][0][2] = volume->getVoxelValue(x1, y0 - negativOffsetY, z1);
    valueGrid[2][0][3] = volume->getVoxelValue(x1, y0 - negativOffsetY, z1 + positivOffsetZ);

    valueGrid[2][1][0] = volume->getVoxelValue(x1, y0, z0 - negativOffsetZ);
    valueGrid[2][1][1] = volume->getVoxelValue(x1, y0, z0);
    valueGrid[2][1][2] = volume->getVoxelValue(x1, y0, z1);
    valueGrid[2][1][3] = volume->getVoxelValue(x1, y0, z1 + positivOffsetZ);

    valueGrid[2][2][0] = volume->getVoxelValue(x1, y1, z0 - negativOffsetZ);
    valueGrid[2][2][1] = volume->getVoxelValue(x1, y1, z0);
    valueGrid[2][2][2] = volume->getVoxelValue(x1, y1, z1);
    valueGrid[2][2][3] = volume->getVoxelValue(x1, y1, z1 + positivOffsetZ);

    valueGrid[2][3][0] = volume->getVoxelValue(x1, y1 + positivOffsetY, z0 - negativOffsetZ);
    valueGrid[2][3][1] = volume->getVoxelValue(x1, y1 + positivOffsetY, z0);
    valueGrid[2][3][2] = volume->getVoxelValue(x1, y1 + positivOffsetY, z1);
    valueGrid[2][3][3] = volume->getVoxelValue(x1, y1 + positivOffsetY, z1 + positivOffsetZ);

    valueGrid[3][0][0] =
        volume->getVoxelValue(x1 + positivOffsetX, y0 - negativOffsetY, z0 - negativOffsetZ);
    valueGrid[3][0][1] = volume->getVoxelValue(x1 + positivOffsetX, y0 - negativOffsetY, z0);
    valueGrid[3][0][2] = volume->getVoxelValue(x1 + positivOffsetX, y0 - negativOffsetY, z1);
    valueGrid[3][0][3] =
        volume->getVoxelValue(x1 + positivOffsetX, y0 - negativOffsetY, z1 + positivOffsetZ);

    valueGrid[3][1][0] = volume->getVoxelValue(x1 + positivOffsetX, y0, z0 - negativOffsetZ);
    valueGrid[3][1][1] = volume->getVoxelValue(x1 + positivOffsetX, y0, z0);
    valueGrid[3][1][2] = volume->getVoxelValue(x1 + positivOffsetX, y0, z1);
    valueGrid[3][1][3] = volume->getVoxelValue(x1 + positivOffsetX, y0, z1 + positivOffsetZ);

    valueGrid[3][2][0] = volume->getVoxelValue(x1 + positivOffsetX, y1, z0 - negativOffsetZ);
    valueGrid[3][2][1] = volume->getVoxelValue(x1 + positivOffsetX, y1, z0);
    valueGrid[3][2][2] = volume->getVoxelValue(x1 + positivOffsetX, y1, z1);
    valueGrid[3][2][3] = volume->getVoxelValue(x1 + positivOffsetX, y1, z1 + positivOffsetZ);

    valueGrid[3][3][0] =
        volume->getVoxelValue(x1 + positivOffsetX, y1 + positivOffsetY, z0 - negativOffsetZ);
    valueGrid[3][3][1] = volume->getVoxelValue(x1 + positivOffsetX, y1 + positivOffsetY, z0);
    valueGrid[3][3][2] = volume->getVoxelValue(x1 + positivOffsetX, y1 + positivOffsetY, z1);
    valueGrid[3][3][3] =
        volume->getVoxelValue(x1 + positivOffsetX, y1 + positivOffsetY, z1 + positivOffsetZ);

    const float interpolatedValue =
        interpolateTricubic(valueGrid, originalPosition.getX() - x0, originalPosition.getY() - y0,
                            originalPosition.getZ() - z0);
    // clip values
    if (interpolatedValue < 0.0f) {
        return 0.0f;
    } else if (interpolatedValue >= static_cast<float>(UINT16_MAX)) {
        return static_cast<float>(UINT16_MAX);
    }

    return interpolatedValue;
}
} // namespace VDTK