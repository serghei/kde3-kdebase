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

add_subdirectory( Editors )
add_subdirectory( xutils )

install( FILES
    GnomeWho.desktop SystemInfo.desktop SystemLogViewer.desktop
    calctool.desktop e-notes.desktop oclock.desktop GKrellM.desktop
    xgnokii.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kappfinder/apps/Utilities/ )
