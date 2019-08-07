cmake_minimum_required(VERSION 3.9...3.14)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# CMake versions older than 3.12 do not support version range
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(VolumeDataToolkit VERSION 1.0.0
                  DESCRIPTION "Volume Data Toolkit"
                  LANGUAGES C CXX)

# Get Git submodule dependencies
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, 
        please checkout submodules manually with \"git submodule update --init --recursive\"")
    endif()
endif()

# This function replicates the directory hierarchy of a target, so that the layout of the files in IDEs match
function(replicate_directory_structure target_name)
    # Add any files that are not in the source list to a separate group 
    source_group("other files" REGULAR_EXPRESSION ".")
    get_target_property(target_sources "${target_name}" SOURCES)
    
    foreach(file_name IN LISTS target_sources)
        get_filename_component(directory_name "${file_name}" PATH)
        # Group names use '\\' instead of '/'
        string(REPLACE "/" "\\" group_name "${directory_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()


# This function replicates the directory hierarchy of a target, so that the layout of the files in IDEs match
function(replicate_directory_structure target_name)
    # Add any files that are not in the source list to a separate group 
    source_group("other files" REGULAR_EXPRESSION ".")
    get_target_property(target_sources "${target_name}" SOURCES)
    
    foreach(file_name IN LISTS target_sources)
        get_filename_component(directory_name "${file_name}" PATH)
        # Group names use '\\' instead of '/'
        string(REPLACE "/" "\\" group_name "${directory_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()


add_library(vdtk_lib 

include/VDTK/common/CommonDataTypes.h
include/VDTK/common/CommonIO.h

src/VolumeDataHandler.cpp

src/file_io/binary_slice/BinarySliceImporter.cpp
src/file_io/binary_slice/BinarySliceImporter.h
src/file_io/bitmap/BitmapExporter.cpp
src/file_io/bitmap/BitmapExporter.h
src/file_io/bitmap/BitmapImporter.cpp
src/file_io/bitmap/BitmapImporter.h
src/file_io/endian_conversion/EndianConverter.h
src/file_io/raw/RawReader.cpp
src/file_io/raw/RawReader.h
src/file_io/raw/RawWriter.cpp
src/file_io/raw/RawWriter.h

src/filter/GridFilter.cpp
src/filter/GridFilter.h
src/filter/InvertVoxelsFilter.cpp
src/filter/InvertVoxelsFilter.h
src/filter/VolumeResizer.cpp
src/filter/VolumeResizer.h
src/filter/WindowFilter.cpp
src/filter/WindowFilter.h

src/manipulation/EdgeCutter.cpp
src/manipulation/EdgeCutter.h

src/imaga_analysis/histogram.h
src/imaga_analysis/histogram.cpp
)

replicate_directory_structure(vdtk_lib)

add_subdirectory(external)

target_link_libraries(vdtk_lib PRIVATE bitmap libbmpread threadpool)

# set C++ language standard to c++17
target_compile_features(vdtk_lib PRIVATE cxx_std_17)

set_target_properties(vdtk_lib PROPERTIES PUBLIC_HEADER include/VDTK/VolumeDataHandler.h)

target_include_directories(vdtk_lib INTERFACE include/)
target_include_directories(vdtk_lib PRIVATE src/)


replicate_directory_structure(vdtk_lib)
