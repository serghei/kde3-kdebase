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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES menuapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### menu_panelapplet (module) #################

kde_add_kpart( menu_panelapplet AUTOMOC
  SOURCES menuapplet.cpp menuapplet.skel
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
