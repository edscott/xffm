cmake_minimum_required (VERSION 3.1)
project (xffm C CXX)
set(CMAKE_INSTALL_PREFIX /usr/local)
set (CMAKE_CXX_STANDARD 11)
set(PREFIX_BIN /usr/local/bin)
# Version info
set(xffm_VERSION_MAJOR 0)
set(xffm_VERSION_MINOR 91)

# work in alpha phase...
set(CMAKE_CXX_FLAGS "-ggdb")
add_definitions(-DENABLE_CUSTOM_RESPONSE)
add_definitions(-DENABLE_PKG_MODULE)
add_definitions(-DENABLE_DIFF_MODULE)
add_definitions(-DENABLE_FSTAB_MODULE)

# Setting up Intl
find_package (Intl REQUIRED)
find_package(Gettext REQUIRED)
include_directories(${INTL_INCLUDE_DIRS})
link_directories(${INTL_LIBRARY_DIRS})




set(CMAKE_EXE_LINKER_FLAGS "-L/usr/lib -L/usr/local/lib")
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
#find_program(AUTOTOOLS aclocal)
#if (NOT AUTOTOOLS)
#	message(FATAL_ERROR, "To build xffm+ you also need aclocal (from automake package).")
#	return()
#endif()

find_program(AUTOMAKE automake)
if (NOT AUTOMAKE)
	message(FATAL_ERROR, "To build xffm+ you also need automake.")
	return()
endif()
find_program(AUTOHEADER autoheader)
if (NOT AUTOHEADER)
	message(FATAL_ERROR, "To build xffm+ you also need autoheader.")
	return()
endif()
find_program(AUTOCONF autoconf)
if (NOT AUTOCONF)
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


FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    MESSAGE("------
    NOTE: msgfmt not found. Translations will *not* be installed
------")
else(NOT GETTEXT_MSGFMT_EXECUTABLE)

  SET(catalogname xffm+)

  FILE(GLOB PO_FILES po/*.po)
  SET(GMO_FILES)
  MESSAGE("creating gmo files")
  foreach(_poFile ${PO_FILES})
    GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
#    MESSAGE("${_poFile} --> ${_poFileName}")
    STRING(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
    STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

    if( _langCode )
      GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
#      MESSAGE("file: ${_gmoFile}")

      ADD_CUSTOM_COMMAND(OUTPUT ${_gmoFile}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
        DEPENDS ${_poFile})
      INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
      LIST(APPEND GMO_FILES ${_gmoFile})
    endif( _langCode )

  endforeach(_poFile ${PO_FILES})

  ADD_CUSTOM_TARGET(translations ALL DEPENDS ${GMO_FILES})

endif(NOT GETTEXT_MSGFMT_EXECUTABLE)


# configure header file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.cmake"
    "${PROJECT_BINARY_DIR}/config.h"
    )
#add_subdirectory(po)

add_subdirectory(xffm)
#add_subdirectory(structure)


