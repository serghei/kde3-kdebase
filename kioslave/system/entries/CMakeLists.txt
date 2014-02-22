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

##### other data ################################

install( FILES
    home.desktop documents.desktop users.desktop
    media.desktop remote.desktop trash.desktop
  DESTINATION ${DATA_INSTALL_DIR}/systemview )

install( PROGRAMS kio_system_documenthelper DESTINATION ${BIN_INSTALL_DIR} )
