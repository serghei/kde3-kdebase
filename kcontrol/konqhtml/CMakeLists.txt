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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    khtml_behavior.desktop khtml_java_js.desktop
    khtml_fonts.desktop khtml_filter.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install(
  FILES uninstall.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/WebBrowsing
  RENAME nsplugin.desktop )

install(
  FILES uninstall.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/WebBrowsing
  RENAME khtml_appearance.desktop )

##### kcm_konqhtml (module) #####################

kde_add_kpart( kcm_konqhtml AUTOMOC
  SOURCES
    advancedTabDialog.cpp htmlopts.cpp jsopts.cpp
    javaopts.cpp pluginopts.cpp appearance.cpp
    khttpoptdlg.cpp policydlg.cpp main.cpp
    jspolicies.cpp nsconfigwidget.ui policies.cpp
    domainlistview.cpp advancedTabOptions.ui
    filteropts.cpp
  LINK khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
