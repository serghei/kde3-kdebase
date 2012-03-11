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

install( FILES kcmcss.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES template.css DESTINATION ${DATA_INSTALL_DIR}/kcmcss )


##### kcm_css (module) ##########################

kde_add_kpart( kcm_css AUTOMOC
  SOURCES template.cpp cssconfig.ui csscustom.ui kcmcss.cpp preview.ui
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
