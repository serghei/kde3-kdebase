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
  ${CMAKE_BINARY_DIR}/kcontrol/background
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kdm.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kdm (module) ##########################

kde_add_kpart( kcm_kdm AUTOMOC
  SOURCES
    background.cpp kdm-appear.cpp kdm-font.cpp kdm-shut.cpp
    kdm-users.cpp kdm-conv.cpp main.cpp kbackedcombobox.cpp
  LINK kcmbgnd-static bgnd-static knewstuff-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
