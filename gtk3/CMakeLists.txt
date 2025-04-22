cmake_minimum_required (VERSION 3.5)
project (xffm C CXX)
set(CMAKE_INSTALL_PREFIX /usr/local)
set (CMAKE_CXX_STANDARD 11)
set(PREFIX_BIN /usr/local/bin)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 987)
include(cmake/pthreads.cmake)


# work in alpha phase...
#set(ALPHA "#define ALPHA")
# debugging in lldb/gdb 
# to avoid disabling ALPHA, set these options via environment
# variable CXXFLAGS.
set(CMAKE_CXX_FLAGS "-g -O0 -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED")
#set(CMAKE_CXX_FLAGS "-g -O0")
#set(CMAKE_CXX_FLAGS "-g -O3")

execute_process(COMMAND uname -s OUTPUT_VARIABLE UNAME)
    
set(FREEBSD "BSD_NOT_FOUND")
if(UNAME MATCHES FreeBSD)
    message(STATUS "System is FreeBSD: uname -a = ${UNAME}")
    set(FREEBSD "BSD_FOUND")
else()
 if(UNAME MATCHES OpenBSD)
    message(STATUS "System is OpenBSD: uname -a = ${UNAME}")
    set(FREEBSD "BSD_FOUND")
 endif()
endif()


set(CMAKE_EXE_LINKER_FLAGS "-L/usr/lib -L/usr/local/lib")
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")


find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
#set(ENV{PKG_CONFIG_PATH} "/opt/lib/pkgconfig")
find_package(GLIB 2.56.2 REQUIRED)
find_package(GTK3 3.24.1 REQUIRED)

include(GNUInstallDirs)
include(cmake/headers.cmake)
include(cmake/freeDesktop.cmake)
include(cmake/programs.cmake)
include(cmake/msgfmt.cmake)



# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )

#add_subdirectory(gtk4)
add_subdirectory(xffm)
#add_subdirectory(structure)


