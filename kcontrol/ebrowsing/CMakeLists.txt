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

add_subdirectory( plugins )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ebrowsing.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kurifilt (module) #####################

kde_add_kpart( kcm_kurifilt AUTOMOC
  SOURCES filteropts.cpp main.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
