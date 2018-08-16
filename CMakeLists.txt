cmake_minimum_required (VERSION 3.1)
project (xffm C CXX)
#set(CMAKE_INSTALL_PREFIX /tmp)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 3)
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
include(GNUInstallDirs)
# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )
list(APPEND CMAKE_MODULE_PATH 
    "${PROJECT_SOURCE_DIR}/cmake")

# look for header files
#find_path(SIGNAL_H
#  NAMES signal.h
#)

find_package(PkgConfig REQUIRED)
find_package(GLIB 2.52.3 REQUIRED)
find_package(GTK3 3.22.17 REQUIRED)
find_package(X11 REQUIRED)
find_package(TUBO 5.0.16 REQUIRED)


include_directories(
    "common/"
    "${PROJECT_BINARY_DIR}" 
    "${GTK3_INCLUDE_DIRS}"
    "${TUBO_INCLUDE_DIRS}"   )

#    "${PROJECT_SOURCE_DIR}"

#add_subdirectory(xftubo)
#add_subdirectory(xffgr)
add_subdirectory(xffind)

