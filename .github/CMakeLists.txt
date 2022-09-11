# This build script is not meant for general use, it is for CI use only!
cmake_minimum_required(VERSION 3.0)
project(implot)

# Same as Dear ImGui
set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_CODE ../implot*.*)
add_library(implot STATIC ${SOURCE_CODE})

if(MSVC)
    target_compile_options(implot PRIVATE /W4 /WX)
else()
    target_compile_options(implot PRIVATE -Wall -Werror -pedantic)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND DEFINED GCC_ARCH)
    if ("${GCC_ARCH}" MATCHES "Win32|x86|32")
        target_compile_options(implot PRIVATE -m32)
    elseif ("${GCC_ARCH}" MATCHES "Win64|x64|64")
        target_compile_options(implot PRIVATE -m64)
    endif ()
endif ()

target_include_directories(implot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../imgui)
target_compile_definitions(implot PRIVATE IMPLOT_INSTANTIATE_ALL_NUMERIC_TYPES=1)
