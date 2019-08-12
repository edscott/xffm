# Setting up Intl
find_package (Intl REQUIRED)
find_package(Gettext REQUIRED)
include_directories(${INTL_INCLUDE_DIRS})
link_directories(${INTL_LIBRARY_DIRS})
#message("INTL_INCLUDE_DIRS -> ${INTL_INCLUDE_DIRS}")
#message("INTL_LIBRARY_DIRS -> ${INTL_LIBRARY_DIRS}")

#FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
#message("GETTEXT_MSGFMT_EXECUTABLE -> ${GETTEXT_MSGFMT_EXECUTABLE}")
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    MESSAGE("------
    NOTE: msgfmt not found. Translations will *not* be installed: You will need gettext package for that.
------")
else(NOT GETTEXT_MSGFMT_EXECUTABLE)

  SET(GETTEXT_PACKAGE xffm+)
  SET(catalogname xffm+)
  set(ENABLE_NLS "#define ENABLE_NLS")
  set(LOCALE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALEDIR}" )

  FILE(GLOB PO_FILES po/*.po)
  SET(GMO_FILES)
  foreach(_poFile ${PO_FILES})
    GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
#    MESSAGE("${_poFile} --> ${_poFileName}")
    STRING(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
    STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

    if( _langCode )
      GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
#      MESSAGE("file: ${_gmoFile}")

      ADD_CUSTOM_COMMAND(OUTPUT ${_gmoFile}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
        DEPENDS ${_poFile})
      INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
      LIST(APPEND GMO_FILES ${_gmoFile})
    endif( _langCode )

  endforeach(_poFile ${PO_FILES})

  ADD_CUSTOM_TARGET(translations ALL DEPENDS ${GMO_FILES})

endif(NOT GETTEXT_MSGFMT_EXECUTABLE)

