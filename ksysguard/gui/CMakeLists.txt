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

add_subdirectory( ksgrd )
add_subdirectory( SensorDisplayLib )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/SensorDisplayLib
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ksgrd
  ${CMAKE_CURRENT_SOURCE_DIR}/SensorDisplayLib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES ksysguard.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES x-ksysguard.desktop DESTINATION ${MIME_INSTALL_DIR}/application )
install( FILES ProcessTable.sgrd SystemLoad.sgrd KSysGuardApplet.xml ksysguardui.rc DESTINATION ${DATA_INSTALL_DIR}/ksysguard )
install( FILES ksysguardapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### sysguard_panelapplet (module) #############

kde_add_kpart( sysguard_panelapplet AUTOMOC
  SOURCES KSysGuardApplet.cc KSGAppletSettings.cc
  LINK sensordisplays-static ksgrd-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### ksysguard (executable) ####################

kde_add_executable( ksysguard AUTOMOC
  SOURCES
    SensorBrowser.cc WorkSheet.cc WorkSheetSettings.cc
    Workspace.cc ksysguard.cc ksysguard.skel
  LINK sensordisplays-static ksgrd-shared kdnssd-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kpm (executable) ##########################

kde_add_executable( kpm
  SOURCES kpm.c
  DESTINATION ${BIN_INSTALL_DIR}
)
