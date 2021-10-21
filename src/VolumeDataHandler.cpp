#include "../include/VDTK/VolumeDataHandler.h"
// IO
#include "file_io/binary_slice/BinarySliceImporter.h"
#include "file_io/bitmap/BitmapExporter.h"
#include "file_io/bitmap/BitmapImporter.h"
#include "file_io/endian_conversion/EndianConverter.h"
#include "file_io/raw/RawReader.h"
#include "file_io/raw/RawWriter.h"
// Filter
#include "filter/GridFilter.h"
#include "filter/InvertVoxelsFilter.h"
#include "filter/VolumeResizer.h"
#include "filter/WindowFilter.h"
// Manipulation
#include "manipulation/EdgeCutter.h"
// Image analysis
#include "imaga_analysis/histogram.h"

namespace VDTK {

VolumeDataHandler::VolumeDataHandler(const std::size_t numberOfUsableThreads)
    : m_numberOfThreads((numberOfUsableThreads > 0) ? numberOfUsableThreads : 1) {}

VolumeDataHandler::~VolumeDataHandler() {}

bool VolumeDataHandler::importRawFile(const std::filesystem::path& filePath,
                                      const uint8_t bitsPerVoxel, const VolumeSize& size,
                                      const VolumeSpacing& spacing) {
    return RawReader::read(&m_VolumeData, filePath, bitsPerVoxel, size, spacing);
}

bool VolumeDataHandler::importMonochromBitmapFolder(const std::filesystem::path& directoryPath,
                                                    const VolumeAxis axis,
                                                    const VolumeSpacing& spacing) {
    return BitmapImporter::importMonochrom(&m_VolumeData, directoryPath, axis, spacing);
}

bool VolumeDataHandler::importColorBitmapFolder(const std::filesystem::path& directoryPath,
                                                const VolumeAxis axis,
                                                const VolumeSpacing& spacing) {
    return BitmapImporter::importColor(&m_VolumeData, directoryPath, axis, spacing);
}

bool VolumeDataHandler::importBinarySlices(const std::filesystem::path& directoryPath,
                                           const uint8_t bitsPerVoxel, const VolumeAxis axis,
                                           const VolumeSize& size, const VolumeSpacing& spacing) {
    return BinarySliceImporter::import(&m_VolumeData, directoryPath, bitsPerVoxel, axis, size,
                                       spacing);
}

bool VolumeDataHandler::exportRawFile(const std::filesystem::path& filePath,
                                      const uint8_t bitsPerVoxel) const {
    return RawWriter::write(filePath, bitsPerVoxel, m_VolumeData);
}

bool VolumeDataHandler::exportToBitmapColor(const std::filesystem::path& directoryPath) const {
    return BitmapExporter::writeColor(directoryPath, m_VolumeData);
}

bool VolumeDataHandler::exportToBitmapMonochrom(const std::filesystem::path& directoryPath) const {
    return BitmapExporter::writeMonochrom(directoryPath, m_VolumeData);
}

uint16_t VolumeDataHandler::getRawValue(const std::size_t x, const std::size_t y,
                                        const std::size_t z) const {
    return m_VolumeData.getVoxelValue(x, y, z);
}

const VolumeData VolumeDataHandler::getVolumeData() const {
    return m_VolumeData;
}

const VolumeSize VolumeDataHandler::getVolumeSize() const {
    return m_VolumeData.getSize();
}

const VolumeSpacing VolumeDataHandler::getVolumeSpacing() const {
    return m_VolumeData.getSpacing();
}

void VolumeDataHandler::applyWindow(WindowingFunction func, const int32_t windowCenter,
                                    const int32_t windowWidth,
                                    const int32_t windowOffset) {
    WindowFilter::applyWindow(&m_VolumeData, func, windowCenter, windowWidth,
                              windowOffset,
                              m_numberOfThreads);
}

void VolumeDataHandler::applyGridFilter(const FilterKernel& filter) {
    GridFilter::applyFilter(&m_VolumeData, filter, m_numberOfThreads);
}

void VolumeDataHandler::cutBorders(const float thresholdISO) {
    if (thresholdISO >= 0.0f && thresholdISO <= 1.0f) {
        EdgeCutter::cutBorders(&m_VolumeData, static_cast<uint16_t>(thresholdISO * UINT16_MAX));
    }
}

void VolumeDataHandler::cutBorders(const uint16_t thresholdISO) {
    EdgeCutter::cutBorders(&m_VolumeData, thresholdISO);
}

void VolumeDataHandler::invertVoxelData() {
    InvertVoxelFilter::invertVoxelData(m_VolumeData);
}

void VolumeDataHandler::scaleToSize(const ScaleMode scaleMode, const VolumeSize& size) {
    const float factorX =
        static_cast<float>(size.getX()) / static_cast<float>(m_VolumeData.getSize().getX());
    const float factorY =
        static_cast<float>(size.getY()) / static_cast<float>(m_VolumeData.getSize().getY());
    const float factorZ =
        static_cast<float>(size.getZ()) / static_cast<float>(m_VolumeData.getSize().getZ());

    scaleVolume(scaleMode, factorX, factorY, factorZ);
}

void VolumeDataHandler::scaleToSpacing(const ScaleMode scaleMode, const VolumeSpacing& spacing) {
    const float factorX = spacing.getX() * m_VolumeData.getSpacing().getX();
    const float factorY = spacing.getY() * m_VolumeData.getSpacing().getY();
    const float factorZ = spacing.getZ() * m_VolumeData.getSpacing().getZ();

    scaleVolume(scaleMode, factorX, factorY, factorZ);
}

void VolumeDataHandler::scaleToEqualSpacing(const ScaleMode scaleMode) {
    float minimumSpacing =
        std::min(m_VolumeData.getSpacing().getX(), m_VolumeData.getSpacing().getY());
    minimumSpacing = std::min(minimumSpacing, m_VolumeData.getSpacing().getZ());

    const float factorX = (1.0f / minimumSpacing) * m_VolumeData.getSpacing().getX();
    const float factorY = (1.0f / minimumSpacing) * m_VolumeData.getSpacing().getY();
    const float factorZ = (1.0f / minimumSpacing) * m_VolumeData.getSpacing().getZ();

    scaleVolume(scaleMode, factorX, factorY, factorZ);
}

void VolumeDataHandler::scaleWithFactor(const ScaleMode scaleMode, const float factor) {
    scaleVolume(scaleMode, factor, factor, factor);
}

void VolumeDataHandler::scaleWithFactor(const ScaleMode scaleMode, const float factorX,
                                        const float factorY, const float factorZ) {
    scaleVolume(scaleMode, factorX, factorY, factorZ);
}

const std::vector<uint16_t> VolumeDataHandler::getHistogram() const {
    return HistogramGenerator::getHistogram(&m_VolumeData);
}

const std::vector<uint16_t> VolumeDataHandler::getHistogramWidthWindowing(
    WindowingFunction func, int32_t windowCenter, int32_t windowWidth, int32_t windowOffset) const {
    return HistogramGenerator::getHistogramWidthWindowing(&m_VolumeData, func, windowCenter,
                                                          windowWidth, windowOffset);
}

void VolumeDataHandler::convertEndianness() {
    EndianConverter::flipEndianness(&m_VolumeData);
}

void VolumeDataHandler::printLegalNotice() {
    // Very ugly, but does the job.
    // Feel free to make it better :-)

    // Volume Data Toolkit
    std::cout << "MIT License\n\nCopyright (c) 2018 Frederic Laing\n\nPermission is "
                 "hereby granted, free of charge, to any person obtaining a copy\nof "
                 "this software and associated documentation files (the \"Software\"), "
                 "to deal\nin the Software without restriction, including without "
                 "limitation the rights\nto use, copy, modify, merge, publish, "
                 "distribute, sublicense, and/or sell\ncopies of the Software, and to "
                 "permit persons to whom the Software is\nfurnished to do so, subject "
                 "to the following conditions:\n\nThe above copyright notice and this "
                 "permission notice shall be included in all\ncopies or substantial "
                 "portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", "
                 "WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT "
                 "LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\nFITNESS FOR A "
                 "PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL "
                 "THE\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES "
                 "OR OTHER\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                 "OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE "
                 "OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE.";
    std::cout << std::endl;

    // C++ Bitmap Library
    std::cout << "C++ Bitmap Library by Arash Partow";
    std::cout << std::endl;

    // libbmpread
    std::cout << "Copyright (C) 2005, 2012, 2016, 2018 Charles Lindsay\n\nThis "
                 "software is provided 'as-is', without any express or "
                 "implied\nwarranty.  In no event will the authors be held liable for "
                 "any damages\narising from the use of this software.\n\nPermission is "
                 "granted to anyone to use this software for any purpose,\nincluding "
                 "commercial applications, and to alter it and redistribute "
                 "it\nfreely, subject to the following restrictions:\n\n1. The origin "
                 "of this software must not be misrepresented; you must not\n   claim "
                 "that you wrote the original software. If you use this software\n   "
                 "in a product, an acknowledgment in the product documentation would "
                 "be\n   appreciated but is not required.\n2. Altered source versions "
                 "must be plainly marked as such, and must not be\n   misrepresented "
                 "as being the original software.\n3. This notice may not be removed "
                 "or altered from any source distribution.";
    std::cout << std::endl;

    std::cout << std::endl;
}

void VolumeDataHandler::scaleVolume(const ScaleMode scaleMode, const float factorX,
                                    const float factorY, const float factorZ) {
    // if spacing is "1, 1, 1" we do not need to scale
    if (factorX != 1.0f || factorY != 1.0f || factorZ != 1.0f) {
        switch (scaleMode) {
        case ScaleMode::NearestNeighbor: {
            VolumeResizer::scaleNearestNeighbor(
                &m_VolumeData, Vector3D<float>(factorX, factorY, factorZ), m_numberOfThreads);
            break;
        }
        case ScaleMode::Linear: {
            VolumeResizer::scaleTrilinear(&m_VolumeData, Vector3D<float>(factorX, factorY, factorZ),
                                          m_numberOfThreads);
            break;
        }
        case ScaleMode::Cubic: {
            VolumeResizer::scaleTricubic(&m_VolumeData, Vector3D<float>(factorX, factorY, factorZ),
                                         m_numberOfThreads);
            break;
        }
        default:
            break;
        }
    }
}

} // namespace VDTK
