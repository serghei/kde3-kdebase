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
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/kicker/menuext )


##### appletproxy (kdeinit) #####################

kde_add_kdeinit_executable( appletproxy AUTOMOC
  SOURCES appletproxy.cpp
  LINK kickermain-shared
)


##### extensionproxy (kdeinit) ##################

kde_add_kdeinit_executable( extensionproxy AUTOMOC
  SOURCES extensionproxy.cpp
  LINK kickermain-shared
)
