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


##### icons #######################

kde_install_icons()

##### other data ################################

install( FILES kpager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Utilities RENAME kpager.desktop )


##### kpager (executable) #######################

kde_add_executable( kpager AUTOMOC
  SOURCES
    desktop.cpp kpager.cpp config.cpp windowdrag.cpp
    kpagerIface.skel main.cpp
  LINK kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
