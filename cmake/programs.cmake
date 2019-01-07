
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


message(STATUS "Checking for grep command...")
find_program(HAVE_GREP grep )
if(NOT HAVE_GREP)
    set(HAVE_GREP "HAVE_GREP_NOTFOUND")
    message( FATAL_ERROR "grep is not found" )
else()
    set(HAVE_GREP "HAVE_GREP 1")
    message( STATUS "grep was found. Is it GNU grep?" )
endif()

