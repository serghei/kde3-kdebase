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


##### other data ################################

install( FILES ioslaveinfo.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
kde_install_empty_directory( ${DATA_INSTALL_DIR}/Settings/Information )


##### kcm_ioslaveinfo (module) ##################

kde_add_kpart( kcm_ioslaveinfo AUTOMOC
  SOURCES kcmioslaveinfo.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
