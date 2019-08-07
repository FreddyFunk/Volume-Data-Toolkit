#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class InvertVoxelFilter {
public:
    InvertVoxelFilter();
    ~InvertVoxelFilter();

    static const void invertVoxelData(VolumeData& volume);

private:
};
} // namespace VDTK
