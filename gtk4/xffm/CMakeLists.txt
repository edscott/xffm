
add_subdirectory(icons)

add_executable(xffm xffm.cc)
#target_link_libraries(ProjectCESGI PRIVATE PkgConfig::GTK4)
include_directories(
    "./"
    "../"
    "${PROJECT_BINARY_DIR}" 
    "${GTK4_INCLUDE_DIRS}")

target_link_libraries(xffm 
    ${HISTORY_LIBRARIES}
    ${GTK4_LIBRARIES}
    ${X11_LIBRARIES}
    ${LIBZIP_LIBRARIES}
    ${LIBMAGIC_LIBRARIES}
    ${THREAD_LIBRARIES})

#execute_process(COMMAND ln -sf xffm xfgetpass WORKING_DIRECTORY xffm)


install(TARGETS xffm RUNTIME DESTINATION bin)
INSTALL(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    xffm \
    xfgetpass   \
    )"
)
    install(CODE "
    EXECUTE_PROCESS(COMMAND ln -sf xffm xfgetpass
       WORKING_DIRECTORY ${PREFIX_BIN}
       )
    ")
