#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( themeengine )
add_subdirectory( pics )
add_subdirectory( kcmksplash )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/ksplashml/themeengine
  ${CMAKE_SOURCE_DIR}/ksplashml/themeengine/default
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ksplash.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### ksplash (executable) ######################

kde_add_executable( ksplash AUTOMOC
  SOURCES wndmain.cpp ksplashiface.skel main.cpp
  LINK themedefault-static kparts-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
