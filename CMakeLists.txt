cmake_minimum_required (VERSION 3.1)
project (xffm C CXX)
set(CMAKE_INSTALL_PREFIX /usr/local)
set (CMAKE_CXX_STANDARD 11)
set(PREFIX_BIN /usr/local/bin)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 3)
set(CMAKE_CXX_FLAGS "-ggdb")
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
include(GNUInstallDirs)

# look for header files
message(STATUS "Checking for signal.h...")
find_path(SIGNAL_H  NAMES signal.h )
if(NOT SIGNAL_H)
    set(SIGNAL_H ${SIGNAL_H})
    message( STATUS "signal.h is not found" )
else()
    set(SIGNAL_H "HAVE_SIGNAL_H 1")
    message( STATUS  "signal.h was found." )
endif()

message(STATUS "Checking for grep command...")
find_program(HAVE_GREP grep )
if(NOT HAVE_GREP)
    set(HAVE_GREP ${HAVE_GREP})
    message( FATAL_ERROR "grep is not found" )
else()
    set(HAVE_GREP "HAVE_GREP 1")
    message( STATUS "grep was found. Is it GNU grep?" )
endif()

# mimeinfo is now replaced by dev-perl/File-MimeInfo
#message(STATUS "Checking for libmagic...")
#find_library(LIBMAGIC NAMES libmagic.so)
#if(NOT LIBMAGIC)
#    set(LIBMAGIC ${LIBMAGIC})
#    message( WARNING "*** libmagic is not found: Look for it in \"file\" package." )
#else()
#    set(LIBMAGIC "HAVE_LIBMAGIC 1")
#    message(STATUS "libmagic was found." )
#endif()
message(STATUS "Checking for mimetype (perl)...")
find_program(MIMETYPE_PROGRAM mimetype)
if(NOT MIMETYPE_PROGRAM)
    set(MIMETYPE_PROGRAM ${MIMETYPE_PROGRAM})
    message( FATAL_ERROR "mimetype is not found, ${MIMETYPE_PROGRAM}. Install: gentoo: dev-perl/File-MimeInfo or Archlinux: perl-file-mimeinfo\n" )
else()
    message( STATUS "${MIMETYPE_PROGRAM} was found. Good!" )
endif()


# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )
list(APPEND CMAKE_MODULE_PATH 
    "${PROJECT_SOURCE_DIR}/cmake")
find_package(PkgConfig REQUIRED)
#DND Works with Archlinux updated box
#find_package(GTK3 3.24.1 REQUIRED)
#find_package(GLIB 2.58.1 REQUIRED)

#DND does not work with gentoo local libs
find_package(GLIB 2.56.2 REQUIRED)
find_package(GTK3 3.24.1 REQUIRED)
#Gentoo installed, DND does not work
find_package(GLIB 2.52.3 REQUIRED)
find_package(GTK3 3.22.30 REQUIRED)
find_package(X11 REQUIRED)
#CHECK_INCLUDE_FILE_CXX("signal.h" HAVE_SIGNAL_H)

#find_program(GREP NAMES "grep")
#CHECK_INCLUDE_FILE_CXX("UnitTest++.h" HAVE_UNITTESTXX)
#IF(NOT HAVE_UNITTESTXX)
#  UNSET(HAVE_UNITTESTXX CACHE)
#  message( FATAL_ERROR "UnitTest++ is not found" )
#ENDIF()

include_directories(
    "./xffm"
    "${PROJECT_BINARY_DIR}" 
    "${GTK3_INCLUDE_DIRS}"   )

#    "${PROJECT_SOURCE_DIR}"

add_subdirectory(xffm)


