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
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kwindecoration.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kwindecoration (module) ###############

kde_add_kpart( kcm_kwindecoration AUTOMOC
  SOURCES
    kwindecoration.cpp buttons.cpp
    kwindecorationIface.skel preview.cpp
  LINK kdecorations-shared kdeui-shared Xext
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
