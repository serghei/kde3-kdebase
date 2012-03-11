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
  ${CMAKE_SOURCE_DIR}/kioslave/media/libmediacommon
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kio_media_mounthelper (executable) ########

set( target kio_media_mounthelper )

set( ${target}_SRCS
  kio_media_mounthelper.cpp decryptdialog.ui
)

kde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK mediacommon-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
