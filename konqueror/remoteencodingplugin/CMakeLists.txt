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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    kremoteencodingplugin.rc kremoteencodingplugin.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqiconview/kpartplugins )

install( FILES
    kremoteencodingplugin.rc kremoteencodingplugin.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqlistview/kpartplugins )


##### konq_remoteencoding (module) ##############

set( target konq_remoteencoding )

set( ${target}_SRCS
  kremoteencodingplugin.cpp
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
