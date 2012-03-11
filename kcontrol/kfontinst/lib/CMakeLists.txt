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

add_definitions( -D_LARGEFILE64_SOURCE )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
)


##### kfontinst (shared) ########################

kde_add_library( kfontinst SHARED
  SOURCES Misc.cpp FcEngine.cpp
  VERSION 0.0.0
  LINK kio-shared ${XFT_LIBRARIES} ${FONTCONFIG_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
