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
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/kicker/ui
  ${CMAKE_SOURCE_DIR}/kicker/kicker/core
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES quicklauncher.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )
install( FILES launcherapplet.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### launcher_panelapplet (module) #############

# NOTE: There we have a circular dependency,
#       for this reason I used -Wl,--start-group / -Wl,--end-group

kde_add_kpart( launcher_panelapplet AUTOMOC
  SOURCES
    quicklauncher.skel quicklauncher.cpp quickbutton.cpp
    quickaddappsmenu.cpp flowgridmanager.cpp popularity.cpp
    configdlgbase.ui prefs.kcfgc configdlg.cpp
  EMBED
    kicker_core-static kicker_buttons-static kicker_ui-static
  LINK
    dmctl-static kickermain-shared kutils-shared kabc-shared
  DEPENDENCIES libkicker-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
