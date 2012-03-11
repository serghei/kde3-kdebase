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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES keys.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kaccel.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS convertShortcuts.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )

install( FILES
    wm3.kksrc kde4.kksrc win3.kksrc
    win4.kksrc mac4.kksrc unix3.kksrc
  DESTINATION ${DATA_INSTALL_DIR}/kcmkeys )


##### kcm_keys (module) #########################

kde_add_kpart( kcm_keys AUTOMOC
  SOURCES
    shortcuts.cpp modifiers.cpp main.cpp khotkeys.cpp
    treeview.cpp commandShortcuts.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
