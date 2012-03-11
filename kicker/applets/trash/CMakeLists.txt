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

if( NOT BUILD_LIBKONQ )
  include( "${TDE_CMAKE_DIR}/libkonq.cmake" )
endif( NOT BUILD_LIBKONQ )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES trashapplet.desktop DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### trash_panelapplet (module) ################

kde_add_kpart( trash_panelapplet AUTOMOC
  SOURCES trashbutton.cpp trashapplet.cpp
  LINK kickermain-shared konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
