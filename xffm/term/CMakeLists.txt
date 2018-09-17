    
add_executable(xfterm xfterm.cc)
target_link_libraries(xfterm 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt)
install(TARGETS xfterm RUNTIME DESTINATION bin)
