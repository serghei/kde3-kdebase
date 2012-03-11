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

# FIXME seems that on freebsd is needed smth named LIB_KVM

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES naughtyapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES naughty-happy.png naughty-sad.png DESTINATION ${DATA_INSTALL_DIR}/naughtyapplet/pics )


##### naughty_panelapplet (module) ##############

kde_add_kpart( naughty_panelapplet AUTOMOC
  SOURCES
    NaughtyProcessMonitor.cpp NaughtyConfigDialog.cpp
    NaughtyApplet.cpp
  LINK kickermain-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
