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
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES remotemenu.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )


##### kickermenu_remotemenu (module) ############

kde_add_kpart( kickermenu_remotemenu AUTOMOC
  SOURCES remotemenu.cpp remotemenu.skel
  LINK kickermain-shared kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
