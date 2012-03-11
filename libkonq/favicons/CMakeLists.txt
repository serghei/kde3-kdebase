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

install( FILES favicons.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES favicons.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS move_favicons.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### kded_favicons (module) ####################

kde_add_kpart( kded_favicons AUTOMOC
  SOURCES favicons.cpp favicons.skel
  LINK kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
