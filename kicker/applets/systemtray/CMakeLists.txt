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
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES systemtrayapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### systemtray_panelapplet (module) ###########

kde_add_kpart( systemtray_panelapplet AUTOMOC
  SOURCES systemtrayapplet.cpp systemtrayapplet.skel
  LINK kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
  DEPENDENCIES kicker_core-static kicker-static kicker-shared
)
