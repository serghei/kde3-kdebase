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

project( konqueror )

if( NOT BUILD_LIBKONQ )
  include( "${TDE_CMAKE_DIR}/libkonq.cmake" )
endif( NOT BUILD_LIBKONQ )


add_definitions(
  -D_LARGEFILE64_SOURCE
)


add_subdirectory( client )
add_subdirectory( iconview )
add_subdirectory( listview )
add_subdirectory( keditbookmarks )
add_subdirectory( shellcmdplugin )
add_subdirectory( about )
add_subdirectory( pics )
add_subdirectory( sidebar )
add_subdirectory( preloader )
add_subdirectory( quickprint )
add_subdirectory( remoteencodingplugin )
add_subdirectory( kttsplugin )


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### headers ###################################

install( FILES
    KonquerorIface.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES konqueror.rc konq-simplebrowser.rc DESTINATION ${DATA_INSTALL_DIR}/konqueror )
install( FILES konqueror.desktop DESTINATION ${APPS_INSTALL_DIR} )
install( FILES konqfilemgr.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )
install( FILES konqueror.kcfg DESTINATION ${KCFG_INSTALL_DIR} )

install( FILES
    kfmclient.desktop kfmclient_dir.desktop kfmclient_html.desktop
    kfmclient_war.desktop konqbrowser.desktop konquerorsu.desktop
    Home.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

set( PROFILEDIR ${DATA_INSTALL_DIR}/konqueror/profiles )
install( FILES profile_webbrowsing.desktop DESTINATION ${PROFILEDIR} RENAME webbrowsing )
install( FILES profile_filemanagement.desktop DESTINATION ${PROFILEDIR} RENAME filemanagement )
install( FILES profile_midnightcommander.desktop DESTINATION ${PROFILEDIR} RENAME midnightcommander )
install( FILES profile_tabbedbrowsing.desktop DESTINATION ${PROFILEDIR} RENAME tabbedbrowsing )
install( FILES profile_kde_devel.desktop DESTINATION ${PROFILEDIR} RENAME kde_devel )
install( FILES profile_simplebrowser.desktop DESTINATION ${PROFILEDIR} RENAME simplebrowser )


##### konqueror_intern (static lib) #############

set( target konqueror_intern )

set( ${target}_SRCS
  konq_settingsxt.kcfgc
)

kde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
)


##### konqueror (kdeinit) #######################

set( target konqueror )

set( ${target}_SRCS
  konq_main.cc KonquerorIface.cc KonquerorIface.skel
  KonqMainWindowIface.cc KonqMainWindowIface.skel
  KonqViewIface.cc KonqViewIface.skel konq_guiclients.cc
  konq_run.cc konq_view.cc konq_viewmgr.cc konq_misc.cc
  konq_frame.cc konq_tabs.cc konq_actions.cc konq_profiledlg.cc
  konq_factory.cc konq_combo.cc konq_browseriface.cc
  delayedinitializer.cc konq_mainwindow.cc konq_extensionmanager.cc
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konqueror_intern-static konq-shared kutils-shared
)


##### install import cmake modules ###############

kde_install_export( )
