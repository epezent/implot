cmake_minimum_required(VERSION 3.0)

project(implot)

option(BUILD_IMPLOT_SHARED "Build implot shared library" ON)
option(BUILD_IMPLOT_STATIC "Build implot static library" ON)
option(BUILD_IMPLOT_DEMO "Build implot demo" OFF)

SET(IMPLOT_SOURCES
    implot.cpp
    implot_items.cpp)

if (BUILD_IMPLOT_DEMO)
    message(STATUS "Building with implot demo.")
    SET(IMPLOT_SOURCES
        ${IMPLOT_SOURCES}
        implot_demo.cpp)
endif ()

if (BUILD_IMPLOT_SHARED)
    message(STATUS "Building implot as a shared library.")
    add_library(implot_shared SHARED
        ${IMPLOT_SOURCES})
endif ()

if (BUILD_IMPLOT_STATIC)
    message(STATUS "Building implot as a static library.")
    add_library(implot STATIC
        ${IMPLOT_SOURCES})
endif ()
