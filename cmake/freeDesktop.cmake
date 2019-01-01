
message(STATUS "Checking for freedesktop.org.xml globs...")
find_file(FREEDESKTOP_GLOBS  NAMES globs 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_GLOBS)
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml globs was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS \"${FREEDESKTOP_GLOBS}\"")
    message( STATUS  "freedesktop.org.xml globs was found." )
endif()

message(STATUS "Checking for freedesktop.org.xml aliases...")
find_file(FREEDESKTOP_ALIAS  NAMES aliases 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_ALIAS)
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml aliases was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS \"${FREEDESKTOP_ALIAS}\" ")
    message( STATUS  "freedesktop.org.xml aliases was found." )
endif()

message(STATUS "Checking for freedesktop.org.xml generic-icons...")
find_file(FREEDESKTOP_ICONS  NAMES generic-icons 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    )
if(NOT FREEDESKTOP_ICONS)
    set(FREEDESKTOP_ICONS  "FREEDESKTOP_ICONS_NOTFOUND")
    message( WARNING "File freedesktop.org.xml generic-icons was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ICONS "FREEDESKTOP_ICONS \"${FREEDESKTOP_ICONS}\" ")
    message( STATUS  "freedesktop.org.xml generic-icons was found." )
endif()


