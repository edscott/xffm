


# mimeinfo is now replaced by dev-perl/File-MimeInfo
message(STATUS "Checking for librt...")
find_library(LIBRT NAMES librt.so librt.so.1)
if(NOT LIBRT)
    set(THREAD_LIBRARIES "-lpthread")
    message(STATUS "pthread libraries: -pthread" )
else()
   set(THREAD_LIBRARIES "-lpthread -lrt")
    message(STATUS "pthread libraries: -lpthread -lrt" )
endif()

