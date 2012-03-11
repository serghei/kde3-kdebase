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

add_subdirectory( kcm )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kdepasswd.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kdepasswd (executable) ####################

kde_add_executable( kdepasswd AUTOMOC
  SOURCES kdepasswd.cpp passwd.cpp passwddlg.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
