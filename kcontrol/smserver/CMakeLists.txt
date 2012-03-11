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

install( FILES kcmsmserver.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_smserver (module) #####################

kde_add_kpart( kcm_smserver AUTOMOC
  SOURCES kcmsmserver.cpp smserverconfigdlg.ui smserverconfigimpl.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
