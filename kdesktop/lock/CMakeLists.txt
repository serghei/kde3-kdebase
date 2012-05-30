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
  ${CMAKE_BINARY_DIR}/kdesktop
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${CMAKE_SOURCE_DIR}/kcheckpass
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kdesktop_lock (executable) ################

set( target kdesktop_lock )

set( ${target}_SRCS
  lockprocess.cc lockdlg.cc
  autologout.cc main.cc
)

kde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdesktopsettings-static dmctl-static kio-shared ${XEXT_LIBRARIES} ${GL_LIBRARY}
  DESTINATION ${BIN_INSTALL_DIR}
)
