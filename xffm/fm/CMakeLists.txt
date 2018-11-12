add_subdirectory(icons)
    
add_executable(xffm xffm.cc)
include_directories(
    "./xffm"
    "${PROJECT_BINARY_DIR}" 
    "${GTK3_INCLUDE_DIRS}" )
#    "${LIBXML2_INCLUDE_DIR}")
target_link_libraries(xffm 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt )
#    ${LIBXML2_LIBRARIES}
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
