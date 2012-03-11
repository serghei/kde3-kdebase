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


##### kdialog (executable) ######################

kde_add_executable( kdialog AUTOMOC
  SOURCES
    kdialog.cpp widgets.cpp klistboxdialog.cpp
    progressdialog.cpp progressdialogiface.skel
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
