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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kdesu (executable) ########################

kde_add_executable( kdesu AUTOMOC
  SOURCES kdesu.cpp sudlg.cpp
  LINK kdesu-shared kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
