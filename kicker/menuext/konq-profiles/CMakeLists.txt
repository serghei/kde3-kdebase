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

install( FILES konquerormenu.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )


##### kickermenu_konqueror (module) #############

kde_add_kpart( kickermenu_konqueror AUTOMOC
  SOURCES konqy_menu.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
