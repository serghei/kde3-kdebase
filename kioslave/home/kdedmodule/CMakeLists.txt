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

install( FILES homedirnotify.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_homedirnotify (module) ###############

set( target kded_homedirnotify )

set( ${target}_SRCS
  homedirnotify.cpp homedirnotify.skel
  homedirnotifymodule.cpp homedirnotifymodule.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
