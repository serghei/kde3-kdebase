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
  ${CMAKE_CURRENT_BINARY_DIR}/../libmediacommon
  ${CMAKE_CURRENT_SOURCE_DIR}/../libmediacommon
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES medianotifier.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_medianotifier (module) ###############

set( target kded_medianotifier )

set( ${target}_SRCS
  medianotifier.cpp medianotifier.skel notificationdialog.cpp
  notificationdialogview.ui
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK mediacommon-static kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
