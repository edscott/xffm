# - Try to find 
# Once done, this will define
#
#  libMagic_FOUND - system has libMagic.
#  libMagic_INCLUDE_DIRS - the libMagic. include directories
#  libMagic_LIBRARIES - link these to use libMagic.
#
find_package(PkgConfig)
pkg_check_modules(libMagic libmagic)

set(VERSION_OK TRUE)
if (libMagic_VERSION)
    if (libMagic_FIND_VERSION_EXACT)
        if (NOT("${libMagic_FIND_VERSION}" VERSION_EQUAL "${libMagic_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${libMagic_VERSION}" VERSION_LESS "${libMagic_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libMagic DEFAULT_MSG libMagic_INCLUDE_DIRS libMagic_LIBRARIES VERSION_OK)
