


# mimeinfo is now replaced by dev-perl/File-MimeInfo
message(STATUS "Checking for libmagic...")
find_library(LIBMAGIC NAMES libmagic.so)
if(NOT LIBMAGIC)
    set(LIBMAGIC "LIBMAGIC_NOTFOUND")
    message("*** libmagic is not found: Look for it in *file* package." )
else()
    set(LIBMAGIC "HAVE_LIBMAGIC")
    message(STATUS "libmagic was found." )
    set(LIBMAGIC_LIBRARIES "-lmagic")
endif()

