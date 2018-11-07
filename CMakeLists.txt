cmake_minimum_required (VERSION 3.1)
project (xffm C CXX)
set(CMAKE_INSTALL_PREFIX /usr/local)
set (CMAKE_CXX_STANDARD 11)
set(PREFIX_BIN /usr/local/bin)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 4)
set(CMAKE_CXX_FLAGS "-ggdb")
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
include(GNUInstallDirs)

# look for header files

#message(STATUS "Checking for freedesktop.org.xml...")
#find_file(FREEDESKTOP  NAMES freedesktop.org.xml 
#    PATHS
#    /usr/share/mime/packages 
#    /usr/local/share/mime/packages 
#    )
#if(NOT FREEDESKTOP)
#    set(FREEDESKTOP ${FREEDESKTOP})
#    message( WARNING "File freedesktop.org.xml was not found.")
#    message( WARNING "Mimetype functionality may be limited." )
#else()
#    set(FREEDESKTOP "FREEDESKTOP \"${FREEDESKTOP}\" ")
#    message( STATUS  "freedesktop.org.xml was found." )
#endif()

message(STATUS "Checking for freedesktop.org.xml globs...")
find_file(FREEDESKTOP_GLOBS  NAMES globs 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_GLOBS)
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml globs was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS \"${FREEDESKTOP_GLOBS}\" ")
    message( STATUS  "freedesktop.org.xml globs was found." )
endif()

message(STATUS "Checking for freedesktop.org.xml aliases...")
find_file(FREEDESKTOP_ALIAS  NAMES aliases 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_ALIAS)
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml aliases was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS \"${FREEDESKTOP_ALIAS}\" ")
    message( STATUS  "freedesktop.org.xml aliases was found." )
endif()

message(STATUS "Checking for freedesktop.org.xml generic-icons...")
find_file(FREEDESKTOP_ICONS  NAMES generic-icons 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_ICONS)
    set(FREEDESKTOP_ICONS  "FREEDESKTOP_ICONS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml generic-icons was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ICONS "FREEDESKTOP_ICONS \"${FREEDESKTOP_ICONS}\" ")
    message( STATUS  "freedesktop.org.xml generic-icons was found." )
endif()


message(STATUS "Checking for mimetype (perl)...")
find_program(MIMETYPE_PROGRAM mimetype)
if(NOT MIMETYPE_PROGRAM)
    set(MIMETYPE_PROGRAM "MIMETYPE_PROGRAM_NOTFOUND")
    message( WARNING "Perl mimetype program was not found, ${MIMETYPE_PROGRAM}.")
    message( WARNING "Mimetype functionality may be limited." )
    message( WARNING "Gentoo: dev-perl/File-MimeInfo or Archlinux: perl-file-mimeinfo" )
else()
    message( STATUS "${MIMETYPE_PROGRAM} was found. Good!" )
    set(MIMETYPE_PROGRAM "MIMETYPE_PROGRAM \"${MIMETYPE_PROGRAM}\" ")
endif()

message(STATUS "Checking for signal.h...")
find_path(SIGNAL_H  NAMES signal.h )
if(NOT SIGNAL_H)
    set(SIGNAL_H "SIGNAL_H_NOTFOUND")
    message( STATUS "signal.h is not found" )
else()
    set(SIGNAL_H "HAVE_SIGNAL_H 1")
    message( STATUS  "signal.h was found." )
endif()

message(STATUS "Checking for grep command...")
find_program(HAVE_GREP grep )
if(NOT HAVE_GREP)
    set(HAVE_GREP "HAVE_GREP_NOTFOUND")
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
#find_package(LibXml2 2.9.8 REQUIRED)
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


