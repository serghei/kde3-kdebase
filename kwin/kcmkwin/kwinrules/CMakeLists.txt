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

add_definitions( -DKCMRULES )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kwinrules.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_kwinrules (module) ####################

kde_add_kpart( kcm_kwinrules AUTOMOC
  SOURCES kcm.cpp
  LINK kwinrules-static
  DESTINATION ${PLUGIN_INSTALL_DIR} )


##### kwin_rules_dialog (kdeinit) ###############

kde_add_kdeinit_executable( kwin_rules_dialog
  SOURCES main.cpp
  LINK ${TQT_LIBRARIES} kwinrules-static )


##### kwinrules (static) ########################

set( target kwinrules )

set( ${target}_SRCS
  ruleswidget.cpp ruleslist.cpp kwinsrc.cpp detectwidget.cpp
  ruleswidgetbase.ui ruleslistbase.ui detectwidgetbase.ui
  editshortcutbase.ui )

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeui-shared )
