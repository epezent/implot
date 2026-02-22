cmake_minimum_required(VERSION 3.18)

project(implot
        LANGUAGES CXX C
        VERSION 0.18.01)

set(CMAKE_CXX_STANDARD  20)

set(LIBRARY_TARGET_NAME ${PROJECT_NAME})

# define source directories
set(${LIBRARY_TARGET_NAME}_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}")

set(${LIBRARY_TARGET_NAME}_HEADER_FILES
      ${${LIBRARY_TARGET_NAME}_SOURCE_DIR}/implot.h
      ${${LIBRARY_TARGET_NAME}_SOURCE_DIR}/implot_internal.h
   )

set(${LIBRARY_TARGET_NAME}_SOURCE_FILES
      ${${LIBRARY_TARGET_NAME}_SOURCE_DIR}/implot.cpp
      ${${LIBRARY_TARGET_NAME}_SOURCE_DIR}/implot_demo.cpp
      ${${LIBRARY_TARGET_NAME}_SOURCE_DIR}/implot_items.cpp
   )

add_library(
  ${LIBRARY_TARGET_NAME}
  ${${LIBRARY_TARGET_NAME}_SOURCE_FILES}
  ${${LIBRARY_TARGET_NAME}_BACKEND_SOURCE_FILES})

add_library(
  ${PROJECT_NAME}::${LIBRARY_TARGET_NAME}
  ALIAS ${LIBRARY_TARGET_NAME})

set_target_properties(${LIBRARY_TARGET_NAME}
  PROPERTIES
    VERSION "${${PROJECT_NAME}_VERSION}"
    PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HEADER_FILES}")

    # check if the CONDA_PREFIX environment variable is defined
if(DEFINED ENV{CONDA_PREFIX})
    message(STATUS "Conda environment detected. Prefix: $ENV{CONDA_PREFIX}")
    set(CONDA_FOUND 1)
else()
    message(STATUS "No active Conda environment detected.")
    set(CONDA_FOUND 0)
endif()

# specify include directories for both compilation and installation process.
target_include_directories(${LIBRARY_TARGET_NAME}
  PUBLIC
    # This path is included if the condition "CONDA_FOUND" is true (1)
    # The $<...: ...> syntax is a generator expression
    $<IF:${CONDA_FOUND},$<BUILD_INTERFACE:$ENV{CONDA_PREFIX}/include>,>
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

# find the libraries which will be needed by implot
find_package(imgui CONFIG REQUIRED)
if(${imgui_FOUND})
    message(STATUS "Found imgui library.")
    message(STATUS "imgui_SOURCE_DIR: ${imgui_SOURCE_DIR}")
else()
    message(FATAL_ERROR "Could not locate imgui library. Please install the required development files.")
endif()

# Print status messages to show what variables were set by find_package(imgui)
# Reference: https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html#properties-on-targets

if(TARGET imgui)
    # get the VERSION property
    get_target_property(IMGUI_PACKAGE_VERSION imgui VERSION)
    message(STATUS "imgui library version: ${IMGUI_PACKAGE_VERSION}")

    # get the INTERFACE_INCLUDE_DIRECTORIES property
    get_target_property(IMGUI_PACKAGE_INCLUDE_DIRS imgui INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "imgui interface include directories: ${IMGUI_PACKAGE_INCLUDE_DIRS}")

    else()
    message(STATUS "imgui target not found, please check top-level CMakeLists.txt and ensure it correctly fetches the imgui library.")
endif()

set(${LIBRARY_TARGET_NAME}_LINK_LIBRARIES
  imgui::imgui
)

# specify target link libraries.
target_link_libraries(${LIBRARY_TARGET_NAME}
  PRIVATE
    ${${LIBRARY_TARGET_NAME}_LINK_LIBRARIES})

# specify default install location
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Specify install location." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# defines CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# see https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# specify installation targets, typology and destination folders.
install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${LIBRARY_TARGET_NAME}   # associates this target with an export name
        LIBRARY          DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE          DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME          DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

# specify export configuration to make the library discoverable by other cmake project using find_package()
install(EXPORT ${LIBRARY_TARGET_NAME}
   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
   NAMESPACE   ${PROJECT_NAME}::
)

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for ${PROJECT_NAME}.")
