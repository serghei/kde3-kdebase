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

install( FILES kwinoptions.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    kwinactions.desktop kwinadvanced.desktop kwinfocus.desktop
    kwinmoving.desktop kwintranslucency.desktop
  DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_kwinoptions (module) ##################

kde_add_kpart( kcm_kwinoptions AUTOMOC
  SOURCES windows.cpp mouse.cpp main.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
