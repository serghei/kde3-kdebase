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

add_subdirectory( Templates )

install( FILES
    directory.trash directory.autostart directory.templates directory.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kdesktop )

install( FILES
    Home.desktop System.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kdesktop/DesktopLinks )
