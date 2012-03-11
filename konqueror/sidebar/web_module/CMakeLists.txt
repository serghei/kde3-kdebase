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
  ${CMAKE_SOURCE_DIR}/konqueror/sidebar
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES webmodule_add.desktop DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/add )
install( FILES websidebar.html DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/websidebar )


##### konqsidebar_web (module) ##################

set( target konqsidebar_web )

kde_add_kpart( ${target} AUTOMOC
  SOURCES web_module.cpp
  LINK konqsidebarplugin-shared konq-shared khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
