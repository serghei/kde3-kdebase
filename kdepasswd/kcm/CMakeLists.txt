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

kde_import( libkonq )

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcm_useraccount.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kcm_useraccount.kcfg kcm_useraccount_pass.kcfg DESTINATION ${KCFG_INSTALL_DIR} )


##### kcm_useraccount (module) ##################

kde_add_kpart( kcm_useraccount AUTOMOC
  SOURCES
    main_widget.ui chfnprocess.cpp main.cpp
    chfacedlg.cpp settings.kcfgc pass.kcfgc
  LINK konq-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
