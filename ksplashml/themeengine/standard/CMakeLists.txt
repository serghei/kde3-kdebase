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

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/ksplashml/themeengine
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES Theme.rc DESTINATION ${DATA_INSTALL_DIR}/ksplash/Themes/Standard )
install( FILES ksplashstandard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### ksplashstandard (module) ##################

kde_add_kpart( ksplashstandard AUTOMOC
  SOURCES themestandardlib.cpp
  LINK themestandard-static
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### themestandard (static) ####################

kde_add_library( themestandard STATIC_PIC AUTOMOC
  SOURCES themestandard.cpp wndicon.cpp wndstatus.cpp
  LINK ksplashthemes-shared
)
