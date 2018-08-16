    
add_executable(xffind src/xffind.cc)
target_link_libraries(xffind 
    ${GTK3_LIBRARIES}
    ${TUBO_LIBRARIES}
    ${X11_LIBRARIES})
install(TARGETS xffind RUNTIME DESTINATION bin)
