# - Try to find 
# Once done, this will define
#
#  libRSVG_FOUND - system has libRSVG.
#  libRSVG_INCLUDE_DIRS - the libRSVG. include directories
#  libRSVG_LIBRARIES - link these to use libRSVG.

find_package(PkgConfig)
pkg_check_modules(libRSVG librsvg-2.0)

set(VERSION_OK TRUE)
if (libRSVG_VERSION)
    if (libRSVG_FIND_VERSION_EXACT)
        if (NOT("${libRSVG_FIND_VERSION}" VERSION_EQUAL "${libRSVG_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${libRSVG_VERSION}" VERSION_LESS "${libRSVG_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libRSVG DEFAULT_MSG libRSVG_INCLUDE_DIRS libRSVG_LIBRARIES VERSION_OK)
