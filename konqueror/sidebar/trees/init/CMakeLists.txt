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

add_subdirectory( remote )
add_subdirectory( services )


##### other data ################################

install( FILES
    home.desktop root.desktop history.desktop bookmarks.desktop
    services.desktop remote.desktop system.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/entries )

install( FILES
    home.desktop root.desktop history.desktop bookmarks.desktop
    services.desktop remote.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/kicker_entries )

install( FILES
    virtualfolderadd.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/add )

install( FILES
    history_module.desktop dirtree_module.desktop bookmarks_module.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/dirtree )
