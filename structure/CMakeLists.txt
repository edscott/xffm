
add_executable(structure structure.cc)
include_directories(
    "./"
    "../"
    "${PROJECT_BINARY_DIR}" 
    "${GTK3_INCLUDE_DIRS}")

target_link_libraries(structure 
    ${GTK3_LIBRARIES}
    ${X11_LIBRARIES}
    -lpthread -lrt -lm
    )

install(TARGETS structure  RUNTIME DESTINATION bin)
INSTALL(PROGRAMS  parse9b.pl  DESTINATION bin)


