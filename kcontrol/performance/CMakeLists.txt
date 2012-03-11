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

install( FILES kcmperformance.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kcmkonqyperformance.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_performance (module) ##################

kde_add_kpart( kcm_performance AUTOMOC
  SOURCES kcmperformance.cpp konqueror.cpp konqueror_ui.ui system.cpp system_ui.ui
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
