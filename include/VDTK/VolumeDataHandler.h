#pragma once

#include "common/CommonDataTypes.h"


namespace VDTK {
	class VolumeDataHandler
	{
	public:
		VolumeDataHandler(const uint32_t numberOfUsableThreads = std::thread::hardware_concurrency());
		~VolumeDataHandler();

		bool importRawFile(const std::filesystem::path& filePath,
			const uint8_t bitsPerVoxel,
			const VolumeSize& size,
			const VolumeSpacing& spacing);

		bool importMonochromBitmapFolder(const std::filesystem::path& directoryPath, const VolumeAxis axis, const VolumeSpacing& spacing);
		bool importColorBitmapFolder(const std::filesystem::path& directoryPath, const VolumeAxis axis, const VolumeSpacing& spacing);

		bool importBinarySlices(const std::filesystem::path& directoryPath, const uint8_t bitsPerVoxel, const VolumeAxis axis, const VolumeSize& size, const VolumeSpacing& spacing);

		// export with 8 or 16 bit
		bool exportRawFile(const std::filesystem::path& filePath, const uint8_t bitsPerVoxel) const;
		// if path is a directory path, generic file name gets generated
		bool exportToBitmapColor(const std::filesystem::path& directoryPath) const;
		// if path is a directory path, generic file name gets generated
		bool exportToBitmapMonochrom(const std::filesystem::path& directoryPath) const;

		// gets the raw voxel value from the curren volume on a given position
		const uint16_t getRawValue(const uint32_t x, const uint32_t y, const uint32_t z) const;
		const VolumeData getVolumeData() const;

		void applyWindow(const int32_t windowLevel, const int32_t windowWidth, const int32_t windowOffset);

		void applyGridFilter(const FilterKernel& filter);

		// threshold between 0.0 and 1.0
		void cutBorders(const float thresholdISO);
		// threshold between 0 and UINT16_MAX
		void cutBorders(const uint16_t thresholdISO);

		const void invertVoxelData();

		// scales volume to the choosen size
		void scaleToSize(const ScaleMode scaleMode, const VolumeSize& size);
		// scales each dimension to so each dimension has choosen spacing
		void scaleToSpacing(const ScaleMode scaleMode, const VolumeSpacing& spacing);
		// upscale each dimension to fit the minimum spacing axis
		// afterwards all spacing dimensions are equal
		void scaleToEqualSpacing(const ScaleMode scaleMode);
		void scaleWithFactor(const ScaleMode scaleMode, const float factor);
		void scaleWithFactor(const ScaleMode scaleMode, const float factorX, const float factorY, const float factorZ);

		const std::vector<uint16_t> getHistogram() const;
		const std::vector<uint16_t> getHistogramWidthWindowing(WindowingFunction func, int32_t windowLevel, int32_t windowWidth, int32_t windowOffset) const;

		void convertEndianness();

		static const void printLegalNotice();

	private:
		// voxel data is stored here
		VolumeData m_VolumeData = VolumeData(VolumeSize(0, 0, 0), VolumeSpacing(0.0, 0.0, 0.0));
		const uint32_t m_numberOfThreads;

		void scaleVolume(const ScaleMode scaleMode, const float factorX, const float factorY, const float factorZ);
	};
}
