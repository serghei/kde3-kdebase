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

install( FILES konsolemenu.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )


##### kickermenu_konsole (module) ###############

kde_add_kpart( kickermenu_konsole AUTOMOC
  SOURCES konsole_mnu.cpp konsolebookmarkhandler.cpp konsolebookmarkmenu.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
