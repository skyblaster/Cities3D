cmake_minimum_required(VERSION 3.24)
project(Cities3D VERSION 0.10.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Shared cores + plugins must resolve one libwx at runtime ($ORIGIN next to the
# binary). Static wx duplicated into each .so crashes on Linux/Flatpak.
if(UNIX AND NOT APPLE)
    set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
    set(CMAKE_BUILD_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(CITIES3D_USE_MINIAUDIO "Play sounds with miniaudio" ON)

set(CITIES3D_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cities3D/src")
set(CITIES3D_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Cities3D/distribute")
if(WIN32)
    set(CITIES3D_PLUGIN_DIR_DEBUG "${CITIES3D_DIST_DIR}/pluginsD")
    set(CITIES3D_PLUGIN_DIR_RELEASE "${CITIES3D_DIST_DIR}/plugins")
else()
    set(CITIES3D_PLUGIN_DIR_DEBUG "${CITIES3D_DIST_DIR}/plugins")
    set(CITIES3D_PLUGIN_DIR_RELEASE "${CITIES3D_DIST_DIR}/plugins")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Cities3DHelpers)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base net xml adv aui gl html)

add_subdirectory(Cities3D/src/json_spirit)
add_subdirectory(Cities3D)
