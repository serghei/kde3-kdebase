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


##### other data ################################

kde_install_icons( )
install( FILES knetattach.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### knetattach (executable) ###################

kde_add_executable( knetattach AUTOMOC
  SOURCES knetattach.ui main.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
