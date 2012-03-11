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

add_subdirectory( Terminal )

install( FILES
    sam.desktop xosview.desktop editres.desktop wine.desktop
    citrix_ica.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kappfinder/apps/System )
