

add_executable(xfgetpass4 xffm.cc)
add_executable(xffm4 xffm.cc)
#add_executable(xffm xffm.cc)
#target_link_libraries(ProjectCESGI PRIVATE PkgConfig::GTK4)
include_directories(
    "./"
    "../"
    "${PROJECT_BINARY_DIR}" 
    "${libRSVG_INCLUDE_DIRS}"
    "${libReadline_INCLUDE_DIRS}"
    "${GTK4_INCLUDE_DIRS}")

target_link_libraries(xffm4 
    ${HISTORY_LIBRARIES}
    ${libRSVG_LIBRARIES}
    ${GTK4_LIBRARIES}
    ${X11_LIBRARIES}
    ${LIBZIP_LIBRARIES}
    ${libMagic_LIBRARIES}
    ${libReadline_LIBRARIES}
    ${THREAD_LIBRARIES})

target_link_libraries(xfgetpass4 
    ${HISTORY_LIBRARIES}
    ${libRSVG_LIBRARIES}
    ${GTK4_LIBRARIES}
    ${X11_LIBRARIES}
    ${LIBZIP_LIBRARIES}
    ${libMagic_LIBRARIES}
    ${libReadline_LIBRARIES}
    ${THREAD_LIBRARIES})


#execute_process(COMMAND cp xffm4 xfgetpass4 WORKING_DIRECTORY xffm)


#install(TARGETS xffm4 RUNTIME DESTINATION bin)
install(TARGETS  xffm4 xfgetpass4 DESTINATION bin)

#INSTALL(CODE "execute_process( \
#    COMMAND ${CMAKE_COMMAND} -E create_symlink \
#    xffm \
#    xfgetpass   \
#    )"
#)
