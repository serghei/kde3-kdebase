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
  ${CMAKE_CURRENT_SOURCE_DIR}/../libmediacommon
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kio_media_mounthelper (executable) ########

kde_add_executable( kio_media_mounthelper AUTOMOC
  SOURCES kio_media_mounthelper.cpp
  LINK mediacommon-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
