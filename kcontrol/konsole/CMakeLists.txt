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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmkonsole.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_konsole (module) ######################

kde_add_kpart( kcm_konsole AUTOMOC
  SOURCES
    schemaeditor.cpp sessioneditor.cpp kcmkonsole.cpp
    schemadialog.ui sessiondialog.ui kcmkonsoledialog.ui
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
