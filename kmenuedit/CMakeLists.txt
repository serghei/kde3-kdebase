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

add_subdirectory( pixmaps )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( kmenuedit )
install( FILES kmenuedit.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/System RENAME kmenuedit.desktop )
install( FILES kmenueditui.rc DESTINATION ${DATA_INSTALL_DIR}/kmenuedit )
install( FILES kcontroleditui.rc DESTINATION ${DATA_INSTALL_DIR}/kcontroledit )


##### kmenuedit (kdeinit) #######################

kde_add_kdeinit_executable( kmenuedit AUTOMOC
  SOURCES main.cpp
  LINK ${TQT_LIBRARIES} kmenueditcommon-static
)


##### kcontroledit (kdeinit) ####################

kde_add_kdeinit_executable( kcontroledit AUTOMOC
  SOURCES kcontrol_main.cpp
  LINK ${TQT_LIBRARIES} kmenueditcommon-static
)


##### kmenueditcommon (static) ##################

kde_add_library( kmenueditcommon STATIC_PIC AUTOMOC
  SOURCES
    basictab.cpp treeview.cpp kmenuedit.cpp
    khotkeys.cpp menufile.cpp menuinfo.cpp
  LINK kio-shared
)
