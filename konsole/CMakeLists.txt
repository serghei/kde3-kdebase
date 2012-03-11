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

add_subdirectory( konsole )
add_subdirectory( other )
# add_subdirectory( doc )
add_subdirectory( fonts )


install( FILES
    konsole.desktop konsolesu.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )


install( FILES
    konsolepart.desktop kwrited.desktop konsole-script.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )


install( FILES
    terminalemulator.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR} )


install( FILES
    konsole.upd
  DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


install( PROGRAMS
    schemaStrip.pl
  DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


install( FILES
    kwrited.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/kded )


install( FILES
    konsolehere.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )


install( FILES
    eventsrc tips
  DESTINATION ${DATA_INSTALL_DIR}/konsole )
