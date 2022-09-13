# This build script is not meant for general use, it is for CI use only!
cmake_minimum_required(VERSION 3.0)
project(implot)

#
# Global options
#

# Same as Dear ImGui
set(CMAKE_CXX_STANDARD 11)

# Arch option for linux
if (NOT APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND DEFINED GCC_ARCH)
    if ("${GCC_ARCH}" MATCHES "Win32|x86|32")
        add_compile_options(-m32)
        add_link_options(-m32)
    elseif ("${GCC_ARCH}" MATCHES "Win64|x64|64")
        add_compile_options(-m64)
        add_link_options(-m64)
    endif ()
endif ()

# Arch option for Mac: arm64 for M1 or x86_64 for intel (32 bits build are deprecated on Mac)
if(APPLE AND DEFINED OSX_ARCH)
    if ("${OSX_ARCH}" MATCHES "x86_64")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    elseif ("${OSX_ARCH}" MATCHES "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        message(FATAL_ERROR "Unhandled OSX_ARCH=${OSX_ARCH}")
    endif()
endif()

#
# Dear ImGui library with no backend
#

set(imgui_sources
    ../imgui/imconfig.h
    ../imgui/imgui.cpp
    ../imgui/imgui.h
    ../imgui/imgui_demo.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_internal.h
    ../imgui/imgui_tables.cpp
    ../imgui/imgui_widgets.cpp
    ../imgui/imstb_rectpack.h
    ../imgui/imstb_textedit.h
    ../imgui/imstb_truetype.h
    )
add_library(imgui ${imgui_sources})
target_include_directories(imgui PUBLIC ../imgui)

#
# ImPlot library
#

file(GLOB SOURCE_CODE ../implot*.*)
add_library(implot STATIC ${SOURCE_CODE})

if(MSVC)
    target_compile_options(implot PRIVATE /W4 /WX)
else()
    target_compile_options(implot PRIVATE -Wall -Werror -pedantic)
endif()

target_include_directories(implot PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
target_link_libraries(implot PUBLIC imgui)

if (UNIX)
    target_link_libraries(implot PUBLIC m stdc++)
endif()

# Switch between several sets of numeric types (by adding `-DIMPLOT_NUMERIC_SET=default|custom|all`)
if (DEFINED IMPLOT_NUMERIC_SET)
    if ("${IMPLOT_NUMERIC_SET}" STREQUAL "default")
        message(STATUS "Compiling for default types")
    elseif("${IMPLOT_NUMERIC_SET}" STREQUAL "custom")
        message(STATUS "Compiling for custom types (float and double)")
        target_compile_definitions(implot PRIVATE "IMPLOT_CUSTOM_NUMERIC_TYPES=(float)(double)")
    elseif("${IMPLOT_NUMERIC_SET}" STREQUAL "all")
        message(STATUS "Compiling for all types")
        target_compile_definitions(implot PRIVATE "IMPLOT_INSTANTIATE_ALL_NUMERIC_TYPES=1")
    else()
        message(FATAL_ERROR "unhandled IMPLOT_NUMERIC_SET=${IMPLOT_NUMERIC_SET}")
    endif()
else()
    # By default, the CI provides support for all known types
    target_compile_definitions(implot PRIVATE IMPLOT_INSTANTIATE_ALL_NUMERIC_TYPES)
endif()

#
# implot example binary application (with no backend)
#
add_executable(example_implot example_implot.cpp)
target_link_libraries(example_implot PRIVATE implot)
