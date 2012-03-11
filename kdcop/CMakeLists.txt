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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kdcopui.rc DESTINATION ${DATA_INSTALL_DIR}/kdcop )


##### kdcop (executable) ########################

kde_add_executable( kdcop AUTOMOC
  SOURCES
    kdcop.cpp kdcopwindow.cpp kdcoplistview.cpp kdcopview.ui
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
