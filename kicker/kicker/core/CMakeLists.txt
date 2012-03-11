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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/kicker/ui
  ${CMAKE_SOURCE_DIR}/kicker/kicker/buttons
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES default-apps DESTINATION ${DATA_INSTALL_DIR}/kicker )
install( FILES childpanelextension.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/extensions )


##### kicker_core (static) ######################

kde_add_library( kicker_core STATIC_PIC AUTOMOC
  SOURCES
    extensionSettings.kcfgc main.cpp kicker.cpp kicker.skel
    userrectsel.cpp containerarea.cpp applethandle.cpp container_base.cpp
    container_button.cpp container_applet.cpp container_extension.cpp
    extensionmanager.cpp menumanager.cpp pluginmanager.cpp
    showdesktop.cpp unhidetrigger.cpp containerarealayout.cpp
    panelextension.cpp panelextension.skel
  DEPENDENCIES libkicker-shared
  LINK konq-shared
)
