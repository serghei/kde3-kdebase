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

install( FILES
    kfile_trash.desktop kfile_trash_system.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )


##### kfile_trash (module) ######################

set( target kfile_trash )

kde_add_kpart( ${target} AUTOMOC
  SOURCES kfile_trash.cpp
  LINK trashcommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
