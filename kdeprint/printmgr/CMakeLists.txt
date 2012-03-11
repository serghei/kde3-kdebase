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

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( printmgr )
install( FILES printers.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_printmgr (module) #####################

kde_add_kpart( kcm_printmgr AUTOMOC
  SOURCES kcmprintmgr.cpp
  LINK kdeprint_management-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
