# - Try to find 
# Once done, this will define
#
#  libReadline_FOUND - system has READLINE.
#  libReadline_INCLUDE_DIRS - the READLINE. include directories
# libReadline_LIBRARIES - link these to use READLINE.

find_package(PkgConfig)
pkg_check_modules(libReadline readline)

set(VERSION_OK TRUE)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libReadline DEFAULT_MSG libReadline_INCLUDE_DIRS libReadline_LIBRARIES VERSION_OK)
