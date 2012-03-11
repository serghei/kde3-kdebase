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

add_subdirectory( componentservices )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES componentchooser.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_componentchooser (module) #############

kde_add_kpart( kcm_componentchooser AUTOMOC
  SOURCES
    componentchooser_ui.ui componentconfig_ui.ui
    emailclientconfig_ui.ui terminalemulatorconfig_ui.ui
    browserconfig_ui.ui componentchooser.cpp kcm_componentchooser.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
