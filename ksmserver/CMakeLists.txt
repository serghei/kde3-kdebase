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

if( NOT WITH_DBUS_SYSTEM_BUS )
  set( WITH_DBUS_SYSTEM_BUS "unix:path=/var/run/dbus/system_bus_socket" CACHE INTERNAL "" )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES shutdownkonq.png DESTINATION ${DATA_INSTALL_DIR}/ksmserver/pics )
install( FILES ksmserver.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES move_session_config.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### ksmserver (kdeinit) #######################

kde_add_kdeinit_executable( ksmserver AUTOMOC
  SOURCES
    main.cpp server.cpp shutdowndlg.cpp legacy.cpp startup.cpp
    shutdown.cpp client.cpp KSMServerInterface.skel server.skel
  LINK dmctl-static kdeui-shared kdedbus-shared
)
