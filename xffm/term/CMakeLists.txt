    
add_executable(xfterm xfterm.cc)
target_link_libraries(xfterm 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt)
install(TARGETS xfterm RUNTIME DESTINATION bin)
INSTALL(CODE "execute_process( \
    COMMAND ${CMAKE_COMMAND} -E create_symlink \
    xfterm \
    xfgetpass   \
    )"
)
    install(CODE "
    EXECUTE_PROCESS(COMMAND ln -sf xfterm xfgetpass
       WORKING_DIRECTORY ${PREFIX_BIN}
       )
    ")
