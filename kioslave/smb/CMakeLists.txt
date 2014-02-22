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

include( ConfigureChecks.cmake )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${SMBCLIENT_INCLUDE_DIRS}
)


##### other data ################################

install( FILES smb.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES smb-network.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/dirtree/remote )
install( FILES smb-network.desktop DESTINATION ${DATA_INSTALL_DIR}/remoteview )
install( FILES x-smb-workgroup.desktop x-smb-server.desktop DESTINATION ${MIME_INSTALL_DIR}/application )


##### kio_smb (module) ##########################

kde_add_kpart( kio_smb AUTOMOC
  SOURCES
    kio_smb.cpp kio_smb_auth.cpp kio_smb_browse.cpp kio_smb_config.cpp
    kio_smb_dir.cpp kio_smb_file.cpp kio_smb_internal.cpp kio_smb_mount.cpp
  LINK kio-shared ${SMBCLIENT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
