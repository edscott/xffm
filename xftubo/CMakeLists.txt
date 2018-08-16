
    
add_executable(xftubo src/tubo_main.c src/tubo.h src/tubo_exec.c src/tubo_exec.i)
target_link_libraries(xftubo -lpthread -lrt ${GLIB_LIBRARIES})
install(TARGETS xftubo RUNTIME DESTINATION bin)
