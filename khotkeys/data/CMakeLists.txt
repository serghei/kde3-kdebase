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
    kde32b1.khotkeys
    konqueror_gestures_kde321.khotkeys
    printscreen.khotkeys
    multimedia_keys.khotkeys
  DESTINATION ${DATA_INSTALL_DIR}/khotkeys )

install( FILES
    khotkeys_32b1_update.upd
    konqueror_gestures_kde321_update.upd
    khotkeys_printscreen.upd
    khotkeys_multimedia_keys.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
