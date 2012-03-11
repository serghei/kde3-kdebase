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
  ${CMAKE_BINARY_DIR}/kioslave/media/libmediacommon
  ${CMAKE_SOURCE_DIR}/kioslave/media/libmediacommon
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES media.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_media (module) ########################

set( target kcm_media )

set( ${target}_SRCS
  notifiermodule.cpp notifiermoduleview.ui serviceconfigdialog.cpp
  serviceview.ui mimetypelistboxitem.cpp managermodule.cpp
  managermoduleview.ui main.cpp
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK mediacommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
