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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( kdeprintfax )
kde_install_icons( abentry DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax/icons )
install( FILES kdeprintfax.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kdeprintfaxui.rc faxfilters DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax )
install( PROGRAMS anytops DESTINATION ${DATA_INSTALL_DIR}/kdeprintfax )


##### kdeprintfax (executable) ##################

kde_add_executable( kdeprintfax AUTOMOC
  SOURCES
    main.cpp kdeprintfax.cpp faxab.cpp faxctrl.cpp
    confgeneral.cpp configdlg.cpp conffax.cpp
    confsystem.cpp conffilters.cpp filterdlg.cpp
    defcmds.cpp
  LINK kabc-shared kdeprint-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
