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

add_subdirectory( sounds )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmnotify.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES play.png DESTINATION ${DATA_INSTALL_DIR}/kcontrol/pics )


##### kcm_knotify (module) ######################

kde_add_kpart( kcm_knotify AUTOMOC
  SOURCES knotify.cpp playersettings.ui
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
