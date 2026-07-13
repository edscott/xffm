
message(STATUS "Checking for freedesktop.org globs...")
find_file(FREEDESKTOP_GLOBS  NAMES globs 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    /data/data/com.termux/files/usr/share/mime
    )
if(NOT FREEDESKTOP_GLOBS)
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS")
    message( WARNING "File freedesktop.org globs was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_GLOBS "FREEDESKTOP_GLOBS \"${FREEDESKTOP_GLOBS}\"")
    message( STATUS  "freedesktop.org globs was found." )
endif()

message(STATUS "Checking for freedesktop.org aliases...")
find_file(FREEDESKTOP_ALIAS  NAMES aliases 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    /data/data/com.termux/files/usr/share/mime
    )
if(NOT FREEDESKTOP_ALIAS)
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS")
    message( WARNING "File freedesktop.org aliases was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ALIAS "FREEDESKTOP_ALIAS \"${FREEDESKTOP_ALIAS}\" ")
    message( STATUS  "freedesktop.org aliases was found." )
endif()

message(STATUS "Checking for freedesktop.org generic-icons...")
find_file(FREEDESKTOP_ICONS  NAMES generic-icons 
    PATHS
    /usr/share/mime 
    /usr/local/share/mime 
    /data/data/com.termux/files/usr/share/mime
    )
if(NOT FREEDESKTOP_ICONS)
    set(FREEDESKTOP_ICONS  "FREEDESKTOP_ICONS")
    message( WARNING "File freedesktop.org generic-icons was not found.")
    message( WARNING "Mimetype functionality may be limited." )
else()
    set(FREEDESKTOP_ICONS "FREEDESKTOP_ICONS \"${FREEDESKTOP_ICONS}\" ")
    message( STATUS  "freedesktop.org generic-icons was found." )
endif()


