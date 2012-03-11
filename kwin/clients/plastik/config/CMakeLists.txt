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

install( FILES  DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kwin_plastik_config (module) ##############

kde_add_kpart( kwin_plastik_config AUTOMOC
  SOURCES config.cpp configdialog.ui
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
