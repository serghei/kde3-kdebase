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

if( BUILD_KCONTROL OR BUILD_KDESKTOP OR BUILD_KDM )

  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libkonq
    ${KDE_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
  )

  ##### bgnd (static) #############################

  kde_add_library( bgnd STATIC_PIC AUTOMOC
    SOURCES bgrender.cpp bgsettings.cpp
  )

endif( )


if( BUILD_KCONTROL )

  add_subdirectory( pics )

  ##### other data ################################

  install( FILES background.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


  ##### kcm_background (module) ###################

  kde_add_kpart( kcm_background AUTOMOC
    SOURCES main.cpp
    LINK kcmbgnd-static bgnd-static knewstuff-shared
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )


  ##### kcmbgnd (static) ##########################

  kde_add_library( kcmbgnd STATIC_PIC AUTOMOC
    SOURCES
      bgdialog.cpp bgdialog_ui.ui bgwallpaper_ui.ui
      bgwallpaper.cpp bgadvanced_ui.ui bgadvanced.cpp
      bgmonitor.cpp
  )

endif( )
