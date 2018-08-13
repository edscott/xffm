cmake_minimum_required (VERSION 3.1)
project (xffind CXX)
#set(CMAKE_INSTALL_PREFIX /tmp)
# Version info
set(xffind_VERSION_MAJOR 0)
set(xffind_VERSION_MINOR 3)
set(ENV{PKG_CONFIG_PATH}  "/usr/local/lib/pkgconfig")
list(APPEND CMAKE_MODULE_PATH 
    "${PROJECT_SOURCE_DIR}/src/cmake")

include_directories(
    "../common/"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_BINARY_DIR}" 
    "${GTK3_INCLUDE_DIRS}"
    "${TUBO_INCLUDE_DIRS}"   )
#    "${X11_INCLUDE_DIRS}")
    
add_executable(xffind src/xffind.cc)
target_link_libraries(xffind 
    ${GTK3_LIBRARIES}
    ${TUBO_LIBRARIES}
    ${X11_LIBRARIES})
install(TARGETS xffind RUNTIME DESTINATION bin)
