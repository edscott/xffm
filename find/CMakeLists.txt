    
add_executable(xffind src/xffind.cc)
target_link_libraries(xffind 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt)
install(TARGETS xffind RUNTIME DESTINATION bin)
