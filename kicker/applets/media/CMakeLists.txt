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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES mediaapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### media_panelapplet (module) ################

kde_add_kpart( media_panelapplet AUTOMOC
  SOURCES preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp
  LINK kickermain-shared konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
