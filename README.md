# Volume Data Toolkit (VDTK)
[![Build Status](https://travis-ci.com/FreddyFunk/Volume-Data-Toolkit.svg?token=2yczxVPQMF8ykEfyfxcb&branch=master)](https://travis-ci.com/FreddyFunk/Volume-Data-Toolkit)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)](https://isocpp.org)

#### Importer
+ 3D RAW (8, 16 bit)
+ Series of bitmap images (.BMP) (1, 4, 8, 16, 24, 32 bit)
+ Series of binary slices (8, 16 Bit)
+ Little-Endian and Big-Endian support

#### Exporter
+ 3D RAW (8, 16 bit)
+ Series of bitmap images (.BMP) (24 bit) monochrom or in color

#### Filter
+ Apply window (level, width, offset) with linear function
+ Apply custom 3x3x3 and 5x5x5 image filter (some example filters are included)
+ Scale volume by one factor or an individual factor for x, y and z (nearest, trilinear, tricubic)
+ Invert voxel data

#### Image Analysis
+ Generate histogram
  + Without or with value window (linear, linear exact, sigmoid)

#### Manipulation
+ Remove empty space on the borders of the volume via a threshold
+ Direct access to volumetric data
  + Read/Write voxel pixel data width xyz coordinates
  + Read/Write slice pixel data width XY, XZ, YZ axis
  + Read/Write whole volume voxel data

#### Build
+ git clone --recursive https://github.com/FreddyFunk/Volume-Data-Toolkit.git
+ use CMake 3.9 or newer to build
+ requires C++ 17 and std::filesystem support
