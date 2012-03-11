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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmtaskbar.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Settings/LookNFeel RENAME kcmtaskbar.desktop )


##### kcm_taskbar (module) ######################

add_custom_command( OUTPUT taskbarsettings.h
  COMMAND ${KDE_KCFGC_EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/kicker/taskbar/taskbar.kcfg
    ${CMAKE_SOURCE_DIR}/kicker/taskbar/taskbarsettings.kcfgc
  DEPENDS
    ${CMAKE_SOURCE_DIR}/kicker/taskbar/taskbar.kcfg
    ${CMAKE_SOURCE_DIR}/kicker/taskbar/taskbarsettings.kcfgc )

set_source_files_properties( kcmtaskbar.cpp
  PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/taskbarsettings.h )

kde_add_kpart( kcm_taskbar AUTOMOC
  SOURCES kcmtaskbarui.ui kcmtaskbar.cpp
  LINK taskbar-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
