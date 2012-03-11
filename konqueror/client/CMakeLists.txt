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
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/../../kdesktop
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kfmclient_3_2.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )
install( PROGRAMS kfmclient_3_2_update.sh DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


##### kfmclient (kdeinit) #######################

set( target kfmclient )

set( ${target}_SRCS
  kfmclient.cc
  ../KonquerorIface.stub
  ../../kdesktop/KDesktopIface.stub
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kio-shared
)
