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

# FIXME this is far from complete!!!

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_BINARY_DIR}
  ${DBUS_QT_INCLUDE_DIR}
  ${DBUS_INCLUDE_DIRS}
)


##### kdm (executable) ##########################

if( WITH_XDMCP )
  set( XDMCP_LIBRARIES "Xdmcp" )
endif()

if( WITH_CONSOLE_KIT )
  add_definitions( -DWITH_CONSOLE_KIT )
  set( consolekit_SRC consolekit.c )
endif( )

kde_add_executable( kdm
  SOURCES
    access.c auth.c bootman.c choose.c client.c
    ctrl.c daemon.c dm.c dpylist.c error.c genauth.c
    inifile.c krb5auth.c mitauth.c netaddr.c policy.c
    process.c protodpy.c reset.c resource.c rpcauth.c
    server.c session.c sessreg.c socket.c streams.c
    util.c xdmauth.c xdmcp.c ${consolekit_SRC}
  DEPENDENCIES config.ci-common
  LINK X11 ${XAU_LIBRARIES} ${DBUS_TQT_LIBRARIES} ${DBUS_LIBRARIES} ${CRYPT_LIBRARY} ${PAM_LIBRARY} ${XDMCP_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
