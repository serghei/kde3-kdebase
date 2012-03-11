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

install( FILES kcmview1394.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES oui.db DESTINATION ${DATA_INSTALL_DIR}/kcmview1394 )


##### kcm_view1394 (module) #####################

kde_add_kpart( kcm_view1394 AUTOMOC
  SOURCES view1394widget.ui view1394.cpp
  LINK kdeui-shared ${LIBRAW1394_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
