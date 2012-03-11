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

project( kwin )

add_subdirectory( lib )
add_subdirectory( killer )
add_subdirectory( kcmkwin )
add_subdirectory( pics )
add_subdirectory( clients )
add_subdirectory( oldheaders )
add_subdirectory( data )
kde_conditional_add_subdirectory( WITH_XCOMPOSITE kompmgr )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### headers ###################################

install( FILES KWinInterface.h DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

kde_install_icons( kwin )
install( FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/kwin )
install( FILES kwin.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kwin (kdeinit) ############################

set( target kwin )

set( ${target}_SRCS
  workspace.cpp client.cpp placement.cpp atoms.cpp
  utils.cpp layers.cpp main.cpp popupinfo.cpp tabbox.cpp
  options.cpp plugins.cpp events.cpp KWinInterface.skel
  killwindow.cpp geometrytip.cpp sm.cpp group.cpp bridge.cpp
  manage.cpp notifications.cpp activation.cpp useractions.cpp
  geometry.cpp rules.cpp
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdecorations-shared kdeui-shared Xext
)

##### install import cmake modules ###############

kde_install_export( )

