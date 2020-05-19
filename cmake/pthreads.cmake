


# mimeinfo is now replaced by dev-perl/File-MimeInfo
message(STATUS "Checking for librt...")
find_library(LIBRT NAMES librt.so)
if(NOT LIBRT)
    set(THREAD_LIBRARIES "-lpthread")
    message("pthread libraries: -lpthread" )
else()
   set(THREAD_LIBRARIES "-lpthread -lrt")
    message("pthread libraries: -lpthread -lrt" )
endif()

