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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES icons.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Settings/LookNFeel/Themes RENAME iconthemes.desktop )


##### kcm_icons (module) ########################

kde_add_kpart( kcm_icons AUTOMOC
  SOURCES iconthemes.cpp icons.cpp main.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
