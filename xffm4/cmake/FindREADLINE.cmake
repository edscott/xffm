# - Try to find 
# Once done, this will define
#
#  READLINE_FOUND - system has READLINE.
#  READLINE_INCLUDE_DIRS - the READLINE. include directories
#  READLINE_LIBRARIES - link these to use READLINE.

find_package(PkgConfig)
pkg_check_modules(libRSVG librsvg-2.0)

set(VERSION_OK TRUE)
if (READLINE_VERSION)
    if (READLINE_FIND_VERSION_EXACT)
        if (NOT("${READLINE_FIND_VERSION}" VERSION_EQUAL "${READLINE_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${READLINE_VERSION}" VERSION_LESS "${READLINE_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(READLINE DEFAULT_MSG READLINE_INCLUDE_DIRS READLINE_LIBRARIES VERSION_OK)
