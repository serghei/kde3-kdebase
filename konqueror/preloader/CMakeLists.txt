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

# FIXME there we have some checks for malloc

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/konqueror
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES konqy_preloader.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES konqy_preload.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )


##### kded_konqy_preloader (module) #############

set( target kded_konqy_preloader )

set( ${target}_SRCS
  preloader.cc preloader.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konqueror_intern-static kio-shared kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
