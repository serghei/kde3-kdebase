#################################################
#
#  (C) 2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

kde_install_icons( kpowermanager )

kde_install_icons(
    ac battery battery_charging suspend hibernate
  DESTINATION ${DATA_INSTALL_DIR}/kpowermanager/icons )
