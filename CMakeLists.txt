cmake_minimum_required (VERSION 3.1)
project (xffm CXX)
#set(CMAKE_INSTALL_PREFIX /tmp)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 3)
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )
list(APPEND CMAKE_MODULE_PATH 
    "${PROJECT_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)
find_package(GTK3 3.22.17 REQUIRED)
find_package(X11 REQUIRED)
find_package(TUBO 5.0.16 REQUIRED)

add_subdirectory(xffind)


