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


##### kwin_keramik_config (module) ##############

kde_add_kpart( kwin_keramik_config AUTOMOC
  SOURCES config.cpp keramikconfig.ui
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
