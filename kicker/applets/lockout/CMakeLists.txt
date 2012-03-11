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
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES lockout.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### lockout_panelapplet (module) ##############

kde_add_kpart( lockout_panelapplet AUTOMOC
  SOURCES lockout.cpp
  LINK kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
