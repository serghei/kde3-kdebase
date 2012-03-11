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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kcminit (kdeinit) #########################

kde_add_kdeinit_executable( kcminit AUTOMOC
  SOURCES main.cpp main.skel
  LINK kutils-shared
)


##### kcminit_startup (kdeinit) #################

kde_add_kdeinit_executable( kcminit_startup AUTOMOC
  SOURCES main.cpp main.skel
  LINK kutils-shared
)
