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

install( FILES
    printscreen.khotkeys
  DESTINATION ${DATA_INSTALL_DIR}/khotkeys )

install( FILES
    khotkeys_32b1_update.upd khotkeys_printscreen.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
