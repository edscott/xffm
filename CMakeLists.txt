cmake_minimum_required (VERSION 3.1)
project (xffm C CXX)
set(CMAKE_INSTALL_PREFIX /usr/local)
set (CMAKE_CXX_STANDARD 11)
set(PREFIX_BIN /usr/local/bin)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 91)
#set(CMAKE_CXX_FLAGS "-ggdb")

# work in alpha phase...
#add_definitions(-DENABLE_CUSTOM_RESPONSE)
#add_definitions(-DENABLE_PKG_MODULE)
#add_definitions(-DENABLE_DIFF_MODULE)
#add_definitions(-DENABLE_FSTAB_MODULE)

set(CMAKE_EXE_LINKER_FLAGS "-L/usr/lib -L/usr/local/lib")
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_program(AUTOTOOLS aclocal)
if (NOT AUTOTOOLS)
	message(FATAL_ERROR, "To build xffm+ you also need aclocal (automake).")
	return()
endif()
find_program(AUTOTOOLS automake)
if (NOT AUTOTOOLS)
	message(FATAL_ERROR, "To build xffm+ you also need automake.")
	return()
endif()
find_program(AUTOTOOLS autoheader)
if (NOT AUTOTOOLS)
	message(FATAL_ERROR, "To build xffm+ you also need autoheader.")
	return()
endif()
find_program(AUTOTOOLS autoconf)
if (NOT AUTOTOOLS)
	message(FATAL_ERROR, "To build xffm+ you also need autoconf.")
	return()
endif()

find_package(PkgConfig REQUIRED)
find_package(GLIB 2.56.2 REQUIRED)
find_package(GTK3 3.22.1 REQUIRED)
#find_package(GTK3 3.24.1 REQUIRED)
find_package(X11 REQUIRED)

include(GNUInstallDirs)
include(cmake/headers.cmake)
include(cmake/freeDesktop.cmake)
include(cmake/programs.cmake)
include(cmake/libraries.cmake)
#include(cmake/xffmMacros.cmake)
#execute_process(COMMAND ls -l /home)


execute_process(COMMAND ln -f -s ../autotools/configure.ac configure.ac)
execute_process(COMMAND ln -f -s ../autotools/Makefile.am Makefile.am)
execute_process(COMMAND rm -rf po)
execute_process(COMMAND cp -R ../po po)
execute_process(COMMAND rm -rf m4)
execute_process(COMMAND ln -f -s ../autotools/m4 m4)
execute_process(COMMAND cp -f ../COPYING ./)
execute_process(COMMAND touch README)
execute_process(COMMAND touch NEWS)
execute_process(COMMAND touch AUTHORS)
execute_process(COMMAND touch ChangeLog)

message(STATUS "execute_process aclocal...")
execute_process(COMMAND aclocal -I m4)
message(STATUS "execute_process autoheader...")
execute_process(COMMAND autoheader)
message(STATUS "execute_process autoconf...")
execute_process(COMMAND autoconf)
message(STATUS "execute_process automake...")
execute_process(COMMAND automake --add-missing --copy)
execute_process(COMMAND ./configure --prefix=/usr/local)
execute_process(COMMAND mv -v po/Makefile po/Makefile.autotools)
#execute_process(COMMAND po/build)
#execute_process(COMMAND make update-po)
execute_process(COMMAND rm Makefile)



# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )
add_subdirectory(po)

add_subdirectory(xffm)
add_subdirectory(structure)


