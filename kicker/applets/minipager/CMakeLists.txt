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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/taskmanager
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES minipagerapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### minipager_panelapplet (module) ############

set( target minipager_panelapplet )

set( ${target}_SRCS
  pagerapplet.cpp pagerbutton.cpp pagersettings.kcfgc
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK taskmanager-shared kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
