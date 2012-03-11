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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kthememanager.desktop installktheme.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES x-ktheme.desktop DESTINATION ${MIME_INSTALL_DIR}/application )
install( PROGRAMS kdeinstallktheme DESTINATION ${BIN_INSTALL_DIR} )


##### kcm_kthememanager (module) ################

kde_add_kpart( kcm_kthememanager AUTOMOC
  SOURCES
    kthemedlg.ui newthemewidget.ui knewthemedlg.cpp
    ktheme.cpp kthememanager.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
