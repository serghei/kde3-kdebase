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
  ${CMAKE_SOURCE_DIR}/ksplashml/themeengine
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES Theme.rc DESTINATION ${DATA_INSTALL_DIR}/ksplash/Themes/Redmond )
install( FILES ksplashredmond.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### ksplashredmond (module) ###################

kde_add_kpart( ksplashredmond AUTOMOC
  SOURCES previewredmond.cpp themeredmond.cpp
  LINK ksplashthemes-shared kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
