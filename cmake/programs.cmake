
message(STATUS "Checking for gvim...")
find_program(GVIM_PROGRAM gvim)
if(NOT GVIM_PROGRAM)
    set(GVIM_PROGRAM "GVIM_PROGRAM_NOTFOUND")
    message( STATUS "Gvim program was not found.")
else()
    message( STATUS "${GVIM_PROGRAM} was found. Good!" )
    set(GVIM_PROGRAM "GVIM_PROGRAM \"${GVIM_PROGRAM}\" ")
endif()

message(STATUS "Checking for perl5 File-MimeInfo...")
find_program(MIMETYPE_PROGRAM mimetype)
if(NOT MIMETYPE_PROGRAM)
    set(MIMETYPE_PROGRAM "MIMETYPE_PROGRAM_NOTFOUND")
    message( STATUS  "*** Perl mimetype program was not found.")
#   if libmagic was not found, then no mimetype information will be available
 if (NOT LIBMAGIC_FOUND)
	message(FATAL_ERROR "either install perl mimetype program or libmagic or both")
 endif()
else()
    message( STATUS "${MIMETYPE_PROGRAM} was found. Good!" )
    set(MIMETYPE_PROGRAM "MIMETYPE_PROGRAM \"${MIMETYPE_PROGRAM}\" ")
#   if libmagic is found, encoding information will also be available
endif()


message(STATUS "Checking for grep command...")
find_program(HAVE_GREP grep )
if(NOT HAVE_GREP)
    set(HAVE_GREP "GREP_NOTFOUND")
    message( FATAL_ERROR "grep is not found" )
else()
    set(HAVE_GREP "HAVE_GREP")
    message( STATUS "grep was found. Is it GNU grep?" )
endif()



message(STATUS "Checking for pkg command...")
find_program(HAVE_PACMAN pacman )
if(NOT HAVE_PACMAN)
    set(HAVE_PACMAN "PACMAN_NOTFOUND")
#	message( STATUS "pacman is not found" )
#	message(STATUS "Checking for emerge command...")
    find_program(HAVE_EMERGE emerge )
    if(NOT HAVE_EMERGE)
        set(HAVE_EMERGE "EMERGE_NOTFOUND")
#	    message( STATUS "emerge is not found" )
#	    message(STATUS "Checking for pkg command...")
        find_program(HAVE_PKG pkg )
        if(NOT HAVE_PKG)
            set(HAVE_PKG "PKG_NOTFOUND")
#		message( STATUS "pkg is not found" )
        else()
            set(HAVE_PKG "HAVE_PKG")
#		message( STATUS "Software Updater will be built for pkg." )
            message( STATUS "Found pkg." )
        endif()
    else()
        set(HAVE_PKG "PKG_SKIPPED")
        set(HAVE_EMERGE "HAVE_EMERGE")
#	    message( STATUS "Software Updater will be built for emerge." )
        message( STATUS "Found emerge." )
    endif()
else()
    set(HAVE_PACMAN "HAVE_PACMAN")
    set(HAVE_EMERGE "EMERGE_SKIPPED")
    set(HAVE_PKG "PKG_SKIPPED")
#	message( STATUS "Software Updater will be built for pacman." )
    message( STATUS "Found pacman." )
endif()

