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


##### kdecorations (shared) #####################

set( target kdecorations )

set( ${target}_SRCS
  kdecoration.cpp kdecoration_p.cpp kdecoration_plugins_p.cpp
  kdecorationfactory.cpp kcommondecoration.cpp
)

kde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kdecore-shared dl
  DESTINATION ${LIB_INSTALL_DIR}
)

##### headers ###################################

install( FILES kdecoration.h kdecoration_p.h kdecoration_plugins_p.h kdecorationfactory.h kcommondecoration.h
         DESTINATION ${INCLUDE_INSTALL_DIR} )
