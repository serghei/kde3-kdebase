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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    konq_treeview.desktop konq_detailedlistview.desktop
    konq_textview.desktop konq_infolistview.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES
    konq_treeview.rc konq_detailedlistview.rc
    konq_textview.rc konq_infolistview.rc
  DESTINATION ${DATA_INSTALL_DIR}/konqlistview )

install( FILES konq_listview.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### konq_listview (module) ####################

set( target konq_listview )

set( ${target}_SRCS
  konq_listview.cc konq_listviewwidget.cc konq_listviewitems.cc
  konq_treeviewwidget.cc konq_treeviewitem.cc konq_textviewwidget.cc
  konq_textviewitem.cc konq_infolistviewwidget.cc
  konq_infolistviewitem.cc konq_listviewsettings.kcfgc
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
