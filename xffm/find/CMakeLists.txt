    
add_executable(xffind xffind.cc)
target_link_libraries(xffind 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt)
install(TARGETS xffind RUNTIME DESTINATION bin)
