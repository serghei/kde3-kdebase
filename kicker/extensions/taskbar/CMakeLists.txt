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
  ${CMAKE_SOURCE_DIR}/kicker/taskbar
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES taskbarextension.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/extensions )


##### taskbar_panelextension (module) ###########

set( target taskbar_panelextension )

set( ${target}_SRCS
  taskbarextension.cpp taskbarextension.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kickermain-shared taskmanager-shared taskbar-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
