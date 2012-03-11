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

install( FILES kateplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES kate.desktop kwrite.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kateui.rc externaltools tips DESTINATION ${DATA_INSTALL_DIR}/kate )
install( FILES kwriteui.rc DESTINATION ${DATA_INSTALL_DIR}/kwrite )
install( FILES kate-2.4.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )
install( FILES katerc DESTINATION ${CONFIG_INSTALL_DIR} )
