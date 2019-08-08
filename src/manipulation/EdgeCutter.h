#pragma once
#include "../include/VDTK/common/CommonDataTypes.h"

namespace VDTK {
class EdgeCutter {
public:
    EdgeCutter();
    ~EdgeCutter();

    static void cutBorders(VolumeData* const volume, const uint16_t threshold = 0);

private:
};
} // namespace VDTK