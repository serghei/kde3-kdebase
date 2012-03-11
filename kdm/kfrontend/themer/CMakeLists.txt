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
  ${CMAKE_CURRENT_BINARY_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kdmthemer (static) ########################

kde_add_library( kdmthemer STATIC_PIC AUTOMOC
  SOURCES
    kdmthemer.cpp kdmitem.cpp kdmpixmap.cpp
    kdmrect.cpp kdmlabel.cpp kdmlayout.cpp
  DEPENDENCIES config.ci-common
)
