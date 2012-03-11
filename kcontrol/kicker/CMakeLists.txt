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

kde_import( kicker )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    panel.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    kicker_config.desktop kicker_config_arrangement.desktop
    kicker_config_hiding.desktop kicker_config_menus.desktop
    kicker_config_appearance.desktop
  DESTINATION ${APPS_INSTALL_DIR}/.hidden )

install(
  FILES uninstall.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/LookNFeel
  RENAME panel.desktop )

install(
  FILES uninstall.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/LookNFeel
  RENAME panel_appearance.desktop )

install(
  FILES uninstall.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR}
  RENAME panel_appearance.desktop )


##### kcm_kicker (module) #######################

set( target kcm_kicker )

add_custom_command( OUTPUT kickerSettings.h
  COMMAND ${KDE_KCFGC_EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/kicker/libkicker/kickerSettings.kcfg
    ${CMAKE_SOURCE_DIR}/kicker/libkicker/kickerSettings.kcfgc
  DEPENDS
    ${CMAKE_SOURCE_DIR}/kicker/libkicker/kickerSettings.kcfg
    ${CMAKE_SOURCE_DIR}/kicker/libkicker/kickerSettings.kcfgc )

set_source_files_properties( lookandfeeltab_impl.cpp
  PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kickerSettings.h )

set( ${target}_SRCS
  positiontab.ui hidingtab.ui lookandfeeltab.ui menutab.ui
  positiontab_impl.cpp hidingtab_impl.cpp lookandfeeltab_impl.cpp
  menutab_impl.cpp extensionInfo.cpp main.cpp main.skel
  advancedOptions.ui advancedDialog.cpp positionconfig.cpp
  hidingconfig.cpp menuconfig.cpp lookandfeelconfig.cpp
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK bgnd-static kickermain-shared kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
