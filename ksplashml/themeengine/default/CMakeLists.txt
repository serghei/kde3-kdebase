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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  #${CMAKE_CURRENT_SOURCE_DIR}
  #${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/ksplashml/themeengine
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ksplashdefault.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### ksplashdefault (module) ###################

kde_add_kpart( ksplashdefault AUTOMOC
  SOURCES main.cpp
  LINK themedefault-static ksplashthemes-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### themedefault (static) #####################

kde_add_library( themedefault STATIC_PIC AUTOMOC
  SOURCES themelegacy.cpp
  LINK ksplashthemes-shared
)
