
find_package(PkgConfig)
pkg_check_modules(GTK4 gtk4)

set(VERSION_OK TRUE)
if (GTK4_VERSION)
    if (GTK4_FIND_VERSION_EXACT)
        if (NOT("${GTK4_FIND_VERSION}" VERSION_EQUAL "${GTK4_VERSION}"))
            set(VERSION_OK FALSE)
        endif ()
    else ()
        if ("${GTK4_VERSION}" VERSION_LESS "${GTK4_FIND_VERSION}")
            set(VERSION_OK FALSE)
        endif ()
    endif ()
endif ()


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK4 DEFAULT_MSG GTK4_INCLUDE_DIRS GTK4_LIBRARIES VERSION_OK)
