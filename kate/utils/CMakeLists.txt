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


##### headers ###################################

install( FILES
    dockviewbase.h messageview.h listboxview.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kate/utils )


##### kateutils (shared lib) ####################

set( target kateutils )

set( ${target}_SRCS
  dockviewbase.cpp messageview.cpp listboxview.cpp )

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.0
  LINK kdeui-shared
  DESTINATION ${LIB_INSTALL_DIR} )
