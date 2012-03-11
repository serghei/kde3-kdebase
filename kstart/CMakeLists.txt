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


##### kstart (executable) #######################

kde_add_executable( kstart AUTOMOC
  SOURCES kstart.cpp
  LINK kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
