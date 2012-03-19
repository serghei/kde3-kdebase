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

kde_import( libkonq )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES sidebarextension.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/extensions )


##### sidebar_panelextension (module) ###########

kde_add_kpart( sidebar_panelextension AUTOMOC
  SOURCES sidebarextension.cpp
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
