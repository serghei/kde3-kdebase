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

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( kjobviewer )
install( FILES kjobviewer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kjobviewerui.rc DESTINATION ${DATA_INSTALL_DIR}/kjobviewer )


##### kjobviewer (kdeinit) ######################

kde_add_kdeinit_executable( kjobviewer AUTOMOC
  SOURCES kjobviewer.cpp main.cpp
  LINK kdeprint_management-shared
)
